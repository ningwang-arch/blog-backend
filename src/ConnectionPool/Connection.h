#ifndef CONNECTION_H
#define CONNECTION_H

#include "ResultSet.h"
#include <bits/types/clock_t.h>
#include <ctime>
#include <mutex>
#include <mysql/mysql.h>
#include <string>

using namespace std;

class Connection
{
private:
    MYSQL* _conn;
    clock_t _alivetime;
    mutex _mutex;

public:
    Connection();

    // connect to database
    bool connect(string ip, unsigned short port, string username, string password, string dbname);

    // insert delete update
    bool update(string sql);

    // select
    ResultSet* query(string sql);

    void refreshAliveTime();

    clock_t getAliveTime();

    ~Connection();
};

#endif /* CONNECTION_H */
