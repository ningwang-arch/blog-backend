#include "util.h"
#include <iostream>
#include <json/json.h>
#include <memory>
#include <regex>
#include <sstream>

#include "src/ConnectionPool/CommonConnectionPool.h"

void dump_buf(char* info, uint8_t* buf, uint32_t len) {
    mbedtls_printf("%s", info);
    for (uint32_t i = 0; i < len; i++) { mbedtls_printf("%02x ", buf[i]); }
    mbedtls_printf("\n");
}

std::string hmac_sha1(const std::string& key, const std::string& data) {
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA1;
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
    mbedtls_md_hmac_starts(&ctx, (const unsigned char*)key.c_str(), key.length());
    mbedtls_md_hmac_update(&ctx, (const unsigned char*)data.c_str(), data.length());
    unsigned char md[MBEDTLS_MD_MAX_SIZE];
    size_t md_len;
    mbedtls_md_hmac_finish(&ctx, md);

    md_len = mbedtls_md_get_size(mbedtls_md_info_from_type(md_type));
    // mbedtls_md_hmac_reset(&ctx);
    // mbedtls_md_hmac_finish(&ctx, md);
    mbedtls_md_free(&ctx);
    return std::string((char*)md, md_len);
}

std::string base64_encode(const char* data, size_t data_len) {
    char base64_buf[256];
    size_t base64_len;
    mbedtls_base64_encode(
        (unsigned char*)base64_buf, 256, &base64_len, (const unsigned char*)data, data_len);
    return std::string(base64_buf, base64_len);
}

std::string base64_decode(const char* encoded_data, size_t encoded_data_len) {
    char base64_buf[256];
    size_t base64_len;
    mbedtls_base64_decode((unsigned char*)base64_buf,
                          256,
                          &base64_len,
                          (const unsigned char*)encoded_data,
                          encoded_data_len);
    return std::string(base64_buf, base64_len);
}

std::string get_gmt_time() {
    time_t rawtime;
    struct tm* timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = gmtime(&rawtime);

    strftime(buffer, 80, "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
    return std::string(buffer);
}

std::string get_auth_header(const std::string& method, const std::string& access_id,
                            const std::string& access_key, const std::string& resource) {
    std::stringstream ss;
    ss << method << "\n";
    ss << "\n";
    ss << "\n";
    ss << get_gmt_time() << "\n";
    ss << resource;

    std::string hmac = hmac_sha1(access_key, ss.str());

    std::string encode = base64_encode(hmac.c_str(), hmac.length());

    ss.clear();

    return "OSS " + access_id + ":" + encode;
}

bool str_startswith(const std::string& str, const std::string& prefix) {
    return str.compare(0, prefix.length(), prefix) == 0;
}

bool strToJson(const std::string& str, Json::Value& json) {
    Json::CharReaderBuilder builder;
    Json::CharReader* reader = builder.newCharReader();

    std::string errors;

    bool parsingSuccessful = reader->parse(str.c_str(), str.c_str() + str.size(), &json, &errors);
    delete reader;

    if (!parsingSuccessful) {
        std::cout << "Failed to parse the JSON, errors:" << std::endl;
        std::cout << errors << std::endl;
        return false;
    }
    return true;
}

std::string jsonToStr(const Json::Value& json) {
    Json::StreamWriterBuilder builder;
    std::string str = Json::writeString(builder, json);
    return str;
}

std::string get_crt_time() {
    time_t t = time(nullptr);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t));
    return std::string(buf);
}

bool CheckParameter(const std::string& Parameter) {
    std::string key[14] = {
        "and", "*", "=", " ", "%0a", "%", "/", "union", "|", "&", "^", "#", "/*", "*/"};
    for (int i = 0; i < 14; i++) {
        if (Parameter.find(key[i]) != std::string::npos) { return false; }
    }
    return true;
}

// 2022-05-19T00:00:00.000Z -> 2022-01-07 21:39:17
std::string format_time(const std::string& time_str) {
    if (time_str.empty()) { return ""; }
    std::string time_str_new = time_str.substr(0, 19);
    time_str_new.replace(10, 1, " ");
    time_str_new.replace(13, 1, ":");
    return time_str_new;
}

/*
                        "comments": [
                            {
                                "_id": 11,
                                "content": "test",
                                "create_time": "2022-01-07 21:39:17",
                                "other_comments": [],
                                "user": {
                                    "avatar": "",
                                    "name": "user",
                                    "type": 1
                                }
                            }
                        ],
                        other_comments: {
                            "id": ""
                            "from": {"user_id": "19", "name": "tc", "type": "0", "avatar": ""},
                            "to": {"user_id": "19", "name": "tc", "type": "0", "avatar": ""},
                            "content": "",
                            "create_time": ""
                        }

*/
Json::Value handle_comments(std::string id) {
    // get comments form MainComment which article_id = id
    ConnectionPool* pool = ConnectionPool::getConnectionPool();
    std::shared_ptr<Connection> conn = pool->getConnection();

    std::string sql = "select * from main_comment where article_id = " + id;

    std::shared_ptr<ResultSet> result = conn->query(sql);
    Result::Ptr res = nullptr;
    Json::Value comments = Json::Value(Json::arrayValue);
    while ((res = result->next()) != nullptr) {
        Json::Value comment;
        if (res->getString("status") == "-1") { continue; }
        comment["_id"] = res->getString("id");
        comment["content"] = res->getString("content");
        comment["create_time"] = format_time(res->getString("created_at"));
        comment["is_handle"] = res->getString("is_handle");
        // get user info from user by user_id
        std::string user_id = res->getString("user_id");
        sql = "select * from user where id = " + user_id;
        std::shared_ptr<ResultSet> result_user = conn->query(sql);
        Result::Ptr res_user = nullptr;
        if ((res_user = result_user->next())) {
            Json::Value user;
            user["name"] = res_user->getString("name");
            user["type"] = res_user->getString("role");
            user["avatar"] = "";
            comment["user"] = user;
        }
        comment["other_comments"] = handle_others(id, comment["_id"].asString());
        comments.append(comment);
    }

    return comments;
}

