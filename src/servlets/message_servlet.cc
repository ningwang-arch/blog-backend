#include "message_servlet.h"

#include "pico/session.h"
#include "util.h"

void GetMessageListServlet::doGet(const pico::HttpRequest::Ptr& req,
                                  pico::HttpResponse::Ptr& resp) {
    auto conn = get_connection();

    int code = 0;
    std::string message = "success";
    Json::Value data = {};


    std::string keyword = req->get_param("keyword");
    int pageNum = std::stoi(req->get_param("pageNum"));
    int pageSize = std::stoi(req->get_param("pageSize"));
    std::string sql = "select * from main_comment where content like '%" + keyword +
                      "%' and status!=-1 order by created_at desc limit " +
                      std::to_string((pageNum - 1) * pageSize) + "," + std::to_string(pageSize);
    std::shared_ptr<ResultSet> rs = conn->query(sql);
    data["count"] = rs->size();
    data["list"] = Json::Value(Json::arrayValue);
    Result::Ptr ret = nullptr;
    while ((ret = rs->next())) {
        Json::Value item;
        item["_id"] = ret->getValue("id");
        item["user_id"] = ret->getValue("user_id");
        item["content"] = ret->getValue("content");
        item["state"] = ret->getValue("status");
        item["create_time"] = ret->getValue("created_at");
        item["update_time"] = ret->getValue("updated_at");
        item["avatar"] = "user";
        item["reply_list"] = get_reply_message(ret->getValue("id"), ret->getValue("user_id"));
        // get user info
        sql = "select * from user where id = " + ret->getValue("user_id");
        std::shared_ptr<ResultSet> rs_user = conn->query(sql);
        if (rs_user->size() > 0) {
            Result::Ptr res_user = rs_user->next();
            item["name"] = res_user->getValue("name");
            item["email"] = res_user->getValue("email");
        }
        data["list"].append(item);
    }

    Json::Value root;
    root["code"] = code;
    root["message"] = message;
    root["data"] = data;

    resp->set_body(jsonToStr(root));
}

void GetMessageDetailServlet::doPost(const pico::HttpRequest::Ptr& req,
                                     pico::HttpResponse::Ptr& resp) {
    auto conn = get_connection();

    Json::Value body;
    if (!strToJson(req->get_body(), body)) {
        resp->set_status(pico::HttpStatus::BAD_REQUEST);
        resp->set_body("invalid request");
        return;
    }

    int code = 0;
    std::string message = "success";
    Json::Value data = {};

    std::string sql = "select * from main_comment where id = " + body["id"].asString();
    std::shared_ptr<ResultSet> rs = conn->query(sql);
    if (rs->size() <= 0) {
        code = 200;
        message = "not found";
    }
    else {
        Result::Ptr ret = rs->next();
        data["_id"] = ret->getValue("id");
        data["user_id"] = ret->getValue("user_id");
        data["content"] = ret->getValue("content");
        data["state"] = ret->getValue("status");
        data["create_time"] = ret->getValue("created_at");
        data["update_time"] = ret->getValue("updated_at");
        data["avatar"] = "user";
        // get user info
        sql = "select * from user where id = " + ret->getValue("user_id");
        std::shared_ptr<ResultSet> rs_user = conn->query(sql);
        if (rs_user->size() > 0) {
            Result::Ptr res_user = rs_user->next();
            data["name"] = res_user->getValue("name");
            data["email"] = res_user->getValue("email");
        }
        data["reply_list"] = get_reply_message(ret->getValue("id"), ret->getValue("user_id"));
    }

    Json::Value root;
    root["code"] = code;
    root["message"] = message;
    root["data"] = data;

    resp->set_body(jsonToStr(root));
}

void AddReplyMessageServlet::doPost(const pico::HttpRequest::Ptr& req,
                                    pico::HttpResponse::Ptr& resp) {
    auto conn = get_connection();

    Json::Value body;
    if (!strToJson(req->get_body(), body)) {
        resp->set_status(pico::HttpStatus::BAD_REQUEST);
        resp->set_body("invalid request");
        return;
    }

    int code = 0;
    std::string message = "success";
    Json::Value data = {};

    auto session = pico::SessionManager::getInstance()->getRequestSession(req, resp);
    if (!session || !session->has("username")) {
        code = 200;
        message = "not login";
    }
    else {
        std::string id = body["id"].asString();
        std::string content = body["content"].asString();

        std::string current_user_id = session->get<std::string>("user_id");

        std::string query_sql = "select article_id ,user_id from main_comment where id = " + id;
        std::shared_ptr<ResultSet> rs = conn->query(query_sql);
        if (rs->size() == 0) {
            code = 200;
            message = "message not found";
        }
        else {
            Result::Ptr res = rs->next();
            std::string article_id = res->getValue("article_id");
            std::string user_id = res->getValue("user_id");
            std::string insert_sql = "insert into reply_comment "
                                     "(article_id,main_comment_id,from_user_id,to_user_id,"
                                     "content,created_at) values (" +
                                     article_id + "," + id + "," + current_user_id + "," + user_id +
                                     ",'" + content + "',now())";
            if (!conn->update(insert_sql)) {
                code = 200;
                message = "insert reply message failed";
            }
            else {
                std::string update_sql =
                    "update article set comments = comments + 1 where id = " + article_id;
                if (!conn->update(update_sql)) {
                    code = 200;
                    message = "update article comments failed";
                }
            }
        }
    }

    Json::Value root;
    root["code"] = code;
    root["message"] = message;

    resp->set_body(jsonToStr(root));
}

void DelMessageServlet::doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& resp) {
    auto conn = get_connection();

    Json::Value body;
    if (!strToJson(req->get_body(), body)) {
        resp->set_status(pico::HttpStatus::BAD_REQUEST);
        resp->set_body("invalid request");
        return;
    }

    int code = 0;
    std::string message = "success";
    Json::Value data = {};


    std::string id = body["id"].asString();
    std::string sql = "select * from main_comment where id = " + id;
    std::shared_ptr<ResultSet> rs = conn->query(sql);
    if (rs->size() == 0) {
        code = 200;
        message = "message not found";
    }
    else {
        Result::Ptr res = rs->next();
        std::string user_id = res->getValue("user_id");
        std::string article_id = res->getValue("article_id");
        std::string del_sql = "delete from main_comment where id = " + id;
        if (!conn->update(del_sql)) {
            code = 200;
            message = "delete message failed";
        }
        else {
            std::string update_sql =
                "update article set comments = comments - 1 where id = " + article_id;
            if (!conn->update(update_sql)) {
                code = 200;
                message = "update article comments failed";
            }
        }
    }

    Json::Value root;
    root["code"] = code;
    root["message"] = message;
    root["data"] = data;

    resp->set_body(jsonToStr(root));
}