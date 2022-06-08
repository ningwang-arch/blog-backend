#include "article_servlet.h"

#include <boost/algorithm/string.hpp>

#include "../tables.hpp"
#include "../util.h"
#include "oss/oss_client.h"
#include "pico/config.h"
#include "pico/env.h"
#include "pico/session.h"
#include "pico/util.h"
#include "util.h"


static pico::ConfigVar<std::string>::Ptr g_oss_bucket =
    pico::Config::Lookup<std::string>("other.oss.bucket", std::string(), "oss bucket name");
static pico::ConfigVar<std::string>::Ptr g_oss_conf =
    pico::Config::Lookup<std::string>("other.oss.conf", std::string(), "oss conf file");

auto get_conf_dir = [] {
    std::string conf_dir = pico::EnvManager::getInstance()->getConfigPath();
    if (conf_dir[conf_dir.size() - 1] != '/') { conf_dir += "/"; }
    return conf_dir;
};

void GetArticleListAdminServlet::doGet(const pico::HttpRequest::Ptr& req,
                                       pico::HttpResponse::Ptr& res) {
    std::string keyword = req->get_param("keyword");
    int page = req->get_param("pageNum").empty() ? 1 : std::stoi(req->get_param("pageNum"));
    int page_size = req->get_param("pageSize").empty() ? 10 : std::stoi(req->get_param("pageSize"));

    int code = 0;
    std::string message = "success";
    res->set_status(pico::HttpStatus::OK);

    Json::Value data = {};

    pico::RowBoundsMapper<Article> mapper("sql_1");

    pico::RowBounds row_bounds(page_size * (page - 1), page_size);

    pico::Base<Article> base;
    auto criteria = base.createCriteria();
    criteria->andLike(&Article::title, "%" + keyword + "%");

    base.orderByDesc(&Article::updated_at);

    auto ret = mapper.selectByRowBounds(base, row_bounds);
    data["count"] = ret.size();
    data["list"] = Json::Value(Json::arrayValue);

    for (auto article : ret) {
        Json::Value item_data = {};
        item_data["_id"] = article.id;
        item_data["title"] = article.title;
        item_data["meta"]["comments"] = std::to_string(article.comments);
        item_data["meta"]["views"] = std::to_string(article.views);
        item_data["author"] = article.author;
        item_data["create_time"] = pico::Time2Str(article.created_at);
        item_data["desc"] = article.description;
        item_data["img_url"] = article.img_url;
        item_data["keyword"] = Json::Value(Json::arrayValue);
        item_data["tags"] = Json::Value(Json::arrayValue);
        item_data["category"] = Json::Value(Json::arrayValue);
        item_data["comments"] = Json::Value(Json::arrayValue);
        if (!article.keyword.empty()) {
            std::vector<std::string> keywords;
            boost::split(keywords, article.keyword, boost::is_any_of(","));
            for (auto keyword : keywords) { item_data["keyword"].append(keyword); }
        }
        if (!article.tags.empty()) {
            std::vector<std::string> tags;
            boost::split(tags, article.tags, boost::is_any_of(","));
            for (auto tag : tags) {
                pico::Mapper<tag_category> mapper_tag;
                mapper_tag.use("sql_1");
                if (mapper_tag.existsWithPrimaryKey(tag)) {
                    auto tag_category = mapper_tag.selectByPrimaryKey(tag);
                    Json::Value tag_data = {};
                    tag_data["id"] = tag;
                    tag_data["name"] = tag_category.name;
                    item_data["tags"].append(tag_data);
                }
            }
        }
        if (!article.category.empty()) {
            std::vector<std::string> categories;
            boost::split(categories, article.category, boost::is_any_of(","));
            for (auto category : categories) {
                pico::Mapper<tag_category> mapper_category;
                mapper_category.use("sql_1");
                if (mapper_category.existsWithPrimaryKey(category)) {
                    auto category_data = mapper_category.selectByPrimaryKey(category);
                    Json::Value category_data_json = {};
                    category_data_json["id"] = category;
                    category_data_json["name"] = category_data.name;
                    item_data["category"].append(category_data_json);
                }
            }
        }
        for (auto& comment : article.main_comments) {
            Json::Value comment_data = {};
            comment_data["_id"] = comment.id;
            comment_data["content"] = comment.content;
            comment_data["create_time"] = pico::Time2Str(comment.created_at);
            comment_data["is_handle"] = std::to_string(comment.is_handle);
            comment_data["other_comments"] = Json::Value(Json::arrayValue);
            int user_id = comment.user_id;
            pico::Mapper<User> mapper_user;
            mapper_user.use("sql_1");
            if (mapper_user.existsWithPrimaryKey(user_id)) {
                auto user = mapper_user.selectByPrimaryKey(user_id);
                Json::Value user_data = {};
                user_data["name"] = user.name;
                user_data["type"] = std::to_string(user.role);
                user_data["avatar"] = "";
                comment_data["user"] = user_data;
            }

            pico::Mapper<reply_comment> mapper_reply;
            mapper_reply.use("sql_1");
            pico::Base<reply_comment> base_reply;
            auto criteria_reply = base_reply.createCriteria();
            criteria_reply->andEqualTo(&reply_comment::main_comment_id, comment.id);
            criteria_reply->andEqualTo(&reply_comment::article_id, article.id);
            auto ret = mapper_reply.select(base_reply);

            for (auto& reply : ret) {
                Json::Value reply_data = {};
                reply_data["id"] = std::to_string(reply.id);
                reply_data["content"] = reply.content;
                reply_data["create_time"] = pico::Time2Str(reply.created_at);

                pico::Mapper<User> mapper_user;
                mapper_user.use("sql_1");
                if (mapper_user.existsWithPrimaryKey(reply.from_user_id)) {
                    auto from_user = mapper_user.selectByPrimaryKey(reply.from_user_id);
                    Json::Value from = {};
                    from["user_id"] = std::to_string(from_user.id);
                    from["name"] = from_user.name;
                    from["type"] = std::to_string(from_user.role);
                    from["avatar"] = "";
                    reply_data["user"] = from;
                }
                if (mapper_user.existsWithPrimaryKey(reply.to_user_id)) {
                    auto to_user = mapper_user.selectByPrimaryKey(reply.to_user_id);
                    Json::Value to = {};
                    to["user_id"] = std::to_string(to_user.id);
                    to["name"] = to_user.name;
                    to["type"] = std::to_string(to_user.role);
                    to["avatar"] = "";
                    reply_data["to_user"] = to;
                }

                comment_data["other_comments"].append(reply_data);
            }
            item_data["comments"].append(comment_data);
        }

        data["list"].append(item_data);
    }

    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = data;

    res->set_body(jsonToStr(json_resp));
}

