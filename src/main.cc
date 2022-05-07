#include "application/application.hpp"
#include "daemon/daemon.h"

#include "pico/config.h"
#include <boost/program_options.hpp>

#include "pico/log/LogAppender.h"

#include "pico/macro.h"


namespace po = boost::program_options;

static std::string address = "";

static pico::ConfigVar<std::string>::Ptr g_server_addr =
    pico::Config::Lookup<std::string>(CONF_ROOT "server.address", "127.0.0.1", "server address");

static pico::ConfigVar<std::string>::Ptr g_server_port =
    pico::Config::Lookup<std::string>(CONF_ROOT "server.port", "8080", "server port");

static pico::ConfigVar<std::string>::Ptr g_blog_log_path =
    pico::Config::Lookup<std::string>(CONF_ROOT "log.blog.path", std::string(), "path of blog log");

static pico::ConfigVar<std::string>::Ptr g_blog_log_file = pico::Config::Lookup<std::string>(
    CONF_ROOT "log.blog.file", std::string(), "filename of blog log");

int app_start(int argc, const char* argv[]) {
    LOG_INFO("app_start");

    Application app(g_server_addr->getValue() + ":" + g_server_port->getValue());

    app.start();

    return 0;
}


int main(int argc, char const* argv[]) {
    pico::Config::LoadFromFile("web.yml");

    pico::FileAppender::Ptr fileAppender(
        new pico::FileAppender(g_blog_log_path->getValue() + g_blog_log_file->getValue()));

    pico::PatternLayout::Ptr layout(new pico::PatternLayout());
    layout->setPattern("[%d{%Y-%m-%d %H:%M:%S}] %p %m");

    fileAppender->setLayout(layout);

    g_logger->addAppender(fileAppender);

    bool is_daemon = false;
    po::options_description desc("options");
    desc.add_options()("help,h", "help message")("daemon,d", po::bool_switch(&is_daemon), "daemon");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }

    start_daemon(
        argc, argv, std::bind(app_start, std::placeholders::_1, std::placeholders::_2), is_daemon);
    return 0;
}
