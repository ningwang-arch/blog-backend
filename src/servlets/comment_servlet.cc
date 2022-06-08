#include "comment_servlet.h"

#include "../tables.hpp"
#include "pico/session.h"
#include "pico/util.h"
#include "util.h"

#include <memory>

void ChangeCommentServlet::doPost(const pico::HttpRequest::Ptr& req,
                                  pico::HttpResponse::Ptr& resp) {
    Json::Value body;
    if (!strToJson(req->get_body(), body)) {
        resp->set_status(pico::HttpStatus::BAD_REQUEST);
        resp->set_body("invalid request");
        return;
    }

    int code = 0;
    std::string message = "success";
    Json::Value data = {};


    std::string id = body.get("id", "").asString();
    int status = std::stoi(body.get("status", "0").asString());

    pico::Mapper<main_comment> mapper("sql_1");

    if (id.empty() || !mapper.existsWithPrimaryKey(id)) {
        code = 200;
        message = "message not found";
    }
    else {
        auto comment = mapper.selectByPrimaryKey(id);
        comment.status = status;
        mapper.updateByPrimaryKey(comment);
    }

    Json::Value resp_json;
    resp_json["code"] = code;
    resp_json["message"] = message;
    resp_json["data"] = data;

    resp->set_body(resp_json.toStyledString());
}

void ChangeThirdCommentServlet::doPost(const pico::HttpRequest::Ptr& req,
                                       pico::HttpResponse::Ptr& resp) {
    Json::Value body;
    if (!strToJson(req->get_body(), body)) {
        resp->set_status(pico::HttpStatus::BAD_REQUEST);
        resp->set_body("invalid request");
        return;
    }

    int code = 0;
    std::string message = "success";
    Json::Value data = {};

    std::string main_comment_id = body.get("id", "").asString();
    std::string reply_comment_id = body.get("_id", "").asString();

    pico::Mapper<Article> mapper("sql_1");
    pico::Mapper<reply_comment> reply_mapper("sql_1");

    if (main_comment_id.empty() || !mapper.existsWithPrimaryKey(reply_comment_id)) {
        code = 200;
        message = "message not found";
    }
    else {
        auto reply = reply_mapper.selectByPrimaryKey(reply_comment_id);

        if (reply.main_comment_id != std::stoi(main_comment_id)) {
            code = 200;
            message = "message not found";
        }
        else {
            int article_id = reply.article_id;
            reply_mapper.deleteByPrimaryKey(reply_comment_id);

            auto article = mapper.selectByPrimaryKey(article_id);
            article.comments -= 1;
            mapper.updateByPrimaryKey(article);
        }
    }

    Json::Value resp_json;
    resp_json["code"] = code;
    resp_json["message"] = message;
    resp_json["data"] = data;

    resp->set_body(jsonToStr(resp_json));
}

void AddCommentServlet::doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& resp) {
    Json::Value body;
    if (!strToJson(req->get_body(), body)) {
        resp->set_status(pico::HttpStatus::BAD_REQUEST);
        resp->set_body("invalid request");
        return;
    }

    int code = 0;
    std::string message = "success";
    Json::Value data = {};

    std::string article_id = body.get("article_id", "").asString();
    Json::Value comment_data = body["data"];

    std::string comment_content = comment_data.get("content", "").asString();
    std::string comment_username = comment_data.get("username", "").asString();
    std::string comment_email = comment_data.get("email", "").asString();

    pico::Mapper<Article> mapper("sql_1");
    pico::Mapper<main_comment> comment_mapper("sql_1");
    pico::Mapper<User> user_mapper("sql_1");

    if (!is_email_valid(comment_email) || article_id.empty() ||
        !mapper.existsWithPrimaryKey(article_id)) {
        code = 200;
        message = "invalid request";
    }
    else {
        auto article = mapper.selectByPrimaryKey(article_id);

        pico::Base<User> user_base;
        auto criteria = user_base.createCriteria();
        criteria->andEqualTo(&User::email, comment_email)
            ->andEqualTo(&User::name, comment_username)
            ->andEqualTo(&User::role, 0);
        auto ret = user_mapper.select(user_base);
        int user_id = 0;
        if (ret.size() == 0) {
            user_id = user_mapper.insert(
                User(comment_username, comment_email, std::time(nullptr), "", 0));
        }
        else {
            user_id = ret[0].id;
        }

        comment_mapper.insert(main_comment(
            article.id, user_id, comment_content, 1, 0, std::time(nullptr), std::time(nullptr)));

        article.comments += 1;
        mapper.updateByPrimaryKey(article);
    }

    Json::Value resp_json;
    resp_json["code"] = code;
    resp_json["message"] = message;
    resp_json["data"] = data;

    resp->set_body(jsonToStr(resp_json));
}

void AddThirdCommentServlet::doPost(const pico::HttpRequest::Ptr& req,
                                    pico::HttpResponse::Ptr& resp) {
    Json::Value body;
    if (!strToJson(req->get_body(), body)) {
        resp->set_status(pico::HttpStatus::BAD_REQUEST);
        resp->set_body("invalid request");
        return;
    }

    int code = 0;
    std::string message = "success";
    Json::Value data = {};

    std::string article_id = body.get("article_id", "").asString();
    std::string comment_id = body.get("comment_id", "").asString();
    std::string content = body.get("content", "").asString();

    pico::Mapper<Article> mapper_article("sql_1");
    pico::Mapper<User> mapper_user("sql_1");
    pico::Mapper<main_comment> mapper_comment("sql_1");
    pico::Mapper<reply_comment> mapper_reply("sql_1");

    Json::Value from_user, to_user;
    if (!strToJson(body["from_user"].asString(), from_user) ||
        !strToJson(body["to_user"].asString(), to_user) || article_id.empty() ||
        comment_id.empty() || !mapper_article.existsWithPrimaryKey(article_id) ||
        !mapper_comment.existsWithPrimaryKey(comment_id)) {
        code = 200;
        message = "invalid request";
    }
    else {
        auto article = mapper_article.selectByPrimaryKey(article_id);

        std::string comment_email = from_user.get("email", "").asString();
        std::string comment_name = from_user.get("username", "").asString();
        if (!is_email_valid(comment_email) || comment_name.empty()) {
            code = 200;
            message = "invalid request";
        }
        else {
            pico::Base<User> user_base;
            auto criteria = user_base.createCriteria();
            criteria->andEqualTo(&User::email, comment_email)
                ->andEqualTo(&User::name, comment_name)
                ->andEqualTo(&User::role, 0);
            auto ret = mapper_user.select(user_base);
            int from_user_id = 0;
            if (ret.size() == 0) {
                from_user_id = mapper_user.insert(
                    User(comment_name, comment_email, std::time(nullptr), "", 0));
            }
            else {
                from_user_id = ret[0].id;
            }

            std::string to_user_id = to_user.get("user_id", "").asString();
            if (to_user_id.empty() || !mapper_user.existsWithPrimaryKey(to_user_id)) {
                code = 200;
                message = "invalid request";
            }
            else {
                mapper_reply.insert(reply_comment(article.id,
                                                  std::stoi(comment_id),
                                                  from_user_id,
                                                  std::stoi(to_user_id),
                                                  content,
                                                  std::time(nullptr)));

                article.comments += 1;

                mapper_article.updateByPrimaryKey(article);
            }
        }
    }

    Json::Value resp_json;
    resp_json["code"] = code;
    resp_json["message"] = message;
    resp_json["data"] = data;

    resp->set_body(jsonToStr(resp_json));
}