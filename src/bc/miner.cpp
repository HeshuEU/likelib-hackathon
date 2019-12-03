#include "miner.hpp"

#include <random>
#include <utility>


namespace
{

    std::size_t calcThreadsNum(const base::PropertyTree& config) {
        if(config.hasKey("miner.threads")) {
            return config.get<std::size_t>("miner.threads");
        }
        else {
            return std::thread::hardware_concurrency();
        }
    }

}



namespace bc
{


namespace impl
{


    class MinerWorker
    {
      public:
        //===================
        MinerWorker(std::atomic<std::size_t>& version, Task& task, std::optional<Block>& block_to_mine,
            std::shared_mutex& state_mutex, std::condition_variable_any& state_changed_cv, Miner::HandlerType handler);

        ~MinerWorker();
        //===================
      private:
        //===================
        std::thread _worker_thread;
        std::size_t _last_read_version;
        Task _last_read_task;
        std::optional<Block> _last_read_block_to_mine;
        //===================
        std::atomic<std::size_t>& _version;
        Task& _task;
        std::optional<Block>& _block_to_mine;
        std::shared_mutex& _state_mutex;
        std::condition_variable_any& _state_changed_cv;
        Miner::HandlerType _handler;
        //===================
        void worker();
        //===================
    };

} // namespace impl


Miner::Miner(const base::PropertyTree& config, Miner::HandlerType handler) : _handler{std::move(handler)}
{
    // setting up initial state
    _task = impl::Task::NONE;
    _job_version = 0;

    // setting up threads
    std::size_t num_threads = calcThreadsNum(config);

    for(std::size_t i = 0; i < num_threads; ++i) {
        _workers.emplace_front(_job_version, _task, _block_to_mine, _state_mutex, _state_changed_cv, _handler);
    }

    LOG_INFO << "Miner is running on " << num_threads << " threads";
}


Miner::~Miner()
{
    stop();
}


void Miner::findNonce(const Block& block_without_nonce)
{
    std::unique_lock lk(_state_mutex);
    ++_job_version;
    _task = impl::Task::FIND_NONCE;
    _block_to_mine = block_without_nonce;
    _state_changed_cv.notify_all();
}


void Miner::dropJob()
{
    std::unique_lock lk(_state_mutex);
    ++_job_version;
    _task = impl::Task::NONE;
    _block_to_mine.reset();
    _state_changed_cv.notify_all();
}


void Miner::stop()
{
    std::unique_lock lk(_state_mutex);
    ++_job_version;
    _task = impl::Task::EXIT;
    _block_to_mine.reset();
    _state_changed_cv.notify_all();
}

//=============================

namespace impl
{

    inline base::Bytes tempGetComplexity()
    {
        // TODO: rewrite
        base::Bytes b(32);
        b[3] = 0x7f;
        return b;
    }

    MinerWorker::MinerWorker(std::atomic<std::size_t>& version, impl::Task& task, std::optional<Block>& block_to_mine,
        std::shared_mutex& state_mutex, std::condition_variable_any& state_changed_cv, Miner::HandlerType handler)
        : _last_read_version{0}, _version{version}, _task{task}, _block_to_mine{block_to_mine},
          _state_mutex{state_mutex}, _state_changed_cv{state_changed_cv}, _handler{handler}
    {
        _worker_thread = std::thread(&MinerWorker::worker, this);
    }


    MinerWorker::~MinerWorker()
    {
        if(_worker_thread.joinable()) {
            _worker_thread.join();
        }
    }


    void MinerWorker::worker()
    {
        bool is_stopping{false};
        std::mt19937_64 mt{std::random_device{}()};

        while(!is_stopping) {
            {
                std::shared_lock lk(_state_mutex);
                _state_changed_cv.wait(lk, [this] {
                    return _last_read_version != _version;
                });
                _last_read_version = _version;
                _last_read_task = _task;
                _last_read_block_to_mine = _block_to_mine;
            }

            switch(_last_read_task) {
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
                    ASSERT(_last_read_block_to_mine);
                    Block& b = _last_read_block_to_mine.value();
                    while(_last_read_version == _version) {
                        auto attempting_nonce = mt();
                        b.setNonce(attempting_nonce);
                        if(base::Sha256::compute(base::toBytes(b)).getBytes() < tempGetComplexity()) {
                            std::unique_lock lk(_state_mutex);
                            _handler(std::move(b));
                            _version++;
                            _task = Task::DROP_JOB;
                            _block_to_mine.reset();
                            _state_changed_cv.notify_all();
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

} // namespace bc
