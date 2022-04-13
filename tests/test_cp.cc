#include "pico/pico.h"
#include "src/ConnectionPool/CommonConnectionPool.h"
#include <memory>

ConnectionPool* pool = ConnectionPool::getConnectionPool();

void test() {
    std::shared_ptr<Connection> sp = pool->getConnection();

    ResultSet* rs = sp->query("select * from t1 limit 100");
    if (rs == nullptr) {
        LOG_ERROR("query failed");
        return;
    }

    Result* r = nullptr;

    while ((r = rs->next()) != nullptr) {
        int id = r->getInt("id");
        if (id == NO_SUCH_KEY) {
            cout << "Key error" << endl;
            break;
        }
        cout << r->getInt("id") << " " << r->getString("name") << endl;
    }
}

int main(int argc, char const* argv[]) {
    pico::IOManager iom;
    iom.addTimer(1000, test, true);
    return 0;
}
