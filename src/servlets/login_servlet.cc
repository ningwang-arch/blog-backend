#include "login_servlet.h"
#include "util.h"

#include "pico/session.h"

#include "../tables.hpp"

void LoginServlet::doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) {
    std::string body = req->get_body();
    Json::Value json;
    if (!strToJson(body, json)) {
        res->set_status(pico::HttpStatus::BAD_REQUEST);
        res->set_body("Bad Request");
        return;
    }

    std::string username = json.get("email", "").asString();
    std::string password = json.get("password", "").asString();

    Json::Value json_resp;

    auto session = pico::SessionManager::getInstance()->getRequestSession(req, res);
    if (!session) {
        res->set_status(pico::HttpStatus::INTERNAL_SERVER_ERROR);
        res->set_body("session error");
        return;
    }

    int code = 0;
    std::string msg = "success";
    res->set_status(pico::HttpStatus::OK);

    pico::Mapper<User> u_mapper;
    u_mapper.use("sql_1");

    pico::Base<User> base;
    auto criteria = base.createCriteria();
    criteria->andEqualTo(&User::name, username)
        ->andEqualTo(&User::email, password)
        ->andEqualTo(&User::role, 1);

    auto ret = u_mapper.select(base);
    if (ret.size() != 1) {
        code = 200;
        msg = "invalid username or password";
    }
    else {
        User user = ret.front();
        session->set("user_id", user.id);
        session->set("username", user.name);
    }

    json_resp["code"] = code;
    json_resp["message"] = msg;
    json_resp["data"] = "";

    res->set_body(jsonToStr(json_resp));
}