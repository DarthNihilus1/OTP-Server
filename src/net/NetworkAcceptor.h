#pragma once
#include <thread>
#include <deps/uvw/uvw.hpp>

class NetworkAcceptor
{
  public:
    virtual ~NetworkAcceptor() {}

    // Parses the string "address" and binds to it. If no port is specified
    // as part of the address, it will use default_port.
    void bind(const std::string &address, unsigned int default_port);

    // The libuv loop thread for each acceptor runs under listener_thread.
    // Each acceptor has its own loop, and each acceptor's loop has its own thread.
    void listener_thread();

    void start();
    void stop();

    inline void set_haproxy_mode(bool haproxy_mode)
    {
        m_haproxy_mode = haproxy_mode;
    }

  protected:
    std::unique_ptr<std::thread> m_thread;
    std::shared_ptr<uvw::Loop> m_loop;
    std::shared_ptr<uvw::TcpHandle> m_acceptor;

    bool m_started = false;
    bool m_haproxy_mode = false;

    NetworkAcceptor();

    virtual void start_accept() = 0;
};