void AddArticleServlet::doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) {
    Json::Value body;
    if (!strToJson(req->get_body(), body)) {
        res->set_status(pico::HttpStatus::BAD_REQUEST);
        res->set_body("invalid request");
        return;
    }

    int code = 0;
    std::string message = "success";
    res->set_status(pico::HttpStatus::OK);
    Json::Value data = {};


    std::string title = body.get("title", "Title").asString();
    std::string author = body.get("author", "").asString();
    std::string desc = body.get("desc", "").asString();
    std::string keyword = body.get("keyword", "").asString();
    std::string content = body.get("content", "").asString();
    std::string filename = std::to_string(time(NULL)) + ".md";
    std::string img_url = body.get("img_url", "").asString();
    std::string tags = body.get("tags", "").asString();
    std::string category = body.get("category", "").asString();

    Article article(title,
                    keyword,
                    author,
                    desc,
                    filename,
                    img_url,
                    tags,
                    category,
                    0,
                    0,
                    std::time(nullptr),
                    std::time(nullptr));

    OSSClient client(g_oss_bucket->getValue(), get_conf_dir() + g_oss_conf->getValue());
    if (!client.upload_data(filename, content)) {
        code = 200;
        message = "upload failed";
    }
    else {
        pico::Mapper<Article> mapper_article("sql_1");
        if (!mapper_article.insert(article)) {
            code = 200;
            message = "insert failed";
        }
    }

    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = data;

    res->set_body(jsonToStr(json_resp));
}


