#ifndef __APPLICATION_APPLICATION_H__
#define __APPLICATION_APPLICATION_H__

#include <memory>
#include <string>

#include "src/ConnectionPool/CommonConnectionPool.h"
#include "src/http_server/http_server.hpp"
#include "src/oss/oss_client.h"
#include "src/util.h"

#include "pico/config.h"


class Application
{
public:
    explicit Application(const std::string& address)
        : m_server(address) {}
    ~Application() {}

    void start() { m_server.start(); }

    void stop() { m_server.stop(); }

private:
    HttpServer m_server;
};

#endif
