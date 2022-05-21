#ifndef CONNECTION_H
#define CONNECTION_H

#include "ResultSet.h"
#include <bits/types/clock_t.h>
#include <ctime>
#include <memory>
#include <mutex>
#include <mysql/mysql.h>
#include <string>


class Connection
{
private:
    std::shared_ptr<MYSQL> _conn;
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
    std::shared_ptr<ResultSet> query(std::string sql);

    void refreshAliveTime();

    // // prepare statement
    // std::shared_ptr<MYSQL_STMT> prepare(std::string sql);
    // // bind parameter
    // bool bindParam(std::shared_ptr<MYSQL_STMT> stmt, int index, int value);
    // bool bindParam(std::shared_ptr<MYSQL_STMT> stmt, int index, std::string value);
    // bool bindParam(std::shared_ptr<MYSQL_STMT> stmt, int index, double value);
    // bool bindParam(std::shared_ptr<MYSQL_STMT> stmt, int index, std::tm value);
    // bool bindParam(std::shared_ptr<MYSQL_STMT> stmt, int index, std::string value, int length);

    // // execute statement
    // bool executeUpdate(std::shared_ptr<MYSQL_STMT> stmt);
    // std::shared_ptr<ResultSet> executeQuery(std::shared_ptr<MYSQL_STMT> stmt);


    clock_t getAliveTime();

    bool ping();

    ~Connection();

private:
    std::string m_ip;
    unsigned short m_port;
    std::string m_username;
    std::string m_password;
    std::string m_dbname;
};

#endif /* CONNECTION_H */
