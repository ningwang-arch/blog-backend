#include "Connection.h"
#include <bits/types/clock_t.h>
#include <cstring>
#include <ctime>
#include <mutex>
#include <mysql/errmsg.h>
#include <mysql/mysql.h>
#include <string>

namespace {
struct MySQLThreadIniter
{
    MySQLThreadIniter() { mysql_thread_init(); }

    ~MySQLThreadIniter() { mysql_thread_end(); }
};
}   // namespace

static MYSQL* mysql_init(std::string host, unsigned short port, std::string user,
                         std::string passwd, std::string dbname, const int& timeout) {

    static thread_local MySQLThreadIniter s_thread_initer;

    MYSQL* mysql = ::mysql_init(nullptr);
    if (mysql == nullptr) {
        LOG_ERROR("mysql_init failed");
        return nullptr;
    }

    if (timeout > 0) { mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT, &timeout); }
    bool close = true;
    mysql_options(mysql, MYSQL_OPT_RECONNECT, &close);
    mysql_options(mysql, MYSQL_SET_CHARSET_NAME, "utf8mb4");


    if (mysql_real_connect(
            mysql, host.c_str(), user.c_str(), passwd.c_str(), dbname.c_str(), port, nullptr, 0) ==
        nullptr) {
        LOG_ERROR("mysql_real_connect failed: %s", mysql_error(mysql));
        mysql_close(mysql);
        return nullptr;
    }
    return mysql;
}

Connection::Connection(std::string ip, unsigned short port, std::string username,
                       std::string password, std::string dbname)
    : m_ip(ip)
    , m_port(port)
    , m_username(username)
    , m_password(password)
    , m_dbname(dbname) {}

Connection::~Connection() {
    if (_conn != nullptr) { mysql_close(_conn.get()); }
}

bool Connection::connect() {
    if (_conn) { return true; }
    MYSQL* m = mysql_init(m_ip, m_port, m_username, m_password, m_dbname, 0);
    if (m == nullptr) {
        LOG_ERROR("mysql_init failed");
        return false;
    }
    _conn.reset(m, [](MYSQL* m) { mysql_close(m); });
    return true;
}

bool Connection::ping() {
    if (!_conn) { return false; }
    if (mysql_ping(_conn.get())) { return false; }
    return true;
}

bool Connection::update(std::string sql) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (::mysql_real_query(_conn.get(), sql.c_str(), (unsigned long)strlen(sql.c_str()))) {
        LOG_ERROR("update failed, mysql_error: %s", mysql_error(_conn.get()));
        return false;
    }
    return true;
}

std::shared_ptr<ResultSet> Connection::query(std::string sql) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (::mysql_real_query(_conn.get(), sql.c_str(), (unsigned long)strlen(sql.c_str()))) {
        LOG_ERROR("query failed, mysql_error: %s", mysql_error(_conn.get()));
        return std::make_shared<ResultSet>();
    }
    MYSQL_RES* result = mysql_store_result(_conn.get());
    return std::make_shared<ResultSet>(result);
}


