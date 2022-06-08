#include "message_servlet.h"

#include "../tables.hpp"
#include "pico/session.h"
#include "pico/util.h"
#include "util.h"

void GetMessageListServlet::doGet(const pico::HttpRequest::Ptr& req,
                                  pico::HttpResponse::Ptr& resp) {
    int code = 0;
    std::string message = "success";
    Json::Value data = {};


    std::string keyword = req->get_param("keyword");
    int pageNum = std::stoi(req->get_param("pageNum", "1"));
    int pageSize = std::stoi(req->get_param("pageSize", "10"));

    pico::RowBoundsMapper<main_comment> mapper("sql_1");
    pico::RowBounds row_bounds((pageNum - 1) * pageSize, pageSize);

    pico::Base<main_comment> base;
    auto criteria = base.createCriteria();
    criteria->andLike(&main_comment::content, "%" + keyword + "%")
        ->andNotEqualTo(&main_comment::status, -1);
    base.orderByDesc(&main_comment::created_at);

    auto comments = mapper.selectByRowBounds(base, row_bounds);
    data["count"] = comments.size();
    data["list"] = Json::Value(Json::arrayValue);

    for (auto& comment : comments) {
        Json::Value item;
        item["_id"] = std::to_string(comment.id);
        item["content"] = comment.content;
        item["user_id"] = std::to_string(comment.user_id);
        item["state"] = std::to_string(comment.status);
        item["create_time"] = pico::Time2Str(comment.created_at);
        item["update_time"] = pico::Time2Str(comment.updated_at);
        item["avatar"] = "user";
        item["reply_list"] = Json::Value(Json::arrayValue);

        pico::Mapper<reply_comment> reply_mapper("sql_1");
        pico::Base<reply_comment> reply_base;
        auto reply_criteria = reply_base.createCriteria();
        reply_criteria->andEqualTo(&reply_comment::main_comment_id, comment.id)
            ->andEqualTo(&reply_comment::to_user_id, comment.user_id);
        auto ret = reply_mapper.select(reply_base);
        for (auto& reply : ret) {
            Json::Value reply_item;
            reply_item["_id"] = std::to_string(reply.id);
            reply_item["content"] = reply.content;
            item["reply_list"].append(reply_item);
        }

        pico::Mapper<User> user_mapper("sql_1");
        if (user_mapper.existsWithPrimaryKey(comment.user_id)) {
            auto user = user_mapper.selectByPrimaryKey(comment.user_id);
            item["name"] = user.name;
            item["email"] = user.email;
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
    Json::Value body;
    if (!strToJson(req->get_body(), body)) {
        resp->set_status(pico::HttpStatus::BAD_REQUEST);
        resp->set_body("invalid request");
        return;
    }
    std::string id = body.get("id", "").asString();
    int code = 0;
    std::string message = "success";
    Json::Value data = {};

    pico::Mapper<main_comment> mapper("sql_1");

    if (id.empty() || !mapper.existsWithPrimaryKey(id)) {
        code = 200;
        message = "message not found";
    }
    else {
        auto comment = mapper.selectByPrimaryKey(id);
        data["_id"] = std::to_string(comment.id);
        data["content"] = comment.content;
        data["user_id"] = std::to_string(comment.user_id);
        data["state"] = std::to_string(comment.status);
        data["create_time"] = pico::Time2Str(comment.created_at);
        data["update_time"] = pico::Time2Str(comment.updated_at);
        data["avatar"] = "user";
        data["reply_list"] = Json::Value(Json::arrayValue);

        pico::Mapper<reply_comment> reply_mapper("sql_1");
        pico::Base<reply_comment> reply_base;
        auto reply_criteria = reply_base.createCriteria();
        reply_criteria->andEqualTo(&reply_comment::main_comment_id, comment.id)
            ->andEqualTo(&reply_comment::to_user_id, comment.user_id);
        auto ret = reply_mapper.select(reply_base);
        for (auto& reply : ret) {
            Json::Value reply_item;
            reply_item["_id"] = std::to_string(reply.id);
            reply_item["content"] = reply.content;
            data["reply_list"].append(reply_item);
        }

        pico::Mapper<User> user_mapper("sql_1");
        if (user_mapper.existsWithPrimaryKey(comment.user_id)) {
            auto user = user_mapper.selectByPrimaryKey(comment.user_id);
            data["name"] = user.name;
            data["email"] = user.email;
        }
    }

    Json::Value root;
    root["code"] = code;
    root["message"] = message;
    root["data"] = data;

    resp->set_body(jsonToStr(root));
}

void AddReplyMessageServlet::doPost(const pico::HttpRequest::Ptr& req,
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

    auto session = pico::SessionManager::getInstance()->getRequestSession(req, resp);

    std::string id = body.get("id", "").asString();
    std::string content = body.get("content", "").asString();
    int current_user_id = session->get<int>("user_id");

    pico::Mapper<main_comment> mapper("sql_1");

    if (id.empty() || !mapper.existsWithPrimaryKey(id)) {
        code = 200;
        message = "message not found";
    }
    else {
        auto comment = mapper.selectByPrimaryKey(id);
        int article_id = comment.article_id;
        int user_id = comment.user_id;

        reply_comment reply(
            article_id, comment.id, current_user_id, user_id, content, std::time(nullptr));
        pico::Mapper<reply_comment> reply_mapper("sql_1");
        reply_mapper.insert(reply);

        pico::Mapper<Article> article_mapper("sql_1");
        auto article = article_mapper.selectByPrimaryKey(article_id);
        article.comments += 1;
        article_mapper.updateByPrimaryKey(article);
    }

    Json::Value root;
    root["code"] = code;
    root["message"] = message;

    resp->set_body(jsonToStr(root));
}

void DelMessageServlet::doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& resp) {
    Json::Value body;
    if (!strToJson(req->get_body(), body)) {
        resp->set_status(pico::HttpStatus::BAD_REQUEST);
        resp->set_body("invalid request");
        return;
    }

    int code = 0;
    std::string message = "success";
    Json::Value data = {};

    pico::Mapper<main_comment> mapper("sql_1");
    pico::Mapper<reply_comment> reply_mapper("sql_1");
    pico::Mapper<Article> article_mapper("sql_1");

    std::string id = body.get("id", "").asString();

    if (id.empty() || !mapper.existsWithPrimaryKey(id)) {
        code = 200;
        message = "message not found";
    }
    else {
        auto comment = mapper.selectByPrimaryKey(id);
        int article_id = comment.article_id;

        mapper.deleteByPrimaryKey(id);

        pico::Base<reply_comment> reply_base;
        auto reply_criteria = reply_base.createCriteria();
        reply_criteria->andEqualTo(&reply_comment::main_comment_id, comment.id);
        int ret = reply_mapper.deleteBy(reply_base);


        auto article = article_mapper.selectByPrimaryKey(article_id);
        article.comments -= (1 + ret);
        article_mapper.updateByPrimaryKey(article);
    }

    Json::Value root;
    root["code"] = code;
    root["message"] = message;
    root["data"] = data;

    resp->set_body(jsonToStr(root));
}