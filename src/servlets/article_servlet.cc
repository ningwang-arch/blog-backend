#include "article_servlet.h"

#include <boost/algorithm/string.hpp>

#include "oss/oss_client.h"
#include "pico/config.h"
#include "pico/env.h"
#include "pico/session.h"
#include "util.h"


static pico::ConfigVar<std::string>::Ptr g_oss_bucket =
    pico::Config::Lookup<std::string>("other.oss.bucket", std::string(), "oss bucket name");
static pico::ConfigVar<std::string>::Ptr g_oss_conf =
    pico::Config::Lookup<std::string>("other.oss.conf", std::string(), "oss conf file");

static std::string conf_dir = [] {
    std::string conf_dir = pico::EnvManager::getInstance()->getConfigPath();
    if (conf_dir[conf_dir.size() - 1] != '/') { conf_dir += "/"; }
    return conf_dir;
}();

void GetArticleListAdminServlet::doGet(const pico::HttpRequest::Ptr& req,
                                       pico::HttpResponse::Ptr& res) {
    auto conn = get_connection();

    std::string keyword = req->get_param("keyword");
    int page = req->get_param("pageNum").empty() ? 1 : std::stoi(req->get_param("pageNum"));
    int page_size = req->get_param("pageSize").empty() ? 10 : std::stoi(req->get_param("pageSize"));
    std::string state = req->get_param("state");

    int code = 0;
    std::string message = "success";
    res->set_status(pico::HttpStatus::OK);

    Json::Value data = {};


    std::string sql = "select * from article where keyword like '%" + keyword +
                      "%' order by created_at desc limit " + std::to_string(page_size) +
                      " offset " + std::to_string((page - 1) * page_size);
    auto rs = conn->query(sql);
    if (!rs) {
        code = 200;
        message = "database error";
    }
    else {
        data["count"] = rs->size();
        data["list"] = Json::Value(Json::arrayValue);

        Result::Ptr ret = nullptr;
        while ((ret = rs->next()) != nullptr) {
            Json::Value item = {};

            item["_id"] = ret->getValue("id");
            item["title"] = ret->getValue("title");
            item["meta"]["comments"] = std::stoi(ret->getValue("comments"));
            item["meta"]["views"] = std::stoi(ret->getValue("views"));
            item["author"] = ret->getValue("author");
            item["create_time"] = ret->getValue("created_at");
            item["desc"] = ret->getValue("description");
            item["img_url"] = ret->getValue("img_url");
            item["keyword"] = Json::Value(Json::arrayValue);
            item["tags"] = Json::Value(Json::arrayValue);
            item["category"] = Json::Value(Json::arrayValue);

            std::string keywords = ret->getValue("keyword");
            if (!keyword.empty()) {
                std::vector<std::string> vec;
                boost::split(vec, keywords, boost::is_any_of(","));
                std::for_each(vec.begin(), vec.end(), [&item](std::string str) {
                    item["keyword"].append(str);
                });
            }
            std::string tags = ret->getValue("tags");
            if (!tags.empty()) {
                std::vector<std::string> vec;
                boost::split(vec, tags, boost::is_any_of(","));
                std::for_each(vec.begin(), vec.end(), [&item, &conn](std::string tag) {
                    std::string sql = "select name from tag_category where id = " + tag;
                    std::shared_ptr<ResultSet> rs = conn->query(sql);
                    if (rs->size() == 0) { item["tag"].append({}); }
                    else {
                        Result::Ptr res = rs->next();
                        // combine tag name and id into json, then append to item
                        Json::Value tag_item = {};
                        tag_item["id"] = tag;
                        tag_item["name"] = res->getValue("name");
                        item["tags"].append(tag_item);
                    }
                });
            }
            std::string category = ret->getValue("category");
            if (!category.empty()) {
                std::vector<std::string> category_list;
                boost::split(category_list, category, boost::is_any_of(","));
                std::for_each(category_list.begin(),
                              category_list.end(),
                              [&item, conn](std::string category) {
                                  std::string sql =
                                      "select name from tag_category where id = " + category;
                                  std::shared_ptr<ResultSet> rs = conn->query(sql);
                                  if (rs->size() == 0) { item["category"].append({}); }
                                  else {
                                      Result::Ptr res = rs->next();
                                      Json::Value category_item = {};
                                      category_item["id"] = category;
                                      category_item["name"] = res->getValue("name");
                                      item["category"].append(category_item);
                                  }
                              });
            }
            item["comments"] = handle_comments(ret->getValue("id"));
            data["list"].append(item);
        }
    }

    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = data;

    res->set_body(jsonToStr(json_resp));
}

