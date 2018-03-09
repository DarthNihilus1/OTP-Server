#pragma once
#include <list>
#include <queue>
#include <mutex>
#include <boost/asio.hpp>
#include <deps/uvw/uvw.hpp>
#include "util/Datagram.h"

// NOTES:
//
// Do not subclass NetworkClient. Instead, you should implement NetworkHandler
// and instantiate NetworkClient with std::make_shared.
//
// To begin receiving, pass it an ASIO socket via initialize().
//
// You must not destruct your NetworkHandler implementor until
// receive_disconnect is called!

class NetworkClient;

class NetworkHandler
{
protected:
    // receive_datagram is called when both a datagram's size and its data
    //     have been received asynchronously from the network.
    virtual void receive_datagram(DatagramHandle dg) = 0;
    // receive_disconnect is called when the remote host closes the
    //     connection or otherwise when the tcp connection is lost.
    virtual void receive_disconnect(const uvw::ErrorEvent &) = 0;

    friend class NetworkClient;
};

class NetworkClient : public std::enable_shared_from_this<NetworkClient>
{
public:
    NetworkClient(NetworkHandler *handler);
    ~NetworkClient();

    inline void initialize(const std::shared_ptr<uvw::TcpHandle>& socket)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        initialize(socket, lock);
    }

    inline void initialize(const std::shared_ptr<uvw::TcpHandle>& socket,
                           const uvw::Addr& remote,
                           const uvw::Addr& local)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        initialize(socket, remote, local, lock);
    }

    void set_write_timeout(unsigned int timeout);
    void set_write_buffer(uint64_t max_bytes);

    // send_datagram immediately sends the datagram over TCP (blocking).
    void send_datagram(DatagramHandle dg);

    // disconnect closes the TCP connection without informing the NetworkHandler.
    inline void disconnect(uv_errno_t ec)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        disconnect(ec, lock);
    }

    inline void disconnect()
    {
        disconnect((uv_errno_t)0);
    }

    // handle_disconnect closes the TCP connection and informs the NetworkHandler.
    inline void handle_disconnect(uv_errno_t ec)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        handle_disconnect(ec, lock);
    }

    // is_connected returns true if the TCP connection is active, or false otherwise
    inline bool is_connected()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return is_connected(lock);
    }

    inline uvw::Addr get_remote()
    {
        return m_remote;
    }

    inline uvw::Addr get_local()
    {
        return m_local;
    }

private:
    // Locked versions of public functions:
    void initialize(const std::shared_ptr<uvw::TcpHandle>& socket,
                    std::unique_lock<std::mutex> &lock);
    void initialize(const std::shared_ptr<uvw::TcpHandle>& socket,
                    const uvw::Addr &remote,
                    const uvw::Addr &local,
                    std::unique_lock<std::mutex> &lock);
    void disconnect(uv_errno_t ec, std::unique_lock<std::mutex> &lock);
    bool is_connected(std::unique_lock<std::mutex> &lock);

    /* Asynchronous call loop */
    // async_send is called by send_datagram or send_finished when the socket is available
    //     for writing to send the next datagram in the queue.
    void async_send(DatagramHandle dg, std::unique_lock<std::mutex> &lock);
    // send_finished is called when an async_send has completed
    void send_finished();
    // send_expired is called when an async_send has expired
    void send_expired();

    // determine_endpoints is called by initialize() after m_socket is set to a
    //     provided socket.
    bool determine_endpoints(uvw::Addr &remote,
                             uvw::Addr &local);

    // start_receive is called by initialize() to begin receiving data.
    void start_receive();

    void socket_write(char* buf, size_t length, std::unique_lock<std::mutex> &lock);

    void handle_disconnect(uv_errno_t ec, std::unique_lock<std::mutex> &lock);

    void defragment_input();
    void process_datagram(const std::unique_ptr<char[]>& data, size_t length);

    bool m_is_sending = false;
    char *m_send_buf = nullptr;

    NetworkHandler *m_handler;
    std::shared_ptr<uvw::TcpHandle> m_socket;
    std::shared_ptr<uvw::TimerHandle> m_async_timer;
    uvw::Addr m_remote;
    uvw::Addr m_local;
    std::vector<unsigned char> m_data_buf;

    uint64_t m_total_queue_size = 0;
    uint64_t m_max_queue_size = 0;
    unsigned int m_write_timeout = 0;
    std::queue<DatagramHandle> m_send_queue;

    std::mutex m_mutex;

    bool m_disconnect_handled = false;
    bool m_local_disconnect = false;
    uv_errno_t m_disconnect_error;
};