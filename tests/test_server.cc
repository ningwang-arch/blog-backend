#include <iostream>
#include <string>

#include "pico/pico.h"
#include "src/http_server/http_server.hpp"

using request = pico::HttpRequest::Ptr;
using response = pico::HttpResponse::Ptr;

int main(int argc, char const* argv[]) {
    // pico::IOManager iom(2);


    // iom.schedule([]() {
    //     pico::IOManager::Ptr worker(new pico::IOManager(2, false, "worker"));
    //     pico::IOManager::Ptr acceptor(new pico::IOManager(2, false, "acceptor"));
    //     pico::HttpServer<>::Ptr server(new pico::HttpServer<>(true, worker.get(),
    //     acceptor.get()));

    //     server->setName("pico");


    //     pico::Address::Ptr addr = pico::Address::LookupAnyIPAddress("0.0.0.0:8080");
    //     if (!addr) {
    //         LOG_INFO("not found");
    //         return;
    //     }
    //     server->bind(addr);

    //     server->start();
    // });
    HttpServer server("0.0.0.0:8080");

    server.start();
    return 0;
}
