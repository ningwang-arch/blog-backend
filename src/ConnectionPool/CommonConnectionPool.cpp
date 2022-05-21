#include "CommonConnectionPool.h"
#include "Connection.h"
#include "pico/config.h"
#include "pico/logging.h"
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>


#include "pico/env.h"
#include "pico/iomanager.h"

static pico::ConfigVar<std::string>::Ptr g_sql_conf =
    pico::Config::Lookup<std::string>("other.mysql.conf", "mysql.ini", "mysql conf");

ConnectionPool* ConnectionPool::getConnectionPool() {
    static ConnectionPool pool;
    return &pool;
}

bool ConnectionPool::loadConfigFile() {
    std::string conf_dir = pico::EnvManager::getInstance()->getConfigPath();
    if (conf_dir[conf_dir.size() - 1] != '/') { conf_dir += "/"; }
    std::string path = conf_dir + g_sql_conf->getValue();
    FILE* fp = fopen(path.c_str(), "r");
    if (fp == nullptr) {
        LOG_ERROR("open %s failed", path.c_str());
        return false;
    }

    while (!feof(fp)) {
        char line[1024] = {0};
        char* res = fgets(line, 1024, fp);
        if (res) {}
        std::string str(line);
        int index = str.find('=', 0);

        if (index == -1) { continue; }

        int endIndex = str.find('\n', index);
        std::string key = str.substr(0, index);
        std::string value = str.substr(index + 1, endIndex - index - 1);

        if (key == "ip") { _ip = value; }
        else if (key == "port") {
            _port = atoi(value.c_str());
        }
        else if (key == "username") {
            _username = value;
        }
        else if (key == "password") {
            _password = value;
        }
        else if (key == "dbname") {
            _dbname = value;
        }
        else if (key == "maxSize") {
            _maxSize = atoi(value.c_str());
        }
        else if (key == "maxIdleTime") {
            _maxIdleTime = atoi(value.c_str());
        }
        else if (key == "connectionTimeout") {
            _connectionTimeout = atoi(value.c_str());
        }
        else if (key == "initSize") {
            _initSize = atoi(value.c_str());
        }
    }
    return true;
}

ConnectionPool::ConnectionPool() {
    if (!loadConfigFile()) { return; }

    mysql_library_init(0, nullptr, nullptr);

    for (int i = 0; i < _initSize; i++) {
        Connection* p = new Connection(_ip, _port, _username, _password, _dbname);
        // p->connect();
        p->refreshAliveTime();
        _connQueue.push(p);
        _connCnt++;
    }

    std::thread produce(std::bind(&ConnectionPool::productConnectionTask, this));
    produce.detach();

    std::thread scanner(std::bind(&ConnectionPool::scannerConnectionTask, this));
    scanner.detach();
}

void ConnectionPool::productConnectionTask() {
    for (;;) {
        std::unique_lock<std::mutex> lock(_queueMutex);
        while (!_connQueue.empty()) { cv.wait(lock); }
        if (_connCnt < _maxSize) {
            Connection* p = new Connection(_ip, _port, _username, _password, _dbname);
            // p->connect();
            _connQueue.push(p);
            _connCnt++;
        }
        cv.notify_all();
    }
}

std::shared_ptr<Connection> ConnectionPool::getConnection() {
    std::unique_lock<std::mutex> lock(_queueMutex);
    while (_connQueue.empty()) {
        if (std::cv_status::timeout ==
            cv.wait_for(lock, std::chrono::milliseconds(_connectionTimeout)))
            if (_connQueue.empty()) {
                LOG_WARN("get connection timeout");
                return nullptr;
            }
    }
    std::shared_ptr<Connection> sp(_connQueue.front(), [&](Connection* pcon) {
        std::unique_lock<std::mutex> lock(_queueMutex);
        pcon->refreshAliveTime();
        _connQueue.push(pcon);
    });
    if (sp->ping()) {
        _connQueue.pop();
        cv.notify_all();
        return sp;
    }
    else if (sp->connect()) {
        _connQueue.pop();
        cv.notify_all();
        return sp;
    }
    else {
        LOG_WARN("connected failed");
        return nullptr;
    }
}

void ConnectionPool::scannerConnectionTask() {
    for (;;) {
        std::this_thread::sleep_for(std::chrono::seconds(_maxIdleTime));
        std::unique_lock<std::mutex> lock(_queueMutex);

        while (_connCnt > _initSize) {
            Connection* p = _connQueue.front();
            if (p->getAliveTime() >= (_maxIdleTime * 1000)) {
                _connQueue.pop();
                _connCnt--;
                delete p;
            }
            else {
                break;
            }
        }
    }
}

ConnectionPool::~ConnectionPool() {
    while (!_connQueue.empty()) {
        Connection* p = _connQueue.front();
        _connQueue.pop();
        delete p;
    }
    mysql_library_end();
}
