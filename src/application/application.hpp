#ifndef __APPLICATION_APPLICATION_H__
#define __APPLICATION_APPLICATION_H__

#include <memory>
#include <string>

#include "src/ConnectionPool/CommonConnectionPool.h"
#include "src/handlers.h"
#include "src/http_server/http_server.hpp"
#include "src/oss/oss_client.h"
#include "src/util.h"

#include "pico/http/middlewares/cors.h"
#include "pico/http/middlewares/session.h"
#include "pico/http/middlewares/utf-8.h"


using request = pico::HttpRequest::Ptr;
using response = pico::HttpResponse::Ptr;

class Application
{
public:
    explicit Application(const std::string& address)
        : m_server(HttpServer<pico::UTF8, pico::CORSHandler, pico::Session>(address)) {
        m_connection_pool = ConnectionPool::getConnectionPool();
    }
    ~Application() {}

    void init() {

#define XX(path, method, handler) \
    m_server.addRoute(            \
        path, method, [&](const request& req, response& resp) { handler(req, resp, this); });

        XX("/", pico::HttpMethod::GET, root);

        XX("/loginAdmin", pico::HttpMethod::POST, login_handler);

        XX("/currentUser", pico::HttpMethod::GET, currentUser);

        XX("/getCategoryList", pico::HttpMethod::GET, getCategoryList);
        XX("/delCategory", pico::HttpMethod::POST, delCategory);
        XX("/addCategory", pico::HttpMethod::POST, addCategory);

        XX("/getTagList", pico::HttpMethod::GET, getTagList);
        XX("/delTag", pico::HttpMethod::POST, delTag);
        XX("/addTag", pico::HttpMethod::POST, addTag);

        XX("/getProjectList", pico::HttpMethod::GET, getProjectList);
        XX("/delProject", pico::HttpMethod::POST, delProject);
        XX("/addProject", pico::HttpMethod::POST, addProject);
        XX("/getProjectDetail", pico::HttpMethod::POST, getProjectDetail);
        XX("/updateProject", pico::HttpMethod::POST, updateProject);

        XX("/getArticleListAdmin", pico::HttpMethod::GET, getArticleListAdmin);
        XX("/addArticle", pico::HttpMethod::POST, addArticle);
        XX("/getArticleDetail", pico::HttpMethod::POST, getArticleDetail);
        XX("/updateArticle", pico::HttpMethod::POST, updateArticle);
        XX("/delArticle", pico::HttpMethod::POST, delArticle);
        XX("/getArticleList", pico::HttpMethod::GET, getArticleList);

        XX("/changeComment", pico::HttpMethod::POST, changeComment);
        XX("/getMessageList", pico::HttpMethod::GET, getMessageList);
        XX("/getMessageDetail", pico::HttpMethod::POST, getMessageDetail);
        XX("/addReplyMessage", pico::HttpMethod::POST, addReplyMessage);
        XX("/delMessage", pico::HttpMethod::POST, delMessage);
        XX("/changeThirdComment", pico::HttpMethod::POST, changeThirdComment);
        XX("/addComment", pico::HttpMethod::POST, addComment);
        XX("/addThirdComment", pico::HttpMethod::POST, addThirdComment);

#undef XX

        m_server.addGlobalRoute("/log/*", pico::HttpMethod::GET, get_log);
    }

    pico::HttpServer<pico::UTF8, pico::CORSHandler, pico::Session>::Ptr get_server() {
        return m_server.get_server();
    }

    // getConnection
    std::shared_ptr<Connection> getConnection() { return m_connection_pool->getConnection(); }

    void start() {
        init();
        m_server.start();
    }

    void stop() { m_server.stop(); }

private:
    HttpServer<pico::UTF8, pico::CORSHandler, pico::Session> m_server;

    ConnectionPool* m_connection_pool = nullptr;
};

#endif
