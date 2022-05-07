#include "login_servlet.h"
#include "util.h"

#include "pico/session.h"

void LoginServlet::doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) {
    auto conn = get_connection();

    std::string body = req->get_body();
    Json::Value json;
    if (!strToJson(body, json)) {
        res->set_status(pico::HttpStatus::BAD_REQUEST);
        res->set_body("Bad Request");
        return;
    }

    std::string username = json["email"].asString();
    std::string password = json["password"].asString();

    if (!CheckParameter(username) || !CheckParameter(password)) {
        res->set_status(pico::HttpStatus::BAD_REQUEST);
        res->set_body("invalid username or password");
        return;
    }

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

    std::string sql = "select * from user where name = '" + username + "' and email = '" +
                      password + "' and role = 1";
    auto result = conn->query(sql);
    if (!result) {
        code = 200;
        msg = "database error";
    }
    else if (result->size() == 0) {
        code = 200;
        msg = "user not exist";
    }
    else {
        std::string user_id = result->next()->getString("id");
        session->set("user_id", user_id);
        session->set("username", username);
    }

    json_resp["code"] = code;
    json_resp["message"] = msg;
    json_resp["data"] = "";

    res->set_body(jsonToStr(json_resp));
}