std::shared_ptr<MYSQL_STMT> Connection::prepare(std::string sql) {
    std::lock_guard<std::mutex> lock(_mutex);
    MYSQL_STMT* stmt = mysql_stmt_init(_conn.get());
    if (stmt == nullptr) {
        LOG_ERROR("mysql_stmt_init failed: %s", mysql_error(_conn.get()));
        return nullptr;
    }
    if (::mysql_stmt_prepare(stmt, sql.c_str(), (unsigned long)strlen(sql.c_str()))) {
        LOG_ERROR("mysql_stmt_prepare failed: %s", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return nullptr;
    }
    return std::make_shared<MYSQL_STMT>(stmt);
}

bool Connection::bindParam(std::shared_ptr<MYSQL_STMT> stmt, int index, int value) {
    std::lock_guard<std::mutex> lock(_mutex);
    MYSQL_BIND bind;
    memset(&bind, 0, sizeof(bind));
    bind.buffer_type = MYSQL_TYPE_LONG;
    bind.buffer = (char*)&value;
    bind.buffer_length = sizeof(value);
    if (::mysql_stmt_bind_param(stmt.get(), &bind)) {
        LOG_ERROR("mysql_stmt_bind_param failed: %s", mysql_stmt_error(stmt.get()));
        return false;
    }
    return true;
}

bool Connection::bindParam(std::shared_ptr<MYSQL_STMT> stmt, int index, std::string value) {
    std::lock_guard<std::mutex> lock(_mutex);
    MYSQL_BIND bind;
    memset(&bind, 0, sizeof(bind));
    bind.buffer_type = MYSQL_TYPE_STRING;
    bind.buffer = (char*)value.c_str();
    bind.buffer_length = (unsigned long)value.length();
    if (::mysql_stmt_bind_param(stmt.get(), &bind)) {
        LOG_ERROR("mysql_stmt_bind_param failed: %s", mysql_stmt_error(stmt.get()));
        return false;
    }
    return true;
}

bool Connection::bindParam(std::shared_ptr<MYSQL_STMT> stmt, int index, double value) {
    std::lock_guard<std::mutex> lock(_mutex);
    MYSQL_BIND bind;
    memset(&bind, 0, sizeof(bind));
    bind.buffer_type = MYSQL_TYPE_DOUBLE;
    bind.buffer = (char*)&value;
    bind.buffer_length = sizeof(value);
    if (::mysql_stmt_bind_param(stmt.get(), &bind)) {
        LOG_ERROR("mysql_stmt_bind_param failed: %s", mysql_stmt_error(stmt.get()));
        return false;
    }
    return true;
}

bool Connection::bindParam(std::shared_ptr<MYSQL_STMT> stmt, int index, std::tm value) {
    std::lock_guard<std::mutex> lock(_mutex);
    MYSQL_BIND bind;
    memset(&bind, 0, sizeof(bind));
    bind.buffer_type = MYSQL_TYPE_DATETIME;
    bind.buffer = (char*)&value;
    bind.buffer_length = sizeof(value);
    if (::mysql_stmt_bind_param(stmt.get(), &bind)) {
        LOG_ERROR("mysql_stmt_bind_param failed: %s", mysql_stmt_error(stmt.get()));
        return false;
    }
    return true;
}

bool Connection::bindParam(std::shared_ptr<MYSQL_STMT> stmt, int index, std::string value,
                           int length) {
    std::lock_guard<std::mutex> lock(_mutex);
    MYSQL_BIND bind;
    memset(&bind, 0, sizeof(bind));
    bind.buffer_type = MYSQL_TYPE_STRING;
    bind.buffer = (char*)value.c_str();
    bind.buffer_length = length;
    if (::mysql_stmt_bind_param(stmt.get(), &bind)) {
        LOG_ERROR("mysql_stmt_bind_param failed: %s", mysql_stmt_error(stmt.get()));
        return false;
    }
    return true;
}

bool Connection::executeUpdate(std::shared_ptr<MYSQL_STMT> stmt) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (::mysql_stmt_execute(stmt.get())) {
        LOG_ERROR("mysql_stmt_execute failed: %s", mysql_stmt_error(stmt.get()));
        return false;
    }
    return true;
}

std::shared_ptr<ResultSet> Connection::executeQuery(std::shared_ptr<MYSQL_STMT> stmt) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (::mysql_stmt_execute(stmt.get())) {
        LOG_ERROR("mysql_stmt_execute failed: %s", mysql_stmt_error(stmt.get()));
        return std::make_shared<ResultSet>();
    }
    MYSQL_RES* result = mysql_stmt_result_metadata(stmt.get());
    return std::make_shared<ResultSet>(result);
}

void Connection::refreshAliveTime() {
    _alivetime = clock();
}

clock_t Connection::getAliveTime() {
    return clock() - _alivetime;
}
