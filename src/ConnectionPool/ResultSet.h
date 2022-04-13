#ifndef __RESULT_SET_H__
#define __RESULT_SET_H__

#include "pico/pico.h"
#include <iostream>
#include <mysql/mysql.h>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

#define NO_SUCH_KEY 1001

class Result
{
private:
    unordered_map<string, string> um;
    string getValue(const string key);

public:
    Result(MYSQL_RES* result, MYSQL_ROW row);
    Result(const Result* r);
    Result(const Result& r);
    Result();
    int getInt(const string key);
    string getString(const string key);
    double getDouble(const string key);
    bool getBool(const string key);
    bool equals(const Result* r);
    ~Result();
};

class ResultSet
{
private:
    vector<Result*> v;
    vector<Result*>::const_iterator it;
    bool isElement(Result* r);

public:
    ResultSet();
    ResultSet(MYSQL_RES* result);
    ResultSet(const ResultSet& rs);
    Result* next();
    int size();
    void reset();
    void insert(Result* r);

    ResultSet* offset(int start, int cnt);

    ResultSet& operator=(const ResultSet& rs);

    ~ResultSet();
};

#endif
