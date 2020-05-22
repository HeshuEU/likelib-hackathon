#pragma once

#include "base/error.hpp"
#include "base/time.hpp"
#include "base/utility.hpp"
#include "core/address.hpp"
#include "core/block.hpp"
#include "core/messages.hpp"
#include "net/error.hpp"
#include "net/session.hpp"

#include <atomic>
#include <forward_list>
#include <memory>

namespace lk
{

// peer context requirements for handling messages
class Core;
class Host;
class PeerPoolBase;
class KademliaPeerPoolBase;
//=======================

class Rating
{
  public:
    // TODO: remember peer rating during several node runs

    explicit Rating(std::int_fast32_t initial_value = base::config::NET_INITIAL_PEER_RATING);

    std::int_fast32_t getValue() const noexcept;
    explicit operator bool() const noexcept; // if false, then the peer is too bad

    Rating& nonExpectedMessage() noexcept;
    Rating& invalidMessage() noexcept;
    Rating& badBlock() noexcept;
    Rating& differentGenesis() noexcept;

  private:
    std::int_fast32_t _value;
};


class Peer : public std::enable_shared_from_this<Peer>
{
  public:
    //=========================
    /**
     * Classes that are required for network messages handling.
     */
    struct Context
    {
        lk::Core& core; //! for operating with blockchain
        lk::Host& host; //! for operating with host data
    };

    enum class State
    {
        JUST_ESTABLISHED,
        REQUESTED_BLOCKS,
        SYNCHRONISED
    };

    //=========================
    static std::shared_ptr<Peer> accepted(std::shared_ptr<net::Session> session, Context context);
    static std::shared_ptr<Peer> connected(std::shared_ptr<net::Session> session, Context context);

    ~Peer();
    //=========================
    base::Time getLastSeen() const;
    net::Endpoint getEndpoint() const;
    net::Endpoint getPublicEndpoint() const;
    bool wasConnectedTo() const noexcept;
    //=========================
    const lk::Address& getAddress() const noexcept;
    //=========================
    msg::NodeIdentityInfo getInfo() const;
    bool isClosed() const;
    //=========================
    void requestLookup(const lk::Address& address, uint8_t alpha);
    void requestBlock(const base::Sha256& block_hash);

    void sendBlock(const base::Sha256& block_hash, const lk::Block& block);
    void sendTransaction(const lk::Transaction& tx);
    //=========================
    /**
     * If the peer was accepted, it responds to it whether the acception was successful or not.
     * If the peer was connected to it waits for reply.
     */
    void startSession();

    template<typename T>
    void endSession(T last_message);

  private:
    //=========================
    Peer(std::shared_ptr<net::Session> session,
         bool was_connected_to,
         boost::asio::io_context& io_context,
         lk::PeerPoolBase& non_handshaked_pool,
         lk::KademliaPeerPoolBase& handshaked_pool,
         lk::Core& core,
         lk::Host& host);
    //=========================
    /*
     * Tries to add peer to a peer pool, to which it is attached.
     * @return true if success, false - otherwise
     */
    bool tryAddToPool();
    void detachFromPools(); // only called inside onClose or inside destructor

    //===========================================================

    class Requests
    {
        using MessageId = std::uint16_t;

        class SessionHandler : public net::Session::Handler
        {
          public:
            explicit SessionHandler(Requests& requests);

            void onReceive(const base::Bytes& bytes) override;
            void onClose() override;

          private:
            Requests& _r;
        };

        class Request : std::enable_shared_from_this<Request>
        {
          public:
            using ResponseCallback = std::function<void(base::SerializationIArchive&&)>;
            using TimeoutCallback = std::function<void()>;

            Request(base::OwningPool<Request>& active_requests,
                    MessageId id,
                    ResponseCallback response_callback,
                    TimeoutCallback timeout_callback,
                    boost::asio::io_context& io_context);

            MessageId getId() const noexcept;

            void runCallback(base::SerializationIArchive&& ia);

