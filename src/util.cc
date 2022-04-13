#include "util.h"
#include <iostream>
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