#include "Connection.h"
#include <bits/types/clock_t.h>
#include <cstring>
#include <ctime>
#include <mutex>
#include <mysql/mysql.h>
#include <string>
Connection::Connection() {
    _conn = mysql_init(nullptr);
}

Connection::~Connection() {
    if (_conn != nullptr) { mysql_close(_conn); }
}

bool Connection::connect(string ip, unsigned short port, string username, string password,
                         string dbname) {
    MYSQL* p = mysql_real_connect(
        _conn, ip.c_str(), username.c_str(), password.c_str(), dbname.c_str(), port, nullptr, 0);
    if (p == nullptr) {
        LOG_ERROR("mysql_real_connect error: %s", mysql_error(_conn));
        exit(1);
    }
    return true;
}

bool Connection::update(string sql) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (mysql_real_query(_conn, sql.c_str(), (unsigned long)strlen(sql.c_str()))) {
        std::string log = "Update failed: " + sql + " mysql_error: " + mysql_error(_conn);
        LOG_ERROR("update failed: %s, mysql_error: %s", sql.c_str(), mysql_error(_conn));
        return false;
    }
    return true;
}

ResultSet* Connection::query(string sql) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (mysql_real_query(_conn, sql.c_str(), (unsigned long)strlen(sql.c_str()))) {
        std::string log = "Query failed: " + sql + " mysql_error: " + mysql_error(_conn);
        LOG_ERROR("query failed: %s, mysql_error: %s", sql.c_str(), mysql_error(_conn));
        return nullptr;
    }
    MYSQL_RES* result = mysql_store_result(_conn);
    return new ResultSet(result);
}

void Connection::refreshAliveTime() {
    _alivetime = clock();
}

clock_t Connection::getAliveTime() {
    return clock() - _alivetime;
}
