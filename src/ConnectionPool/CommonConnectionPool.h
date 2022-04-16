#pragma once

#include "Connection.h"
#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>


class ConnectionPool
{
public:
    static ConnectionPool* getConnectionPool();
    std::shared_ptr<Connection> getConnection();

private:
    ConnectionPool();
    bool loadConfigFile();

    void productConnectionTask();

    void scannerConnectionTask();

    ~ConnectionPool();

    std::string _ip;
    unsigned short _port;
    std::string _username;
    std::string _password;
    std::string _dbname;
    int _initSize = 5;
    int _maxSize = 10;
    int _maxIdleTime = 20;
    int _connectionTimeout = 40;

    std::queue<Connection*> _connQueue;

    std::mutex _queueMutex;

    std::atomic_int _connCnt;

    std::condition_variable cv;
};