Json::Value handle_others(std::string article_id, std::string main_comment_id) {
    ConnectionPool* pool = ConnectionPool::getConnectionPool();
    std::shared_ptr<Connection> conn = pool->getConnection();

    // get reply_comment from reply_comment which main_comment_id = main_comment_id and article_id =
    // article_id
    std::string sql = "select * from reply_comment where main_comment_id = " + main_comment_id +
                      " and article_id = " + article_id;
    std::shared_ptr<ResultSet> result = conn->query(sql);
    Result::Ptr res = nullptr;

    Json::Value others = Json::Value(Json::arrayValue);
    while ((res = result->next()) != nullptr) {
        Json::Value other;
        other["id"] = res->getString("id");
        other["content"] = res->getString("content");
        other["create_time"] = format_time(res->getString("created_at"));
        // get user info from user by from_user_id and to_user_id
        std::string from_user_id = res->getString("from_user_id");
        std::string to_user_id = res->getString("to_user_id");
        sql = "select * from user where id = " + from_user_id;
        std::shared_ptr<ResultSet> result_from = conn->query(sql);
        Result::Ptr res_from = nullptr;
        if ((res_from = result_from->next())) {
            Json::Value from;
            from["user_id"] = res_from->getString("id");
            from["name"] = res_from->getString("name");
            from["type"] = res_from->getString("role");
            from["avatar"] = "";
            other["user"] = from;
        }
        sql = "select * from user where id = " + to_user_id;
        std::shared_ptr<ResultSet> result_to = conn->query(sql);
        Result::Ptr res_to = nullptr;
        if ((res_to = result_to->next())) {
            Json::Value to;
            to["user_id"] = res_to->getString("id");
            to["name"] = res_to->getString("name");
            to["type"] = res_to->getString("role");
            to["avatar"] = "";
            other["to_user"] = to;
        }
        others.append(other);
    }

    return others;
}

Json::Value get_reply_message(std::string main_comment_id, std::string user_id) {
    ConnectionPool* pool = ConnectionPool::getConnectionPool();
    std::shared_ptr<Connection> conn = pool->getConnection();

    Json::Value reply_message = Json::Value(Json::arrayValue);
    std::string sql = "select * from reply_comment where main_comment_id = " + main_comment_id +
                      " and to_user_id = " + user_id;
    std::shared_ptr<ResultSet> result = conn->query(sql);
    Result::Ptr res = nullptr;

    while ((res = result->next()) != nullptr) {
        Json::Value message;
        message["_id"] = res->getString("id");
        message["content"] = res->getString("content");

        reply_message.append(message);
    }

    return reply_message;
}

bool is_email_valid(std::string email) {
    std::regex pattern("^[a-zA-Z0-9_-]+@[a-zA-Z0-9_-]+(\\.[a-zA-Z0-9_-]+)+$");
    return std::regex_match(email, pattern);
}

std::string get_user_id(std::string email, std::string username) {
    ConnectionPool* pool = ConnectionPool::getConnectionPool();
    std::shared_ptr<Connection> conn = pool->getConnection();
    if (!CheckParameter(email) || !CheckParameter(username)) { return ""; }

    std::string sql = "select * from user where email = '" + email + "' and name = '" + username +
                      "' and role = 0";
    std::shared_ptr<ResultSet> result = conn->query(sql);
    Result::Ptr res = nullptr;
    if ((res = result->next())) { return res->getString("id"); }


    // username and email not match
    if (!is_email_valid(email)) { return ""; }
    // if email is exist, then return
    sql = "select * from user where email = '" + email + "'";
    result = conn->query(sql);
    if (result->size() > 0) { return ""; }
    // if username is exist, then return
    sql = "select * from user where name = '" + username + "'";
    result = conn->query(sql);
    if (result->size() > 0) { return ""; }
    // if email and username is both not exist, then insert
    std::string crt_time = get_crt_time();
    sql = "insert into user (email, name, role, created_at) values ('" + email + "', '" + username +
          "', 0, '" + crt_time + "')";
    conn->update(sql);
    sql = "select * from user where email = '" + email + "'";
    result = conn->query(sql);
    if ((res = result->next())) { return res->getString("id"); }
    return "";
}

// convert Percent-encoding to UTF-8
std::string url_decode(std::string str) {
    std::string result;
    char ch;
    int i, ii;
    for (i = 0; i < (int)str.length(); i++) {
        if (str[i] != '%') { result.push_back(str[i]); }
        else {
            sscanf(str.substr(i + 1, 2).c_str(), "%x", &ii);
            ch = static_cast<char>(ii);
            result.push_back(ch);
            i = i + 2;
        }
    }
    return result;
}

std::shared_ptr<Connection> get_connection() {
    static ConnectionPool* pool = ConnectionPool::getConnectionPool();
    return pool->getConnection();
}