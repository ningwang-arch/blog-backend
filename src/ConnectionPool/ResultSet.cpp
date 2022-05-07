#include "ResultSet.h"
#include <unordered_map>

Result::Result(MYSQL_RES* result, MYSQL_ROW row) {
    int num_fields = mysql_num_fields(result);
    MYSQL_FIELD* field;
    for (int i = 0; i < num_fields; i++) {
        field = mysql_fetch_field_direct(result, i);
        um.insert(std::pair<std::string, std::string>(field->name, row[i]));
    }
}

Result::Result() {}

bool Result::equals(const Result* r) {
    if (um.size() != r->um.size()) return false;
    std::unordered_map<std::string, std::string>::const_iterator it = um.begin();
    while (it != um.end()) {
        std::unordered_map<std::string, std::string>::const_iterator it2 = r->um.find(it->first);
        if (it2 != r->um.end()) {
            if (it2->second != it->second) return false;
        }
        else {
            return false;
        }
        it++;
    }
    return true;
}

Result::Result(const Result* r) {
    for (std::unordered_map<std::string, std::string>::const_iterator it = r->um.begin();
         it != r->um.end();
         it++) {
        um.insert(std::pair<std::string, std::string>(it->first, it->second));
    }
}

std::string Result::getValue(const std::string key) {
    if (um.find(key) == um.end()) {
        LOG_ERROR("key '%s' not found", key.c_str());
        return "";
    }
    return um.find(key)->second;
}

int Result::getInt(const std::string key) {
    if (getValue(key) == "") { return NO_SUCH_KEY; }
    return stoi(getValue(key));
}

double Result::getDouble(const std::string key) {
    if (getValue(key) == "") { return NO_SUCH_KEY; }
    return stod(getValue(key));
}

std::string Result::getString(const std::string key) {
    return getValue(key);
}

bool Result::getBool(const std::string key) {
    int value = getInt(key);
    if (value == NO_SUCH_KEY) { return false; }
    return value == 1;
}

Result::~Result() {
    um.clear();
}

ResultSet::ResultSet(MYSQL_RES* result) {
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        Result* r = new Result(result, row);
        v.push_back(r);
    }
    it = v.begin();
}

ResultSet::ResultSet() {
    v.clear();
    it = v.begin();
}

ResultSet::~ResultSet() {
    for (it = v.begin(); it != v.end(); it++) { delete *it; }
    v.clear();
}

Result* ResultSet::next() {
    if (it == v.end()) { return nullptr; }
    return *it++;
}
int ResultSet::size() {
    return v.size();
}

void ResultSet::reset() {
    for (it = v.begin(); it != v.end(); it++) { delete *it; }
    v.clear();
}

bool ResultSet::isElement(Result* r) {
    for (it = v.begin(); it != v.end(); it++) {
        if (r->equals(*it)) { return true; }
    }
    return false;
}

void ResultSet::insert(Result* r) {
    Result* r2 = new Result(r);
    v.push_back(r2);
}

void ResultSet::offset(int start, int cnt) {
    if (start < 0) { start = 0; }
    if (cnt < 0) { cnt = 0; }
    if (start + cnt > (int)v.size()) { cnt = v.size() - start; }
    it = v.begin();

    std::vector<Result*> v2;
    v2.assign(v.begin() + start, v.begin() + start + cnt);
    v.clear();
    v.assign(v2.begin(), v2.end());
    v2.clear();
}

ResultSet::ResultSet(const ResultSet& rs) {
    for (it = rs.v.begin(); it != rs.v.end(); it++) {
        Result* r = new Result(*it);
        v.push_back(r);
    }
    it = v.begin();
}

ResultSet& ResultSet::operator=(const ResultSet& rs) {
    for (it = v.begin(); it != v.end(); it++) { delete *it; }
    for (it = rs.v.begin(); it != rs.v.end(); it++) {
        Result* r = new Result(*it);
        v.push_back(r);
    }
    it = v.begin();
    return *this;
}