void AddArticleServlet::doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) {
    auto conn = get_connection();
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


    std::string title = body["title"].asString();
    std::string author = body["author"].asString();
    std::string desc = body["desc"].asString();
    std::string keyword = body["keyword"].asString();
    std::string content = body["content"].asString();
    std::string filename = std::to_string(time(NULL)) + ".md";
    std::string img_url = body["img_url"].asString();
    std::string tags = body["tags"].asString();
    std::string category = body["category"].asString();

    if (!CheckParameter(title) || !CheckParameter(author) || !CheckParameter(desc) ||
        !CheckParameter(keyword) || !CheckParameter(content) || !CheckParameter(img_url) ||
        !CheckParameter(tags) || !CheckParameter(category)) {
        code = 200;
        message = "invalid parameter";
    }
    else {

        OSSClient client(g_oss_bucket->getValue(), conf_dir + g_oss_conf->getValue());
        if (!client.upload_data(filename, content)) {
            code = 200;
            message = "upload file failed";
        }
        else {

            std::string sql = "insert into article (title, author, description, keyword, content, "
                              "img_url, tags, category, created_at, updated_at) values ('" +
                              title + "', '" + author + "', '" + desc + "', '" + keyword + "', '" +
                              filename + "', '" + img_url + "', '" + tags + "', '" + category +
                              "', now(), now())";
            if (!conn->query(sql)) {
                code = 200;
                message = "database error";
            }
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
    auto conn = get_connection();
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

    std::string id = body["id"].asString();
    if (!CheckParameter(id)) {
        code = 200;
        message = "invalid parameter";
    }
    else {
        std::string sql = "select * from article where id = " + id;
        std::shared_ptr<ResultSet> rs = conn->query(sql);
        if (rs->size() == 0) {
            code = 200;
            message = "article not found";
        }
        else {
            Result::Ptr ret = rs->next();
            std::string filename = ret->getValue("content");
            OSSClient client(g_oss_bucket->getValue(), conf_dir + g_oss_conf->getValue());
            std::string content;
            if (!client.download_data(filename, content)) {
                code = 200;
                message = "download file failed";
            }
            else {
                data["content"] = content;

                data["_id"] = ret->getValue("id");
                data["title"] = ret->getValue("title");
                data["author"] = ret->getValue("author");
                data["desc"] = ret->getValue("description");
                data["meta"]["views"] = std::stoi(ret->getValue("views")) + 1;
                data["meta"]["comments"] = std::stoi(ret->getValue("comments"));
                data["keyword"] = Json::Value(Json::arrayValue);
                data["category"] = Json::Value(Json::arrayValue);
                data["tags"] = Json::Value(Json::arrayValue);
                data["img_url"] = ret->getValue("img_url");
                std::string kw = ret->getValue("keyword");
                std::string tags = ret->getValue("tags");
                std::string category = ret->getValue("category");

                if (!kw.empty()) {
                    // split keyword into vector by comma, then append to data["keyword"]
                    std::vector<std::string> keyword_list;
                    boost::split(keyword_list, kw, boost::is_any_of(","));
                    std::for_each(
                        keyword_list.begin(), keyword_list.end(), [&data](std::string keyword) {
                            data["keyword"].append(keyword);
                        });
                }

                if (!tags.empty()) {
                    std::vector<std::string> tag_list;
                    boost::split(tag_list, tags, boost::is_any_of(","));
                    std::for_each(tag_list.begin(), tag_list.end(), [&data, conn](std::string tag) {
                        std::string sql = "select name from tag_category where id = " + tag;
                        std::shared_ptr<ResultSet> rs = conn->query(sql);
                        if (rs->size() == 0) { data["tag"].append({}); }
                        else {
                            Result::Ptr res = rs->next();
                            // combine tag name and id into json, then append to item
                            Json::Value tag_item = {};
                            tag_item["id"] = tag;
                            tag_item["name"] = res->getValue("name");
                            data["tags"].append(tag_item);
                        }
                    });
                }
                if (!category.empty()) {
                    std::vector<std::string> category_list;
                    boost::split(category_list, category, boost::is_any_of(","));
                    std::for_each(category_list.begin(),
                                  category_list.end(),
                                  [&data, conn](std::string category) {
                                      std::string sql =
                                          "select name from tag_category where id = " + category;
                                      std::shared_ptr<ResultSet> rs = conn->query(sql);
                                      if (rs->size() == 0) { data["category"].append({}); }
                                      else {
                                          Result::Ptr res = rs->next();
                                          // combine category name and id into json, then append to
                                          // item
                                          Json::Value category_item = {};
                                          category_item["id"] = category;
                                          category_item["name"] = res->getValue("name");
                                          data["category"].append(category_item);
                                      }
                                  });
                }
                data["created_at"] = ret->getValue("created_at");
                data["updated_at"] = ret->getValue("updated_at");
                // data["comments"] = Json::Value(Json::arrayValue);
                data["comments"] = handle_comments(id);
            }
        }
        sql = "update article set views = views + 1 where id = " + id;
        conn->update(sql);
    }

    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = data;

    res->set_body(jsonToStr(json_resp));
}

void UpdateArticleServlet::doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) {
    auto conn = get_connection();
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


    std::string id = body["id"].asString();
    if (!CheckParameter(id)) {
        code = 200;
        message = "invalid parameter";
    }
    else {
        std::string sql = "select * from article where id = " + id;
        std::shared_ptr<ResultSet> rs = conn->query(sql);
        if (rs->size() == 0) {
            code = 200;
            message = "article not found";
        }
        else {
            Result::Ptr ret = rs->next();
            std::string filename = ret->getValue("content");
            OSSClient client(g_oss_bucket->getValue(), conf_dir + g_oss_conf->getValue());
            if (!client.delete_file(filename)) {
                code = 200;
                message = "delete file failed";
            }
            else {
                std::string title = body["title"].asString();
                std::string author = body["author"].asString();
                std::string desc = body["desc"].asString();
                std::string keyword = body["keyword"].asString();
                std::string content = body["content"].asString();
                std::string img_url = body["img_url"].asString();
                std::string tags = body["tags"].asString();
                std::string category = body["category"].asString();

                if (!CheckParameter(title) || !CheckParameter(author) || !CheckParameter(desc) ||
                    !CheckParameter(keyword) || !CheckParameter(content) ||
                    !CheckParameter(img_url) || !CheckParameter(tags) ||
                    !CheckParameter(category)) {
                    code = 200;
                    message = "invalid parameter";
                }
                else {

                    std::string filename = std::to_string(time(NULL)) + ".md";
                    if (!client.upload_file(filename, content)) {
                        code = 200;
                        message = "upload file failed";
                    }
                    else {
                        std::string sql = "update article set title = '" + title + "', author = '" +
                                          author + "', description = '" + desc + "', keyword = '" +
                                          keyword + "', content = '" + filename + "', img_url = '" +
                                          img_url + "', tags = '" + tags + "', category = '" +
                                          category + "', updated_at = now() where id = " + id;
                        if (!conn->update(sql)) {
                            code = 200;
                            message = "update article failed";
                        }
                    }
                }
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
    auto conn = get_connection();
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

    std::string id = body["id"].asString();
    if (!CheckParameter(id)) {
        code = 200;
        message = "invalid parameter";
    }
    else {
        std::string sql = "select * from article where id = " + id;
        std::shared_ptr<ResultSet> rs = conn->query(sql);
        if (rs->size() == 0) {
            code = 200;
            message = "article not found";
        }
        else {
            Result::Ptr ret = rs->next();
            std::string filename = ret->getValue("content");

            OSSClient client(g_oss_bucket->getValue(), conf_dir + g_oss_conf->getValue());
            if (!client.delete_file(filename)) {
                code = 200;
                message = "delete file failed";
            }
            else {
                std::string sql = "delete from article where id = " + id;
                if (!conn->update(sql)) {
                    code = 200;
                    message = "delete article failed";
                }
            }
        }
    }

    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = data;

    res->set_body(jsonToStr(json_resp));
}

void GetArticleListServlet::doGet(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) {
    auto conn = get_connection();

    int code = 0;
    std::string message = "success";
    res->set_status(pico::HttpStatus::OK);
    Json::Value data = {};

    int page = std::stoi(req->get_param("pageNum"));
    int page_size = std::stoi(req->get_param("pageSize"));
    std::string keyword = req->get_param("keyword");
    std::string tag_id = req->get_param("tag_id");
    std::string title = req->get_param("title");
    std::string category_id = req->get_param("category_id");

    if (!CheckParameter(keyword) || !CheckParameter(title)) {
        code = 200;
        message = "invalid parameter";
    }
    else {
        keyword = url_decode(keyword);
        title = url_decode(title);

        std::string sql = " select * from article where keyword like '%" + keyword + "%'" +
                          " and tags like '%" + tag_id + "%' and category like '%" + category_id +
                          "%' and title like '%" + title + "%' order by id desc limit " +
                          std::to_string((page - 1) * page_size) + "," + std::to_string(page_size);
        std::shared_ptr<ResultSet> rs = conn->query(sql);
        data["count"] = rs->size();
        Result::Ptr ret = nullptr;
        while ((ret = rs->next()) != nullptr) {
            Json::Value article;
            article["_id"] = ret->getValue("id");
            article["title"] = ret->getValue("title");
            article["desc"] = ret->getValue("description");
            article["meta"]["views"] = std::stoi(ret->getValue("views"));
            article["meta"]["comments"] = std::stoi(ret->getValue("comments"));
            article["create_time"] = ret->getValue("created_at");
            article["img_url"] = ret->getValue("img_url");
            data["list"].append(article);
        }
    }

    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = data;

    res->set_body(jsonToStr(json_resp));
}