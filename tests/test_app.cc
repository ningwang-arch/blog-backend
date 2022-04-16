#include "src/application/application.hpp"

int main(int argc, char const* argv[]) {
    Application app("0.0.0.0:8080");

    // app.init();
    app.start();
    return 0;
}
