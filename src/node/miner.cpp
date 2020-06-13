#include "miner.hpp"

#include "base/log.hpp"

#include <random>
#include <utility>


namespace
{

std::size_t calcThreadsNum(const base::PropertyTree& config)
{
    if (config.hasKey("miner.threads")) {
        return config.get<std::size_t>("miner.threads");
    }
    else {
        return std::thread::hardware_concurrency();
    }
}

} // namespace


namespace impl
{

CommonState::CommonState(CommonData&& initial_state, MinerHandlerType handler)
  : _version{ 0 }
  , _common_data{ std::move(initial_state) }
  , _handler{ handler }
{}


std::size_t CommonState::getVersion() const
{
    return _version.load(std::memory_order_consume);
}


[[maybe_unused]] std::size_t CommonState::getCommonData(CommonData& data) const
{
    std::shared_lock lk(_state_mutex);
    data = _common_data;
    return getVersion();
}


void CommonState::setCommonData(const CommonData& data)
{
    std::unique_lock lk(_state_mutex);
    _version.fetch_add(1, std::memory_order_release);
    _common_data = data;
    _state_changed_cv.notify_all();
}


template<typename... Args>
void CommonState::callHandlerAndDrop(Args&&... args)
{
    {
        std::unique_lock lk(_state_mutex);
        if (_common_data.task != Task::FIND_NONCE) {
            /* guard on case if several handlers want to be called simultaneously.
             * we cannot bring _handler call under mutex, because if _handler
             * sets some work to miner, it will cause deadlock.
             */
            return;
        }

        _version.fetch_add(1, std::memory_order_release);
        _common_data.task = Task::DROP_JOB;
        _common_data.block_to_mine.reset();
        _common_data.complexity.reset();
        _state_changed_cv.notify_all();
    }

    _handler(std::forward<Args>(args)...);
}


void CommonState::waitAndReadNewData(std::size_t& last_read_version, CommonData& data)
{
    std::shared_lock lk(_state_mutex);
    _state_changed_cv.wait(lk, [this, last_read_version] { return getVersion() != last_read_version; });
    last_read_version = getVersion();
    data = _common_data;
}


class MinerWorker
{
  public:
    //===================
    MinerWorker(CommonState& common_state);
    ~MinerWorker();
    //===================
  private:
    //===================
    std::thread _worker_thread;
    //===================
    CommonState& _common_state;
    //===================
    void worker();
    //===================
};

} // namespace impl


Miner::Miner(const base::PropertyTree& config, Miner::HandlerType handler)
  : _common_state{ { impl::Task::NONE, std::nullopt, std::nullopt }, std::move(handler) }
{
    // setting up threads
    std::size_t num_threads = calcThreadsNum(config);

    for (std::size_t i = 0; i < num_threads; ++i) {
        _workers.emplace_front(_common_state);
    }

    LOG_INFO << "Miner is running on " << num_threads << " threads";
}


Miner::~Miner()
{
    stop();
}


void Miner::findNonce(const lk::MutableBlock& block_without_nonce, const lk::Complexity& complexity)
{
    _common_state.setCommonData({ impl::Task::FIND_NONCE, block_without_nonce, complexity });
}


void Miner::dropJob()
{
    _common_state.setCommonData({ impl::Task::NONE, std::nullopt, std::nullopt });
}


void Miner::stop()
{
    _common_state.setCommonData({ impl::Task::EXIT, std::nullopt, std::nullopt });
}

//=============================

namespace impl
{

MinerWorker::MinerWorker(CommonState& common_state)
  : _common_state{ common_state }
{
    _worker_thread = std::thread(&MinerWorker::worker, this);
}


MinerWorker::~MinerWorker()
{
    if (_worker_thread.joinable()) {
        _worker_thread.join();
    }
}


void MinerWorker::worker()
{
    bool is_stopping{ false };
    std::mt19937_64 mt{ std::random_device{}() };

    std::size_t last_read_version{ 0 };
    CommonData data;

    while (!is_stopping) {
        _common_state.waitAndReadNewData(last_read_version, data);

        switch (data.task) {
            case Task::NONE: {
                // do nothing
                break;
            }
            case Task::EXIT: {
                is_stopping = true;
                break;
            }
            case Task::DROP_JOB: {
                // do nothing, the job is already dropped
                break;
            }
            case Task::FIND_NONCE: {
                ASSERT(data.block_to_mine);
                ASSERT(data.complexity);
                lk::MutableBlock& b = data.block_to_mine.value();
                const auto complexity = data.complexity->getComparer();
                auto attempting_nonce = mt();
                while (last_read_version == _common_state.getVersion()) {
                    b.setNonce(attempting_nonce++); // overflow must go by modulo 2, since unsigned
                    if (base::Sha256::compute(base::toBytes(b)).getBytes() < complexity) {
                        lk::BlockBuilder builder(b);
                        _common_state.callHandlerAndDrop(std::move(builder).buildImmutable());
                    }
                }
                break;
            }
            default: {
                ASSERT(false);
                break;
            }
        }
    }
}

} // namespace impl