          private:
            base::OwningPool<Request>& _active_requests;
            MessageId _id;
            ResponseCallback _response_callback;
            TimeoutCallback _timeout_callback;
            boost::asio::steady_timer _timer;
            std::atomic_flag _is_already_processed;
        };


      public:
        using CloseCallback = std::function<void()>;

        Requests(std::weak_ptr<net::Session> session,
                 boost::asio::io_context& io_context,
                 Request::ResponseCallback default_callback,
                 CloseCallback close_callback);

        void onMessageReceive(const base::Bytes& received_bytes);

        void onClose();

        template<typename T>
        void send(const T& msg);

        template<typename T>
        void requestWaitResponseById(const T& msg,
                                     Request::ResponseCallback response_callback,
                                     Request::TimeoutCallback timeout_callback);

      private:
        std::weak_ptr<net::Session> _session;
        boost::asio::io_context& _io_context;
        MessageId _next_message_id{ 0 };
        base::OwningPool<Request> _active_requests;
        Request::ResponseCallback _default_callback;
        CloseCallback _close_callback;

        template<typename T>
        base::Bytes prepareMessage(const T& msg);
    };

    //===========================================================
    std::shared_ptr<net::Session> _session;
    bool _is_started{ false };
    bool _was_connected_to; // peer was connected to or accepted?
    //=========================
    boost::asio::io_context& _io_context;
    //=========================
    Rating _rating;
    State _state{ State::JUST_ESTABLISHED };
    std::optional<net::Endpoint> _endpoint_for_incoming_connections;
    lk::Address _address;

    void setServerEndpoint(net::Endpoint endpoint);
    void setState(State state);
    State getState() const noexcept;
    //=========================
    class Synchronizer
    {
      public:
        explicit Synchronizer(Peer& peer);

        void handleReceivedTopBlockHash(const base::Sha256& peers_top_block);
        bool handleReceivedBlock(const base::Sha256& hash, const Block& block);
        bool isSynchronised() const;

      private:
        Peer& _peer;
        std::optional<base::Sha256> _requested_block;
        std::vector<lk::Block> _sync_blocks;

        void requestBlock(base::Sha256 block_hash);
    };

    Synchronizer _synchronizer{ *this };
    //================
    lk::PeerPoolBase& _non_handshaked_pool;
    bool _is_attached_to_handshaked_pool{ false };
    lk::KademliaPeerPoolBase& _handshaked_pool;
    lk::Core& _core;
    lk::Host& _host;
    //=========================
    Requests _requests;
    void process(base::SerializationIArchive&& ia);
    //=========================
    void handle(msg::Connect&& msg);
    void handle(msg::CannotAccept&& msg);
    void handle(msg::Accepted&& msg);
    void handle(msg::Ping&& msg);
    void handle(msg::Pong&& msg);
    void handle(msg::Lookup&& msg);
    void handle(msg::LookupResponse&& msg);
    void handle(msg::Transaction&& msg);
    void handle(msg::GetBlock&& msg);
    void handle(msg::Block&& msg);
    void handle(msg::BlockNotFound&& msg);
    void handle(msg::Close&& msg);
    //=========================
};


class PeerPoolBase
{
  public:
    virtual bool tryAddPeer(std::shared_ptr<Peer> peer) = 0;
    virtual void removePeer(std::shared_ptr<Peer> peer) = 0;
    virtual void removePeer(Peer* peer) = 0;

    virtual void forEachPeer(std::function<void(const Peer&)> f) const = 0;
    virtual void forEachPeer(std::function<void(Peer&)> f) = 0;

    virtual bool hasPeerWithEndpoint(const net::Endpoint& endpoint) const = 0;

    std::vector<msg::NodeIdentityInfo> allPeersInfo() const;
};

class KademliaPeerPoolBase : public PeerPoolBase
{
  public:
    virtual std::vector<msg::NodeIdentityInfo> lookup(const lk::Address& address, std::size_t alpha) = 0;
};

}

#include "peer.tpp"
