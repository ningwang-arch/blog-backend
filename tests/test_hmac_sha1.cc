#include "pico/pico.h"
#include "src/util.h"

void dump_buf(char* info, uint8_t* buf, uint32_t len) {
    mbedtls_printf("%s", info);
    for (int i = 0; i < (int)len; i++) {
        mbedtls_printf(
            "%s%02X%s", i % 16 == 0 ? "\n     " : " ", buf[i], i == (int)(len - 1) ? "\n" : "");
    }
}

void test() {
    char key[] = "AzHFefiEv0gekoS45TtMJ8NJdK3sR2";

    std::string date = get_gmt_time();

    std::string msg = "GET\n\n\n" + date + "\n/pico-img/.vimrc";

    std::string hmac = hmac_sha1(key, msg);

    std::string encoded = base64_encode(hmac.c_str(), hmac.length());

    std::string decoded = base64_decode(encoded.c_str(), encoded.length());

    mbedtls_printf("msg: %s\n", msg.c_str());
    mbedtls_printf("hmac: %s\n", hmac.c_str());
    dump_buf((char*)"hmac_sha1: ", (uint8_t*)hmac.c_str(), hmac.length());
    mbedtls_printf("encoded: %s\n", encoded.c_str());
    mbedtls_printf("decoded: %s\n", decoded.c_str());

    std::map<std::string, std::string> headers;

    headers["Date"] = date;
    headers["Authorization"] = "OSS LTAI5tHE9RBu9yXVVzVTPG7i:" + encoded;
    headers["Host"] = "pico-img.oss-cn-beijing.aliyuncs.com";

    pico::HttpResponse::Ptr resp =
        pico::HttpConnection::doGet("http://pico-img.oss-cn-beijing.aliyuncs.com/.vimrc", headers);

    std::cout << "status: " << (int)resp->get_status() << std::endl;
    std::cout << "body: " << resp->get_body() << std::endl;
}

int main(int argc, char const* argv[]) {
    test();
    return 0;
}