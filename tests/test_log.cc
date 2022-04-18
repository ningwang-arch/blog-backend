#include "pico/log/LogAppender.h"
#include "pico/pico.h"

void test() {

    pico::FileAppender::Ptr fileApender(new pico::FileAppender("test.log"));
    fileApender->setLayout(std::shared_ptr<pico::PatternLayout>(new pico::PatternLayout()));

    g_logger->addAppender(fileApender);

    // g_logger->info("Hello World!");

    LOG_INFO("Hello World!");
}

int main(int argc, char const* argv[]) {
    /* code */
    test();
    return 0;
}