void GetArticleDetailServlet::doPost(const pico::HttpRequest::Ptr& req,
                                     pico::HttpResponse::Ptr& res) {
    Json::Value body;
    if (!strToJson(req->get_body(), body)) {
        res->set_status(pico::HttpStatus::BAD_REQUEST);
        res->set_body("invalid request");
        return;
    }

    int code = 0;
    std::string message = "success";
    res->set_status(pico::HttpStatus::OK);
    Json::Value data = {};

    pico::Mapper<Article> mapper_article("sql_1");

    std::string id = body.get("id", "").asString();
    if (id.empty() || !mapper_article.existsWithPrimaryKey(id)) {
        code = 200;
        message = "invalid id";
    }
    else {
        auto article = mapper_article.selectByPrimaryKey(id);

        article.views += 1;

        std::string filename = article.content;
        std::string content;
        OSSClient client(g_oss_bucket->getValue(), get_conf_dir() + g_oss_conf->getValue());
        if (!client.download_data(filename, content)) {
            code = 200;
            message = "download failed";
        }
        else {
            data["content"] = content;

            data["_id"] = std::to_string(article.id);
            data["title"] = article.title;
            data["author"] = article.author;
            data["description"] = article.description;
            data["meta"]["views"] = article.views;
            data["meta"]["comments"] = article.comments;
            data["img_url"] = article.img_url;
            data["created_at"] = pico::Time2Str(article.created_at);
            data["updated_at"] = pico::Time2Str(article.updated_at);
            data["tags"] = Json::Value(Json::arrayValue);
            data["category"] = Json::Value(Json::arrayValue);
            data["keyword"] = Json::Value(Json::arrayValue);
            data["comments"] = Json::Value(Json::arrayValue);

            if (!article.keyword.empty()) {
                std::vector<std::string> keywords;
                boost::split(keywords, article.keyword, boost::is_any_of(","));
                for (auto keyword : keywords) { data["keyword"].append(keyword); }
            }
            if (!article.tags.empty()) {
                std::vector<std::string> tags;
                boost::split(tags, article.tags, boost::is_any_of(","));
                for (auto tag : tags) {
                    pico::Mapper<tag_category> mapper_tag;
                    mapper_tag.use("sql_1");
                    if (mapper_tag.existsWithPrimaryKey(tag)) {
                        auto tag_category = mapper_tag.selectByPrimaryKey(tag);
                        Json::Value tag_data = {};
                        tag_data["id"] = tag;
                        tag_data["name"] = tag_category.name;
                        data["tags"].append(tag_data);
                    }
                }
            }
            if (!article.category.empty()) {
                std::vector<std::string> categories;
                boost::split(categories, article.category, boost::is_any_of(","));
                for (auto category : categories) {
                    pico::Mapper<tag_category> mapper_category;
                    mapper_category.use("sql_1");
                    if (mapper_category.existsWithPrimaryKey(category)) {
                        auto category_data = mapper_category.selectByPrimaryKey(category);
                        Json::Value category_data_json = {};
                        category_data_json["id"] = category;
                        category_data_json["name"] = category_data.name;
                        data["category"].append(category_data_json);
                    }
                }
            }

            for (auto& comment : article.main_comments) {
                Json::Value comment_data = {};
                comment_data["_id"] = std::to_string(comment.id);
                comment_data["content"] = comment.content;
                comment_data["create_time"] = pico::Time2Str(comment.created_at);
                comment_data["is_handle"] = std::to_string(comment.is_handle);
                comment_data["other_comments"] = Json::Value(Json::arrayValue);
                int user_id = comment.user_id;
                pico::Mapper<User> mapper_user;
                mapper_user.use("sql_1");
                if (mapper_user.existsWithPrimaryKey(user_id)) {
                    auto user = mapper_user.selectByPrimaryKey(user_id);
                    Json::Value user_data = {};
                    user_data["id"] = std::to_string(user.id);
                    user_data["name"] = user.name;
                    user_data["type"] = std::to_string(user.role);
                    user_data["avatar"] = "";
                    comment_data["user"] = user_data;
                }

                pico::Mapper<reply_comment> mapper_reply;
                mapper_reply.use("sql_1");
                pico::Base<reply_comment> base_reply;
                auto criteria_reply = base_reply.createCriteria();
                criteria_reply->andEqualTo(&reply_comment::main_comment_id, comment.id);
                criteria_reply->andEqualTo(&reply_comment::article_id, article.id);
                auto ret = mapper_reply.select(base_reply);

                for (auto& reply : ret) {
                    Json::Value reply_data = {};
                    reply_data["id"] = std::to_string(reply.id);
                    reply_data["content"] = reply.content;
                    reply_data["create_time"] = pico::Time2Str(reply.created_at);

                    pico::Mapper<User> mapper_user;
                    mapper_user.use("sql_1");
                    if (mapper_user.existsWithPrimaryKey(reply.from_user_id)) {
                        auto from_user = mapper_user.selectByPrimaryKey(reply.from_user_id);
                        Json::Value from = {};
                        from["user_id"] = std::to_string(from_user.id);
                        from["name"] = from_user.name;
                        from["type"] = std::to_string(from_user.role);
                        from["avatar"] = "";
                        reply_data["user"] = from;
                    }
                    if (mapper_user.existsWithPrimaryKey(reply.to_user_id)) {
                        auto to_user = mapper_user.selectByPrimaryKey(reply.to_user_id);
                        Json::Value to = {};
                        to["user_id"] = std::to_string(to_user.id);
                        to["name"] = to_user.name;
                        to["type"] = std::to_string(to_user.role);
                        to["avatar"] = "";
                        reply_data["to_user"] = to;
                    }

                    comment_data["other_comments"].append(reply_data);
                }
                data["comments"].append(comment_data);
            }

            mapper_article.updateByPrimaryKey(article);
        }
    }

    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = data;

    res->set_body(jsonToStr(json_resp));
}

