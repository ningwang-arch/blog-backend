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

void dump_buf(char* info, uint8_t* buf, uint32_t len);

std::string hmac_sha1(const std::string& key, const std::string& data);

std::string base64_encode(const char* data, size_t data_len);

std::string base64_decode(const char* encoded_data, size_t encoded_data_len);

std::string get_gmt_time();

std::string get_auth_header(const std::string& method, const std::string& access_id,
                            const std::string& access_key, const std::string& resource);

#endif