#ifndef CONNECTION_H
#define CONNECTION_H

#include "ResultSet.h"
#include <bits/types/clock_t.h>
#include <ctime>
#include <mutex>
#include <mysql/mysql.h>
#include <string>


class Connection
{
private:
    MYSQL* _conn;
    clock_t _alivetime;
    std::mutex _mutex;

public:
    Connection(std::string ip, unsigned short port, std::string username, std::string password,
               std::string dbname);

    // connect to database
    bool connect();

    // insert delete update
    bool update(std::string sql);

    // select
    ResultSet* query(std::string sql);

    void refreshAliveTime();

    clock_t getAliveTime();

    void reconnect();

    ~Connection();

private:
    bool reexecute(std::string sql);
    std::string m_ip;
    unsigned short m_port;
    std::string m_username;
    std::string m_password;
    std::string m_dbname;
};

#endif /* CONNECTION_H */
