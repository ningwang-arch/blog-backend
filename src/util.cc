#include "util.h"
#include <iostream>
#include <json/json.h>
#include <memory>
#include <regex>
#include <sstream>


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


bool is_email_valid(std::string email) {
    std::regex pattern("^[a-zA-Z0-9_-]+@[a-zA-Z0-9_-]+(\\.[a-zA-Z0-9_-]+)+$");
    return std::regex_match(email, pattern);
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
