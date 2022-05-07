#include "Connection.h"
#include <bits/types/clock_t.h>
#include <cstring>
#include <ctime>
#include <mutex>
#include <mysql/errmsg.h>
#include <mysql/mysql.h>
#include <string>
Connection::Connection(std::string ip, unsigned short port, std::string username,
                       std::string password, std::string dbname)
    : m_ip(ip)
    , m_port(port)
    , m_username(username)
    , m_password(password)
    , m_dbname(dbname) {
    _conn = mysql_init(nullptr);
    char value = 1;
    mysql_options(_conn, MYSQL_OPT_RECONNECT, &value);
}

Connection::~Connection() {
    if (_conn != nullptr) { mysql_close(_conn); }
}

bool Connection::connect() {
    MYSQL* p = mysql_real_connect(_conn,
                                  m_ip.c_str(),
                                  m_username.c_str(),
                                  m_password.c_str(),
                                  m_dbname.c_str(),
                                  m_port,
                                  nullptr,
                                  0);
    if (p == nullptr) {
        LOG_ERROR("mysql_real_connect error: %s", mysql_error(_conn));
        exit(1);
    }
    return true;
}

void Connection::reconnect() {
    std::mutex m;
    std::lock_guard<std::mutex> lock(m);
    if (_conn != nullptr) { mysql_close(_conn); }
    _conn = mysql_init(nullptr);
    char value = 1;
    mysql_options(_conn, MYSQL_OPT_RECONNECT, &value);
    connect();
    m.unlock();
}

bool Connection::update(std::string sql) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (mysql_ping(_conn)) { reconnect(); }
    if (mysql_real_query(_conn, sql.c_str(), (unsigned long)strlen(sql.c_str()))) {
        if (mysql_errno(_conn) == CR_SERVER_GONE_ERROR || mysql_errno(_conn) == CR_SERVER_LOST) {
            reconnect();
            if (!reexecute(sql)) {
                LOG_ERROR("update failed, mysql_error: %s", mysql_error(_conn));
                return false;
            }
        }
        LOG_ERROR("update failed, mysql_error: %s", mysql_error(_conn));
        return false;
    }
    return true;
}

ResultSet* Connection::query(std::string sql) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (mysql_ping(_conn)) { reconnect(); }
    if (mysql_real_query(_conn, sql.c_str(), (unsigned long)strlen(sql.c_str()))) {
        if (mysql_errno(_conn) == CR_SERVER_GONE_ERROR || mysql_errno(_conn) == CR_SERVER_LOST) {
            reconnect();
            if (!reexecute(sql)) {
                LOG_ERROR("query failed, mysql_error: %s", mysql_error(_conn));
                return new ResultSet();
            }
        }
        else {
            LOG_ERROR("query failed, mysql_error: %s", mysql_error(_conn));
            return new ResultSet();
        }
    }
    MYSQL_RES* result = mysql_store_result(_conn);
    return new ResultSet(result);
}

bool Connection::reexecute(std::string sql) {
    std::mutex ctx;
    std::lock_guard<std::mutex> lock(ctx);

    if (mysql_real_query(_conn, sql.c_str(), (unsigned long)strlen(sql.c_str()))) {
        ctx.unlock();
        return false;
    }
    ctx.unlock();
    return true;
}

void Connection::refreshAliveTime() {
    _alivetime = clock();
}

clock_t Connection::getAliveTime() {
    return clock() - _alivetime;
}
