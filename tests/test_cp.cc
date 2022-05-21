#include "pico/pico.h"
#include "src/ConnectionPool/CommonConnectionPool.h"
#include <memory>

ConnectionPool* pool = ConnectionPool::getConnectionPool();

void test() {
    std::shared_ptr<Connection> sp = pool->getConnection();

    std::shared_ptr<ResultSet> rs = sp->query("select * from t1 limit 100");
    if (rs == nullptr) {
        LOG_ERROR("query failed");
        return;
    }

    Result::Ptr r = nullptr;

    while ((r = rs->next()) != nullptr) {
        int id = r->getInt("id");
        if (id == NO_SUCH_KEY) {
            std::cout << "Key error" << std::endl;
            break;
        }
        std::cout << r->getInt("id") << " " << r->getString("name") << std::endl;
    }
}

int main(int argc, char const* argv[]) {
    pico::IOManager iom;
    iom.addTimer(1000, test, true);
    return 0;
}