void UpdateArticleServlet::doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) {
    Json::Value body;
    if (!strToJson(req->get_body(), body)) {
        res->set_status(pico::HttpStatus::BAD_REQUEST);
        res->set_body("invalid request");
        return;
    }

    int code = 0;
    std::string message = "success";
    res->set_status(pico::HttpStatus::OK);
    Json::Value data = {};

    pico::Mapper<Article> mapper_article("sql_1");
    std::string id = body.get("id", "").asString();

    if (id.empty() || !mapper_article.existsWithPrimaryKey(id)) {
        code = 200;
        message = "article not found";
    }
    else {
        auto article = mapper_article.selectByPrimaryKey(id);

        std::string filename = article.content;
        OSSClient client(g_oss_bucket->getValue(), get_conf_dir() + g_oss_conf->getValue());
        if (!client.delete_file(filename)) {
            code = 200;
            message = "delete file failed";
        }
        else {
            std::string content = body.get("content", "").asString();

            article.title = body.get("title", "Title").asString();
            article.author = body.get("author", "Author").asString();
            article.description = body.get("description", "").asString();
            article.keyword = body.get("keyword", "").asString();
            article.content = std::to_string(std::time(nullptr)) + ".md";
            article.category = body.get("category", "").asString();
            article.tags = body.get("tags", "").asString();
            article.img_url = body.get("img_url", "").asString();
            article.updated_at = std::time(nullptr);

            if (!client.upload_data(article.content, content)) {
                code = 200;
                message = "upload file failed";
            }
            else {
                mapper_article.updateByPrimaryKey(article);
            }
        }
    }

    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = data;

    res->set_body(jsonToStr(json_resp));
}

void DelArticleServlet::doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) {
    Json::Value body;
    if (!strToJson(req->get_body(), body)) {
        res->set_status(pico::HttpStatus::BAD_REQUEST);
        res->set_body("invalid request");
        return;
    }

    int code = 0;
    std::string message = "success";
    res->set_status(pico::HttpStatus::OK);
    Json::Value data = {};

    pico::Mapper<Article> mapper_article("sql_1");

    std::string id = body.get("id", "").asString();
    if (id.empty() || !mapper_article.existsWithPrimaryKey(id)) {
        code = 200;
        message = "article not found";
    }
    else {
        auto article = mapper_article.selectByPrimaryKey(id);

        std::string filename = article.content;
        OSSClient client(g_oss_bucket->getValue(), get_conf_dir() + g_oss_conf->getValue());
        if (!client.delete_file(filename)) {
            code = 200;
            message = "delete file failed";
        }
        else {
            mapper_article.deleteByPrimaryKey(id);
        }
    }

    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = data;

    res->set_body(jsonToStr(json_resp));
}

void GetArticleListServlet::doGet(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) {
    int code = 0;
    std::string message = "success";
    res->set_status(pico::HttpStatus::OK);
    Json::Value data = {};

    int page = std::stoi(req->get_param("pageNum", "1"));
    int page_size = std::stoi(req->get_param("pageSize", "10"));
    std::string keyword = req->get_param("keyword");
    std::string tag_id = req->get_param("tag_id");
    std::string title = req->get_param("title");
    std::string category_id = req->get_param("category_id");

    keyword = url_decode(keyword);
    title = url_decode(title);

    pico::RowBoundsMapper<Article> mapper_article("sql_1");

    pico::RowBounds row_bounds(page_size * (page - 1), page_size);
    pico::Base<Article> base;
    auto criteria = base.createCriteria();
    criteria->andLike(&Article::title, "%" + title + "%")
        ->andLike(&Article::keyword, "%" + keyword + "%")
        ->andLike(&Article::tags, "%" + tag_id + "%")
        ->andLike(&Article::category, "%" + category_id + "%");

    base.orderByDesc(&Article::updated_at);

    auto articles = mapper_article.selectByRowBounds(base, row_bounds);

    data["list"] = Json::Value(Json::arrayValue);
    data["count"] = articles.size();
    for (auto& article : articles) {
        Json::Value json_article;
        json_article["_id"] = std::to_string(article.id);
        json_article["title"] = article.title;
        json_article["desc"] = article.description;
        json_article["meta"]["views"] = article.views;
        json_article["meta"]["comments"] = article.comments;
        json_article["create_time"] = pico::Time2Str(article.created_at);
        json_article["img_url"] = article.img_url;
        data["list"].append(json_article);
    }

    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = data;

    res->set_body(jsonToStr(json_resp));
}