#include "application/application.hpp"
#include "daemon/daemon.h"

#include <boost/program_options.hpp>

namespace po = boost::program_options;

static std::string address = "";

#ifndef DEFAULT_ADDRESS
#    define DEFAULT_ADDRESS "127.0.0.1:8080"
#endif

int app_start(int argc, const char* argv[]) {
    LOG_INFO("app_start");

    Application app(address);

    app.start();

    return 0;
}


int main(int argc, char const* argv[]) {
    bool is_daemon = false;
    po::options_description desc("options");
    desc.add_options()("help,h", "help message")(
        "addr,a", po::value<std::string>(&address)->default_value(DEFAULT_ADDRESS), "address")(
        "daemon,d", po::bool_switch(&is_daemon), "daemon");
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
