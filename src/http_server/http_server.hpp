#ifndef __HTTP_SERVER_HTTP_SERVER_H__
#define __HTTP_SERVER_HTTP_SERVER_H__

#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "pico/http/http_server.hpp"

template<typename... Middlewares>
class HttpServer
{
    typedef std::function<void(const pico::HttpRequest::Ptr&, pico::HttpResponse::Ptr&)> Handler;

public:
    explicit HttpServer(std::string addr)
        : m_request_handler(new pico::RequestHandler()) {
        m_main_worker.reset(new pico::IOManager(1, true, "main"));


        m_addr = pico::Address::LookupAnyIPAddress(addr);
    }

    ~HttpServer() { m_main_worker->stop(); }

    void addRoute(std::string path, pico::HttpMethod method, Handler handler) {
        m_request_handler->addRoute(path, method, handler);
    }

    void addGlobalRoute(std::string path, pico::HttpMethod method, Handler handler) {
        m_request_handler->addGlobalRoute(path, method, handler);
    }

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

    typename pico::HttpServer<Middlewares...>::Ptr& get_server() { return m_http_server; }

private:
    void run_in_fiber() {
        pico::IOManager::Ptr worker(new pico::IOManager(2, false, "worker"));
        pico::IOManager::Ptr acceptor(new pico::IOManager(2, false, "acceptor"));

        m_http_server.reset(
            new pico::HttpServer<Middlewares...>(true, worker.get(), acceptor.get()));
        m_http_server->setRequestHandler(m_request_handler);


        m_http_server->setName("pico");

        m_http_server->bind(m_addr);

        m_http_server->start();
    }

private:
    typename pico::HttpServer<Middlewares...>::Ptr m_http_server;

    pico::IOManager::Ptr m_main_worker;

    pico::Address::Ptr m_addr;

    pico::RequestHandler::Ptr m_request_handler;
};

#endif
