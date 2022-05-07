#ifndef __HTTP_SERVER_HTTP_SERVER_H__
#define __HTTP_SERVER_HTTP_SERVER_H__

#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "pico/http/http_server.h"


class HttpServer
{
    typedef std::function<void(const pico::HttpRequest::Ptr&, pico::HttpResponse::Ptr&)> Handler;

public:
    explicit HttpServer(std::string addr) {
        m_main_worker.reset(new pico::IOManager(1, true, "main"));
        m_addr = pico::Address::LookupAnyIPAddress(addr);
    }

    ~HttpServer() { m_main_worker->stop(); }


    void start() {
        m_main_worker->schedule([this]() { this->run_in_fiber(); });
        m_main_worker->addTimer(
            2000,
            []() {

            },
            true);
        m_main_worker->stop();
    }

    void stop() { m_main_worker->stop(); }

    // get_server

private:
    void run_in_fiber() {
        pico::IOManager::Ptr worker(new pico::IOManager(2, false, "worker"));
        pico::IOManager::Ptr acceptor(new pico::IOManager(2, false, "acceptor"));

        m_http_server.reset(new pico::HttpServer(true, worker.get(), acceptor.get()));


        m_http_server->setName("pico");

        m_http_server->bind(m_addr);

        m_http_server->start();
    }

private:
    pico::HttpServer::Ptr m_http_server;

    pico::IOManager::Ptr m_main_worker;

    pico::Address::Ptr m_addr;
};

#endif
