#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>

#include <mbedtls/base64.h>
#include <mbedtls/md.h>
#include <mbedtls/platform.h>
#include <mbedtls/sha1.h>

#include <json/json.h>


void dump_buf(char* info, uint8_t* buf, uint32_t len);

std::string hmac_sha1(const std::string& key, const std::string& data);

std::string base64_encode(const char* data, size_t data_len);

std::string base64_decode(const char* encoded_data, size_t encoded_data_len);

std::string get_gmt_time();

std::string get_auth_header(const std::string& method, const std::string& access_id,
                            const std::string& access_key, const std::string& resource);

bool str_startswith(const std::string& str, const std::string& prefix);

bool strToJson(const std::string& str, Json::Value& json);

std::string jsonToStr(const Json::Value& json);

// 2022-01-07 21:39:17
std::string get_crt_time();

bool CheckParameter(const std::string& Parameter);

// 2022-05-19T00:00:00.000Z -> 2022-01-07 21:39:17
std::string format_time(const std::string& time_str);

Json::Value handle_comments(std::string id);

Json::Value handle_others(std::string article_id, std::string main_comment_id);

Json::Value get_reply_message(std::string main_comment_id, std::string user_id);

bool is_email_valid(std::string email);

std::string get_user_id(std::string email, std::string username);

#endif