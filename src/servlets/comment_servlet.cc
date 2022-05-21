#include "comment_servlet.h"

#include "pico/session.h"
#include "util.h"

#include <memory>

void ChangeCommentServlet::doPost(const pico::HttpRequest::Ptr& req,
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


    std::string id = body["id"].asString();
    std::string status = body["state"].asString();
    std::string sql = "update main_comment set status = " + status + " where id = " + id;
    if (!conn->update(sql)) {
        code = 500;
        message = "update failed";
    }

    Json::Value resp_json;
    resp_json["code"] = code;
    resp_json["message"] = message;
    resp_json["data"] = data;

    resp->set_body(resp_json.toStyledString());
}

void ChangeThirdCommentServlet::doPost(const pico::HttpRequest::Ptr& req,
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

    std::string main_comment_id = body["id"].asString();
    std::string reply_comment_id = body["_id"].asString();

    if (!CheckParameter(main_comment_id) || !CheckParameter(reply_comment_id)) {
        code = 200;
        message = "invalid request";
    }
    else {
        std::string sql = "select * from reply_comment where id = " + reply_comment_id +
                          " and main_comment_id = " + main_comment_id;
        std::shared_ptr<ResultSet> rs = conn->query(sql);

        if (rs->size() == 0) {
            code = 200;
            message = "message not found";
        }
        else {
            std::string article_id = rs->next()->getValue("article_id");
            std::string del_sql = "delete from reply_comment where id = " + reply_comment_id +
                                  " and main_comment_id = " + main_comment_id;
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
    }

    Json::Value resp_json;
    resp_json["code"] = code;
    resp_json["message"] = message;
    resp_json["data"] = data;

    resp->set_body(jsonToStr(resp_json));
}

void AddCommentServlet::doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& resp) {
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

    std::string article_id = body["article_id"].asString();
    Json::Value comment_data = body["data"];

    std::string comment_content = comment_data["content"].asString();
    std::string comment_username = comment_data["username"].asString();
    std::string comment_email = comment_data["email"].asString();

    if (!CheckParameter(article_id) || !CheckParameter(comment_content) ||
        !CheckParameter(comment_username) || !CheckParameter(comment_email)) {
        code = 200;
        message = "invalid request";
    }
    else {
        std::string user_id = get_user_id(comment_email, comment_username);
        if (user_id.empty()) {
            code = 200;
            message = "user not found";
        }
        else {
            std::string sql =
                "insert into main_comment(article_id,user_id,content,status,created_at,updated_at) "
                "values(" +
                article_id + "," + user_id + ",'" + comment_content + "',1,now(),now())";
            if (!conn->update(sql)) {
                code = 200;
                message = "add comment failed";
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

    Json::Value resp_json;
    resp_json["code"] = code;
    resp_json["message"] = message;
    resp_json["data"] = data;

    resp->set_body(jsonToStr(resp_json));
}

void AddThirdCommentServlet::doPost(const pico::HttpRequest::Ptr& req,
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

    std::string article_id = body["article_id"].asString();
    std::string comment_id = body["comment_id"].asString();
    std::string content = body["content"].asString();

    Json::Value from_user, to_user;
    if (!strToJson(body["from_user"].asString(), from_user) ||
        !strToJson(body["to_user"].asString(), to_user)) {
        code = 200;
        message = "invalid request";
    }
    else {
        std::string from_user_id =
            get_user_id(from_user["email"].asString(), from_user["username"].asString());
        // get to_user id from users table
        std::string to_user_name = to_user["name"].asString();
        std::string query_sql =
            "select id from user where name = '" + to_user_name + "' and role = 0";
        std::shared_ptr<ResultSet> rs = conn->query(query_sql);
        Result::Ptr res = nullptr;
        if ((res = rs->next()) == nullptr) {
            code = 200;
            message = "to_user not found";
        }
        else {
            std::string to_user_id = res->getValue("id");
            // insert int reply_comment
            std::string sql = "insert into "
                              "reply_comment(article_id,main_comment_id,from_user_id,to_user_id,"
                              "content,created_at) "
                              "values(" +
                              article_id + "," + comment_id + "," + from_user_id + "," +
                              to_user_id + ",'" + content + "',now())";
            if (!conn->update(sql)) {
                code = 200;
                message = "add third comment failed";
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

    Json::Value resp_json;
    resp_json["code"] = code;
    resp_json["message"] = message;
    resp_json["data"] = data;

    resp->set_body(jsonToStr(resp_json));
}