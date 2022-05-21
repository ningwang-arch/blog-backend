#ifndef __RESULT_SET_H__
#define __RESULT_SET_H__

#include "pico/logging.h"
#include <iostream>
#include <memory>
#include <mysql/mysql.h>
#include <string>
#include <unordered_map>
#include <vector>

#define NO_SUCH_KEY 1001

class Result
{
private:
    std::unordered_map<std::string, std::string> um;


public:
    typedef std::shared_ptr<Result> Ptr;
    std::string getValue(const std::string key);
    Result(MYSQL_RES* result, MYSQL_ROW row);
    Result(const std::shared_ptr<Result> r);
    Result(const Result& r);
    Result();
    int getInt(const std::string key);
    std::string getString(const std::string key);
    double getDouble(const std::string key);
    bool getBool(const std::string key);
    bool equals(const Result::Ptr r);
    ~Result();
};

class ResultSet
{
private:
    std::vector<Result::Ptr> v;
    std::vector<Result::Ptr>::const_iterator it;
    bool isElement(Result::Ptr r);

public:
    typedef std::shared_ptr<ResultSet> Ptr;
    ResultSet();
    ResultSet(MYSQL_RES* result);
    ResultSet(const ResultSet& rs);
    Result::Ptr next();
    int size();
    void reset();
    void insert(Result::Ptr r);

    void offset(int start, int cnt);

    ResultSet& operator=(const ResultSet& rs);

    ~ResultSet();
};

#endif
