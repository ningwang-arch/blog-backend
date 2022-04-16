#include "handlers.h"

#include "application/application.hpp"
#include "oss/oss_client.h"

#include "pico/http/middlewares/cors.h"
#include "pico/http/middlewares/session.h"
#include "pico/http/middlewares/utf-8.h"

#define CTX(app, mw, req) app->get_server()->get_context<mw>(req)

void root(const request& req, response& resp, Application* app) {
    resp->set_status(pico::HttpStatus::OK);
    resp->set_body("Hello World!");
}

void login_handler(const request& req, response& resp, Application* app) {
    auto conn = app->getConnection();
    std::string body = req->get_body();
    Json::Value json;
    if (!strToJson(body, json)) {
        resp->set_status(pico::HttpStatus::BAD_REQUEST);
        resp->set_body("invalid request body");
        return;
    }
    std::string username = json.get("email", "").asString();
    std::string password = json.get("password", "").asString();

    if (!CheckParameter(username) || !CheckParameter(password)) {
        resp->set_status(pico::HttpStatus::BAD_REQUEST);
        resp->set_body("invalid parameter");
        return;
    }
    Json::Value json_resp;

    auto session = CTX(app, pico::Session, req).session;
    if (!session) {
        resp->set_status(pico::HttpStatus::INTERNAL_SERVER_ERROR);
        resp->set_body("session not found");
        return;
    }

    int code = 0;
    std::string message = "success";
    resp->set_status(pico::HttpStatus::OK);
    std::string sql =
        "select * from user where name='" + username + "' and email='" + password + "' and role=1";
    ResultSet* result = conn->query(sql);
    if (result == nullptr) {
        code = 200;
        message = "database error";
    }
    else if (result->size() == 0) {
        code = 200;
        message = "user not exist";
    }
    else {
        std::string user_id = result->next()->getString("id");
        session->set("username", username);
        session->set("user_id", user_id);
    }

    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = "";
    resp->set_body(jsonToStr(json_resp));
}

void getCategoryList(const request& req, response& resp, Application* app) {
    auto conn = app->getConnection();
    std::string keyword = req->get_param("keyword");
    int page = std::stoi(req->get_param("pageNum"));
    int pageSize = std::stoi(req->get_param("pageSize"));


    std::string sql = "select * from tag_category where name like '%" + keyword + "%' and t_c =1";

    int code = 0;
    std::string message = "success";


    resp->set_status(pico::HttpStatus::OK);

    Json::Value data = {};


    ResultSet* rs = conn->query(sql);
    if (!rs) {
        code = 200;
        message = "database error";
    }
    else {
        rs->offset((page - 1) * pageSize, pageSize);
        data["count"] = rs->size();
        Result* res = nullptr;
        Json::Value ca_list;
        while ((res = rs->next()) != nullptr) {
            Json::Value item = {};
            item["_id"] = res->getValue("id");
            item["name"] = res->getValue("name");
            item["desc"] = res->getValue("desc");
            item["create_time"] = res->getValue("created_at");
            ca_list.append(item);
        }

        data["list"] = ca_list;
    }

    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = data;


    resp->set_body(jsonToStr(json_resp));
}

void delCategory(const request& req, response& resp, Application* app) {
    auto conn = app->getConnection();
    std::string body = req->get_body();
    Json::Value json;
    if (!strToJson(body, json)) {
        resp->set_status(pico::HttpStatus::BAD_REQUEST);
        resp->set_body("invalid request body");
        return;
    }
    std::string id = json.get("id", "").asString();

    int code = 0;
    std::string message = "success";

    resp->set_status(pico::HttpStatus::OK);

    auto session = CTX(app, pico::Session, req).session;
    if (!session->has("username")) {
        code = 200;
        message = "not login";
        goto label_1;
    }

    if (!CheckParameter(id)) {
        code = 200;
        message = "invalid parameter";
    }
    else {
        std::string sql = "delete from tag_category where id = " + id;
        if (!conn->update(sql)) {
            code = 200;
            message = "database error";
        }
    }
label_1:
    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = "";

    resp->set_body(jsonToStr(json_resp));
}

void addCategory(const request& req, response& resp, Application* app) {
    auto conn = app->getConnection();
    std::string body = req->get_body();
    Json::Value json;
    if (!strToJson(body, json)) {
        resp->set_status(pico::HttpStatus::BAD_REQUEST);
        resp->set_body("invalid request body");
        return;
    }
    std::string name = json.get("name", "").asString();
    std::string desc = json.get("desc", "").asString();

    int code = 0;
    std::string message = "success";

    resp->set_status(pico::HttpStatus::OK);

    auto session = CTX(app, pico::Session, req).session;
    if (!session->has("username")) {
        code = 200;
        message = "not login";
        goto label_2;
    }

    if (!CheckParameter(name)) {
        code = 200;
        message = "invalid parameter";
    }
    else {
        // search if exist
        std::string sql = "select * from tag_category where name = '" + name +
                          "' and t_c = 1 and `desc` = '" + desc + "'";
        ResultSet* rs = conn->query(sql);

        if (rs->size() > 0) {
            code = 200;
            message = "category exist";
        }
        else {

            std::string crt_time = get_crt_time();
            std::string sql =
                "insert into tag_category(name, `desc`, created_at, updated_at, t_c) values('" +
                name + "', '" + desc + "', '" + crt_time + "', '" + crt_time + "', 1)";
            if (!conn->update(sql)) {
                code = 200;
                message = "database error";
            }
        }
    }

label_2:
    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = "";

    resp->set_body(jsonToStr(json_resp));
}

void getTagList(const request& req, response& resp, Application* app) {
    auto conn = app->getConnection();
    std::string keyword = req->get_param("keyword");
    int page = std::stoi(req->get_param("pageNum"));
    int pageSize = std::stoi(req->get_param("pageSize"));

    std::string sql = "select * from tag_category where name like '%" + keyword + "%' and t_c =0";

    int code = 0;
    std::string message = "success";

    resp->set_status(pico::HttpStatus::OK);

    Json::Value data = {};

    ResultSet* rs = conn->query(sql);
    if (!rs) {
        code = 200;
        message = "database error";
    }
    else {
        rs->offset((page - 1) * pageSize, pageSize);
        data["count"] = rs->size();
        Result* res = nullptr;
        Json::Value ca_list;
        while ((res = rs->next()) != nullptr) {
            Json::Value item = {};
            item["_id"] = res->getValue("id");
            item["name"] = res->getValue("name");
            item["desc"] = res->getValue("desc");
            item["create_time"] = res->getValue("created_at");
            ca_list.append(item);
        }

        data["list"] = ca_list;
    }

    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = data;

    resp->set_body(jsonToStr(json_resp));
}

void delTag(const request& req, response& resp, Application* app) {
    auto conn = app->getConnection();
    std::string body = req->get_body();
    Json::Value json;
    if (!strToJson(body, json)) {
        resp->set_status(pico::HttpStatus::BAD_REQUEST);
        resp->set_body("invalid request body");
        return;
    }
    std::string id = json.get("id", "").asString();

    int code = 0;
    std::string message = "success";

    resp->set_status(pico::HttpStatus::OK);

    auto session = CTX(app, pico::Session, req).session;
    if (!session->has("username")) {
        code = 200;
        message = "not login";
        goto label_3;
    }

    if (!CheckParameter(id)) {
        code = 200;
        message = "invalid parameter";
    }
    else {
        std::string sql = "delete from tag_category where id = " + id;
        if (!conn->update(sql)) {
            code = 200;
            message = "database error";
        }
    }
label_3:
    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = "";

    resp->set_body(jsonToStr(json_resp));
}

void addTag(const request& req, response& resp, Application* app) {
    auto conn = app->getConnection();
    std::string body = req->get_body();
    Json::Value json;
    if (!strToJson(body, json)) {
        resp->set_status(pico::HttpStatus::BAD_REQUEST);
        resp->set_body("invalid request body");
        return;
    }
    std::string name = json.get("name", "").asString();
    std::string desc = json.get("desc", "").asString();

    int code = 0;
    std::string message = "success";

    resp->set_status(pico::HttpStatus::OK);

    auto session = CTX(app, pico::Session, req).session;
    if (!session->has("username")) {
        code = 200;
        message = "not login";
        goto label_4;
    }

    if (!CheckParameter(name)) {
        code = 200;
        message = "invalid parameter";
    }
    else {
        // search if exist
        std::string sql = "select * from tag_category where name = '" + name +
                          "' and t_c = 0 and `desc` = '" + desc + "'";
        ResultSet* rs = conn->query(sql);

        if (rs->size() > 0) {
            code = 200;
            message = "tag exist";
        }
        else {

            std::string crt_time = get_crt_time();
            std::string sql =
                "insert into tag_category(name, `desc`, created_at, updated_at, t_c) values('" +
                name + "', '" + desc + "', '" + crt_time + "', '" + crt_time + "', 0)";
            if (!conn->update(sql)) {
                code = 200;
                message = "database error";
            }
        }
    }

label_4:
    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = "";

    resp->set_body(jsonToStr(json_resp));
}

void getProjectList(const request& req, response& resp, Application* app) {
    auto conn = app->getConnection();
    std::string keyword = req->get_param("keyword");
    int page = std::stoi(req->get_param("pageNum"));
    std::string state = req->get_param("state");
    int pageSize = std::stoi(req->get_param("pageSize"));

    std::string sql;
    if (state.empty()) { sql = "select * from project where title like '%" + keyword + "%'"; }
    else {
        sql = "select * from project where title like '%" + keyword + "%' and status = " + state;
    }

    int code = 0;
    std::string message = "success";

    resp->set_status(pico::HttpStatus::OK);

    Json::Value data = {};

    ResultSet* rs = conn->query(sql);
    if (!rs) {
        code = 200;
        message = "database error";
    }
    else {
        rs->offset((page - 1) * pageSize, pageSize);
        data["count"] = rs->size();
        Result* res = nullptr;
        Json::Value ca_list;
        while ((res = rs->next()) != nullptr) {
            Json::Value item = {};
            item["_id"] = res->getValue("id");
            item["title"] = res->getValue("title");
            item["start_time"] = res->getValue("created_at");
            item["end_time"] = res->getValue("end_at");
            item["content"] = res->getValue("description");
            item["img"] = res->getValue("img");
            item["url"] = res->getValue("url");
            item["state"] = res->getValue("status");
            ca_list.append(item);
        }

        data["list"] = ca_list;
    }

    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = data;

    resp->set_body(jsonToStr(json_resp));
}


void delProject(const request& req, response& resp, Application* app) {
    auto conn = app->getConnection();
    std::string body = req->get_body();
    Json::Value json;
    if (!strToJson(body, json)) {
        resp->set_status(pico::HttpStatus::BAD_REQUEST);
        resp->set_body("invalid request body");
        return;
    }
    std::string id = json.get("id", "").asString();

    int code = 0;
    std::string message = "success";

    resp->set_status(pico::HttpStatus::OK);

    auto session = CTX(app, pico::Session, req).session;
    if (!session->has("username")) {
        code = 200;
        message = "not login";
        goto label_5;
    }

    if (!CheckParameter(id)) {
        code = 200;
        message = "invalid parameter";
    }
    else {
        std::string sql = "delete from project where id = " + id;
        if (!conn->update(sql)) {
            code = 200;
            message = "database error";
        }
    }
label_5:
    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = "";

    resp->set_body(jsonToStr(json_resp));
}

void addProject(const request& req, response& resp, Application* app) {
    auto conn = app->getConnection();
    std::string body = req->get_body();
    Json::Value json;
    if (!strToJson(body, json)) {
        resp->set_status(pico::HttpStatus::BAD_REQUEST);
        resp->set_body("invalid request body");
        return;
    }
    std::string state = json.get("state", "").asString();
    std::string title = json.get("title", "").asString();
    std::string description = json.get("content", "").asString();
    std::string img = json.get("img", "").asString();
    std::string url = json.get("url", "").asString();
    std::string start_time = format_time(json.get("start_time", "").asString());
    std::string end_time = format_time(json.get("end_time", "").asString());

    int code = 0;
    std::string message = "success";

    resp->set_status(pico::HttpStatus::OK);

    auto session = CTX(app, pico::Session, req).session;
    if (!session->has("username")) {
        code = 200;
        message = "not login";
        goto label_6;
    }

    if (!CheckParameter(title)) {
        code = 200;
        message = "invalid parameter";
    }
    else {
        // search if exist
        std::string sql = "select * from project where title = '" + title + "'";
        ResultSet* rs = conn->query(sql);

        if (rs->size() > 0) {
            code = 200;
            message = "project exist";
        }
        else {
            std::string sql = "insert into project(title, description, img, url, created_at, "
                              "end_at, status) values('" +
                              title + "', '" + description + "', '" + img + "', '" + url + "', '" +
                              start_time + "', '" + end_time + "', " + state + ")";
            if (!conn->update(sql)) {
                code = 200;
                message = "database error";
            }
        }
    }
label_6:

    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = "";

    resp->set_body(jsonToStr(json_resp));
}

void getProjectDetail(const request& req, response& resp, Application* app) {
    auto conn = app->getConnection();
    std::string body = req->get_body();
    Json::Value json;
    if (!strToJson(body, json)) {
        resp->set_status(pico::HttpStatus::BAD_REQUEST);
        resp->set_body("invalid request body");
        return;
    }
    std::string id = json.get("id", "").asString();

    int code = 0;
    std::string message = "success";

    resp->set_status(pico::HttpStatus::OK);

    Json::Value data = {};

    auto session = CTX(app, pico::Session, req).session;
    if (!session->has("username")) {
        code = 200;
        message = "not login";
        goto label_7;
    }

    if (!CheckParameter(id)) {
        code = 200;
        message = "invalid parameter";
    }
    else {
        std::string sql = "select * from project where id = " + id;
        ResultSet* rs = conn->query(sql);
        if (rs->size() == 0) {
            code = 200;
            message = "project not exist";
        }
        else {
            Result* res = rs->next();
            data["_id"] = res->getValue("id");
            data["title"] = res->getValue("title");
            data["start_time"] = res->getValue("created_at");
            data["end_time"] = res->getValue("end_at");
            data["content"] = res->getValue("description");
            data["img"] = res->getValue("img");
            data["url"] = res->getValue("url");
            data["state"] = res->getValue("status");
        }
    }
label_7:

    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = data;

    resp->set_body(jsonToStr(json_resp));
}

void updateProject(const request& req, response& resp, Application* app) {
    auto conn = app->getConnection();
    std::string body = req->get_body();
    Json::Value json;
    if (!strToJson(body, json)) {
        resp->set_status(pico::HttpStatus::BAD_REQUEST);
        resp->set_body("invalid request body");
        return;
    }
    std::string id = json.get("id", "").asString();
    std::string state = json.get("state", "").asString();
    std::string title = json.get("title", "").asString();
    std::string description = json.get("content", "").asString();
    std::string img = json.get("img", "").asString();
    std::string url = json.get("url", "").asString();
    std::string start_time = format_time(json.get("start_time", "").asString());
    std::string end_time = format_time(json.get("end_time", "").asString());

    int code = 0;
    std::string message = "success";

    resp->set_status(pico::HttpStatus::OK);

    auto session = CTX(app, pico::Session, req).session;
    if (!session->has("username")) {
        code = 200;
        message = "not login";
        goto label_8;
    }

    if (!CheckParameter(id)) {
        code = 200;
        message = "invalid parameter";
    }
    else {
        std::string sql = "select * from project where id = " + id;
        ResultSet* rs = conn->query(sql);
        if (rs->size() == 0) {
            code = 200;
            message = "project not exist";
        }
        else {
            std::string sql = "update project set title = '" + title + "', description = '" +
                              description + "', img = '" + img + "', url = '" + url +
                              "', "
                              "created_at = '" +
                              start_time + "', end_at = '" + end_time + "', status = " + state +
                              " where id = " + id;
            if (!conn->update(sql)) {
                code = 200;
                message = "database error";
            }
        }
    }
label_8:

    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = "";

    resp->set_body(jsonToStr(json_resp));
}

void getArticleListAdmin(const request& req, response& resp, Application* app) {
    auto conn = app->getConnection();
    std::string keyword = req->get_param("keyword");
    std::string state = req->get_param("state");
    int page = std::stoi(req->get_param("pageNum"));
    int pageSize = std::stoi(req->get_param("pageSize"));

    int code = 0;
    std::string message = "success";

    resp->set_status(pico::HttpStatus::OK);

    Json::Value data = {};
    std::string sql;

    auto session = CTX(app, pico::Session, req).session;
    if (!session->has("username")) {
        code = 200;
        message = "not login";
        goto label_9;
    }
    {


        sql = "select * from article where keyword like '%" + keyword +
              "%' order by created_at desc limit " + std::to_string(pageSize) + " offset " +
              std::to_string((page - 1) * pageSize);

        ResultSet* rs = conn->query(sql);

        if (!rs) {
            code = 200;
            message = "database error";
        }
        else {
            data["count"] = rs->size();
            data["list"] = Json::Value(Json::arrayValue);

            Result* res = nullptr;
            while ((res = rs->next())) {
                Json::Value item = {};
                item["_id"] = res->getValue("id");
                item["title"] = res->getValue("title");
                item["meta"]["comments"] = std::stoi(res->getValue("comments"));
                item["meta"]["views"] = std::stoi(res->getValue("views"));
                item["author"] = res->getValue("author");
                item["create_time"] = res->getValue("created_at");
                item["desc"] = res->getValue("description");
                item["img_url"] = res->getValue("img_url");
                item["keyword"] = Json::Value(Json::arrayValue);
                item["tags"] = Json::Value(Json::arrayValue);
                item["category"] = Json::Value(Json::arrayValue);
                // split keyword by ','
                std::string keywords = res->getValue("keyword");
                if (!keywords.empty()) {
                    std::vector<std::string> vec;
                    boost::split(vec, keywords, boost::is_any_of(","));
                    std::for_each(vec.begin(), vec.end(), [&item](std::string str) {
                        item["keyword"].append(str);
                    });
                }
                // split tag by ',' and query tag name, then append to item
                std::string tags = res->getValue("tags");
                if (!tags.empty()) {
                    std::vector<std::string> tag_list;
                    boost::split(tag_list, tags, boost::is_any_of(","));
                    std::for_each(tag_list.begin(), tag_list.end(), [&item, conn](std::string tag) {
                        std::string sql = "select name from tag_category where id = " + tag;
                        ResultSet* rs = conn->query(sql);
                        if (rs->size() == 0) { item["tag"].append({}); }
                        else {
                            Result* res = rs->next();
                            // combine tag name and id into json, then append to item
                            Json::Value tag_item = {};
                            tag_item["id"] = tag;
                            tag_item["name"] = res->getValue("name");
                            item["tags"].append(tag_item);
                        }
                    });
                }
                // handle category same as tag
                std::string category = res->getValue("category");
                if (!category.empty()) {
                    std::vector<std::string> category_list;
                    boost::split(category_list, category, boost::is_any_of(","));
                    std::for_each(category_list.begin(),
                                  category_list.end(),
                                  [&item, conn](std::string category) {
                                      std::string sql =
                                          "select name from tag_category where id = " + category;
                                      ResultSet* rs = conn->query(sql);
                                      if (rs->size() == 0) { item["category"].append({}); }
                                      else {
                                          Result* res = rs->next();
                                          // combine category name and id into json, then append to
                                          // item
                                          Json::Value category_item = {};
                                          category_item["id"] = category;
                                          category_item["name"] = res->getValue("name");
                                          item["category"].append(category_item);
                                      }
                                  });
                }
                item["comments"] = handle_comments(res->getValue("id"));
                data["list"].append(item);
            }
        }
    }

label_9:

    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = data;

    resp->set_body(jsonToStr(json_resp));
}

/*
POST /addArticle
request: {
    'title': '',
    'author': '',
    'desc': '',
    'keyword': '',
    'content': '',
    'img_url': '',
    'tags': '',
    'category': ''
}
response: {"data": {}, "code": 0, "message": "success"}
*/

void addArticle(const request& req, response& resp, Application* app) {
    auto conn = app->getConnection();
    int code = 0;
    std::string message = "success";
    Json::Value data = {};

    auto session = CTX(app, pico::Session, req).session;
    if (!session->has("username")) {
        code = 200;
        message = "not login";
        goto label_10;
    }
    {
        Json::Value body;
        if (!strToJson(req->get_body(), body)) {
            code = 200;
            message = "invalid request";
            goto label_10;
        }
        else {
            std::string title = body["title"].asString();
            std::string author = body["author"].asString();
            std::string desc = body["desc"].asString();
            std::string keyword = body["keyword"].asString();
            std::string content = body["content"].asString();
            std::string filename = std::to_string(time(NULL)) + ".md";

            OSSClient client("pico-blog");
            if (!client.upload_data(filename, content)) {
                code = 200;
                message = "upload failed";
                goto label_10;
            }

            std::string img_url = body["img_url"].asString();
            std::string tags = body["tags"].asString();
            std::string category = body["category"].asString();

            std::string sql = "insert into article (title, author, description, keyword, content, "
                              "img_url, tags, category, created_at, updated_at) values ('" +
                              title + "', '" + author + "', '" + desc + "', '" + keyword + "', '" +
                              filename + "', '" + img_url + "', '" + tags + "', '" + category +
                              "', now(), now())";
            if (conn->query(sql)) {
                code = 0;
                message = "success";
            }
            else {
                code = 200;
                message = "database error";
            }
        }
    }

label_10:

    Json::Value resp_json;
    resp_json["code"] = code;
    resp_json["message"] = message;
    resp_json["data"] = data;

    resp->set_body(jsonToStr(resp_json));
}

void getArticleDetail(const request& req, response& resp, Application* app) {
    auto conn = app->getConnection();
    int code = 0;
    std::string message = "success";
    Json::Value data = {};

    Json::Value body;
    if (!strToJson(req->get_body(), body)) {
        code = 200;
        message = "invalid request";
        goto label_11;
    }
    else {
        std::string id = body["id"].asString();
        std::string sql = "select * from article where id = " + id;
        ResultSet* rs = conn->query(sql);
        if (rs->size() == 0) {
            code = 200;
            message = "article not found";
            goto label_11;
        }
        else {
            Result* res = rs->next();
            data["_id"] = res->getValue("id");
            data["title"] = res->getValue("title");
            data["author"] = res->getValue("author");
            data["desc"] = res->getValue("description");
            data["meta"]["views"] = std::stoi(res->getValue("views")) + 1;
            data["meta"]["comments"] = std::stoi(res->getValue("comments"));
            std::string kw = res->getValue("keyword");
            data["keyword"] = Json::Value(Json::arrayValue);
            if (!kw.empty()) {

                // split keyword into vector by comma, then append to data["keyword"]
                std::vector<std::string> keyword_list;
                boost::split(keyword_list, kw, boost::is_any_of(","));
                std::for_each(keyword_list.begin(),
                              keyword_list.end(),
                              [&data](std::string keyword) { data["keyword"].append(keyword); });
            }
            std::string flename = res->getValue("content");

            OSSClient client("pico-blog");
            std::string content;
            if (!client.download_data(flename, content)) {
                code = 200;
                message = "download failed";
                goto label_11;
            }
            data["content"] = content;
            data["img_url"] = res->getValue("img_url");
            // split tags and category by comma
            std::string tags = res->getValue("tags");
            std::string category = res->getValue("category");
            data["category"] = Json::Value(Json::arrayValue);
            data["tags"] = Json::Value(Json::arrayValue);
            if (!tags.empty()) {
                std::vector<std::string> tag_list;
                boost::split(tag_list, tags, boost::is_any_of(","));
                std::for_each(tag_list.begin(), tag_list.end(), [&data, conn](std::string tag) {
                    std::string sql = "select name from tag_category where id = " + tag;
                    ResultSet* rs = conn->query(sql);
                    if (rs->size() == 0) { data["tag"].append({}); }
                    else {
                        Result* res = rs->next();
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
                                  ResultSet* rs = conn->query(sql);
                                  if (rs->size() == 0) { data["category"].append({}); }
                                  else {
                                      Result* res = rs->next();
                                      // combine category name and id into json, then append to
                                      // item
                                      Json::Value category_item = {};
                                      category_item["id"] = category;
                                      category_item["name"] = res->getValue("name");
                                      data["category"].append(category_item);
                                  }
                              });
            }
            data["created_at"] = res->getValue("created_at");
            data["updated_at"] = res->getValue("updated_at");
            // data["comments"] = Json::Value(Json::arrayValue);
            data["comments"] = handle_comments(id);
        }
        // update article views
        sql = "update article set views = views + 1 where id = " + id;
        conn->query(sql);
    }
label_11:
    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = data;

    resp->set_body(jsonToStr(json_resp));
}

void updateArticle(const request& req, response& resp, Application* app) {
    auto conn = app->getConnection();
    int code = 0;
    std::string message = "success";
    Json::Value data = {};

    auto session = CTX(app, pico::Session, req).session;
    if (!session->has("username")) {
        code = 200;
        message = "not login";
        goto label_12;
    }
    {
        Json::Value body;
        if (!strToJson(req->get_body(), body)) {
            code = 200;
            message = "invalid request";
            goto label_12;
        }
        else {
            std::string id = body["id"].asString();
            std::string sql = "select content from article where id = " + id;
            ResultSet* rs = conn->query(sql);
            if (rs->size() == 0) {
                code = 200;
                message = "article not found";
                goto label_12;
            }
            else {
                Result* res = rs->next();
                std::string flename = res->getValue("content");

                OSSClient client("pico-blog");
                if (!client.delete_file(flename)) {
                    code = 200;
                    message = "delete failed";
                    goto label_12;
                }

                std::string title = body["title"].asString();
                std::string author = body["author"].asString();
                std::string desc = body["desc"].asString();
                std::string keyword = body["keyword"].asString();
                std::string content = body["content"].asString();
                std::string img_url = body["img_url"].asString();
                std::string tags = body["tags"].asString();
                std::string category = body["category"].asString();

                std::string filename = std::to_string(time(NULL)) + ".md";
                if (!client.upload_data(filename, content)) {
                    code = 200;
                    message = "upload failed";
                    goto label_12;
                }

                std::string sql_update = "update article set title = '" + title + "', author = '" +
                                         author + "', description = '" + desc + "', keyword = '" +
                                         keyword + "', content = '" + filename + "', img_url = '" +
                                         img_url + "', tags = '" + tags + "', category = '" +
                                         category + "', updated_at = now() where id = " + id;
                if (conn->query(sql_update)) {
                    code = 0;
                    message = "success";
                }
                else {
                    code = 200;
                    message = "database error";
                }
            }
        }
    }
label_12:

    Json::Value resp_json;
    resp_json["code"] = code;
    resp_json["message"] = message;
    resp_json["data"] = data;

    resp->set_body(jsonToStr(resp_json));
}

void delArticle(const request& req, response& resp, Application* app) {
    auto conn = app->getConnection();
    int code = 0;
    std::string message = "success";
    Json::Value data = {};

    auto session = CTX(app, pico::Session, req).session;
    if (!session->has("username")) {
        code = 200;
        message = "not login";
        goto label_13;
    }
    {
        Json::Value body;
        if (!strToJson(req->get_body(), body)) {
            code = 200;
            message = "invalid request";
            goto label_13;
        }
        else {
            std::string id = body["id"].asString();
            std::string sql = "select content from article where id = " + id;
            ResultSet* rs = conn->query(sql);
            if (rs->size() == 0) {
                code = 200;
                message = "article not found";
                goto label_13;
            }
            else {
                Result* res = rs->next();
                std::string flename = res->getValue("content");

                OSSClient client("pico-blog");
                if (!client.delete_file(flename)) {
                    code = 200;
                    message = "delete failed";
                    goto label_13;
                }

                std::string sql_delete = "delete from article where id = " + id;
                if (conn->query(sql_delete)) {
                    code = 0;
                    message = "success";
                }
                else {
                    code = 200;
                    message = "database error";
                }
            }
        }
    }
label_13:

    Json::Value resp_json;
    resp_json["code"] = code;
    resp_json["message"] = message;
    resp_json["data"] = data;

    resp->set_body(jsonToStr(resp_json));
}

void changeComment(const request& req, response& resp, Application* app) {
    auto conn = app->getConnection();
    int code = 0;
    std::string message = "success";
    Json::Value data = {};

    auto session = CTX(app, pico::Session, req).session;
    if (!session->has("username")) {
        code = 200;
        message = "not login";
        goto label_14;
    }
    {
        Json::Value body;
        if (!strToJson(req->get_body(), body)) {
            code = 200;
            message = "invalid request";
            goto label_14;
        }
        else {
            std::string id = body["id"].asString();
            std::string status = body["state"].asString();
            std::string sql = "update main_comment set status = " + status + " where id = " + id;
            if (conn->query(sql)) {
                code = 0;
                message = "success";
            }
            else {
                code = 200;
                message = "database error";
            }
        }
    }
label_14:

    Json::Value resp_json;
    resp_json["code"] = code;
    resp_json["message"] = message;
    resp_json["data"] = data;

    resp->set_body(jsonToStr(resp_json));
}

void getMessageList(const request& req, response& resp, Application* app) {
    auto conn = app->getConnection();
    int code = 0;
    std::string message = "success";
    Json::Value data = {};

    auto session = CTX(app, pico::Session, req).session;
    if (!session->has("username")) {
        code = 200;
        message = "not login";
        goto label_15;
    }
    {
        std::string keyword = req->get_param("keyword");
        int pageNum = std::stoi(req->get_param("pageNum"));
        int pageSize = std::stoi(req->get_param("pageSize"));
        std::string sql = "select * from main_comment where content like '%" + keyword +
                          "%' and status!=-1 order by created_at desc limit " +
                          std::to_string((pageNum - 1) * pageSize) + "," + std::to_string(pageSize);
        ResultSet* rs = conn->query(sql);
        data["count"] = rs->size();
        data["list"] = Json::Value(Json::arrayValue);
        Result* res = nullptr;
        while ((res = rs->next())) {
            Json::Value item;
            item["_id"] = res->getValue("id");
            item["user_id"] = res->getValue("user_id");
            item["content"] = res->getValue("content");
            item["state"] = res->getValue("status");
            item["create_time"] = res->getValue("created_at");
            item["update_time"] = res->getValue("updated_at");
            item["avatar"] = "user";
            item["reply_list"] = get_reply_message(res->getValue("id"), res->getValue("user_id"));
            // get user info
            sql = "select * from user where id = " + res->getValue("user_id");
            ResultSet* rs_user = conn->query(sql);
            if (rs_user->size() > 0) {
                Result* res_user = rs_user->next();
                item["name"] = res_user->getValue("name");
                item["email"] = res_user->getValue("email");
            }
            data["list"].append(item);
        }
    }
label_15:

    Json::Value resp_json;
    resp_json["code"] = code;
    resp_json["message"] = message;
    resp_json["data"] = data;

    resp->set_body(jsonToStr(resp_json));
}

void getMessageDetail(const request& req, response& resp, Application* app) {
    auto conn = app->getConnection();
    int code = 0;
    std::string message = "success";
    Json::Value data = {};

    auto session = CTX(app, pico::Session, req).session;
    if (!session->has("username")) {
        code = 200;
        message = "not login";
        goto label_16;
    }
    {
        Json::Value body;
        if (!strToJson(req->get_body(), body)) {
            code = 200;
            message = "invalid request";
            goto label_16;
        }
        else {
            std::string id = body["id"].asString();
            std::string sql = "select * from main_comment where id = " + id;
            ResultSet* rs = conn->query(sql);
            if (rs->size() == 0) {
                code = 200;
                message = "message not found";
                goto label_16;
            }
            else {
                Result* res = rs->next();
                data["_id"] = res->getValue("id");
                data["user_id"] = res->getValue("user_id");
                data["content"] = res->getValue("content");
                data["state"] = res->getValue("status");
                data["create_time"] = res->getValue("created_at");
                data["update_time"] = res->getValue("updated_at");
                data["avatar"] = "user";
                // get user info
                sql = "select * from user where id = " + res->getValue("user_id");
                ResultSet* rs_user = conn->query(sql);
                if (rs_user->size() > 0) {
                    Result* res_user = rs_user->next();
                    data["name"] = res_user->getValue("name");
                    data["email"] = res_user->getValue("email");
                }
                // get reply message
                data["reply_list"] =
                    get_reply_message(res->getValue("id"), res->getValue("user_id"));
            }
        }
    }
label_16:

    Json::Value resp_json;
    resp_json["code"] = code;
    resp_json["message"] = message;
    resp_json["data"] = data;

    resp->set_body(jsonToStr(resp_json));
}


void addReplyMessage(const request& req, response& resp, Application* app) {
    auto conn = app->getConnection();
    int code = 0;
    std::string message = "success";
    Json::Value data = {};

    auto session = CTX(app, pico::Session, req).session;
    if (!session->has("username")) {
        code = 200;
        message = "not login";
        goto label_17;
    }
    {
        Json::Value body;
        if (!strToJson(req->get_body(), body)) {
            code = 200;
            message = "invalid request";
            goto label_17;
        }
        else {
            std::string id = body["id"].asString();
            std::string content = body["content"].asString();

            std::string current_user_id = session->get("user_id").asString();

            std::string query_sql = "select article_id ,user_id from main_comment where id = " + id;
            ResultSet* rs = conn->query(query_sql);
            if (rs->size() == 0) {
                code = 200;
                message = "message not found";
                goto label_17;
            }
            else {
                Result* res = rs->next();
                std::string article_id = res->getValue("article_id");
                std::string user_id = res->getValue("user_id");
                std::string insert_sql = "insert into reply_comment "
                                         "(article_id,main_comment_id,from_user_id,to_user_id,"
                                         "content,created_at) values (" +
                                         article_id + "," + id + "," + current_user_id + "," +
                                         user_id + ",'" + content + "',now())";
                if (!conn->update(insert_sql)) {
                    code = 200;
                    message = "insert reply message failed";
                    goto label_17;
                }
                else {
                    std::string update_sql =
                        "update article set comments = comments + 1 where id = " + article_id;
                    if (!conn->update(update_sql)) {
                        code = 200;
                        message = "update article comments failed";
                        goto label_17;
                    }
                }
            }
        }
    }
label_17:

    Json::Value resp_json;
    resp_json["code"] = code;
    resp_json["message"] = message;
    resp_json["data"] = data;

    resp->set_body(jsonToStr(resp_json));
}

void delMessage(const request& req, response& resp, Application* app) {
    auto conn = app->getConnection();
    int code = 0;
    std::string message = "success";
    Json::Value data = {};

    auto session = CTX(app, pico::Session, req).session;
    if (!session->has("username")) {
        code = 200;
        message = "not login";
        goto label_18;
    }
    {
        Json::Value body;
        if (!strToJson(req->get_body(), body)) {
            code = 200;
            message = "invalid request";
            goto label_18;
        }
        else {
            std::string id = body["id"].asString();
            std::string sql = "select * from main_comment where id = " + id;
            ResultSet* rs = conn->query(sql);
            if (rs->size() == 0) {
                code = 200;
                message = "message not found";
                goto label_18;
            }
            else {
                Result* res = rs->next();
                std::string article_id = res->getValue("article_id");
                std::string user_id = res->getValue("user_id");
                std::string delete_sql = "delete from main_comment where id = " + id;
                if (!conn->update(delete_sql)) {
                    code = 200;
                    message = "delete message failed";
                    goto label_18;
                }
                else {
                    std::string update_sql =
                        "update article set comments = comments - 1 where id = " + article_id;
                    if (!conn->update(update_sql)) {
                        code = 200;
                        message = "update article comments failed";
                        goto label_18;
                    }
                }
            }
        }
    }
label_18:

    Json::Value resp_json;
    resp_json["code"] = code;
    resp_json["message"] = message;
    resp_json["data"] = data;

    resp->set_body(jsonToStr(resp_json));
}

void changeThirdComment(const request& req, response& resp, Application* app) {
    auto conn = app->getConnection();
    int code = 0;
    std::string message = "success";
    Json::Value data = {};

    auto session = CTX(app, pico::Session, req).session;
    if (!session->has("username")) {
        code = 200;
        message = "not login";
        goto label_19;
    }
    {
        Json::Value body;
        if (!strToJson(req->get_body(), body)) {
            code = 200;
            message = "invalid request";
            goto label_19;
        }
        else {
            std::string main_comment_id = body["id"].asString();
            std::string reply_comment_id = body["_id"].asString();

            std::string sql = "select * from reply_comment where id = " + reply_comment_id +
                              " and main_comment_id = " + main_comment_id;
            ResultSet* rs = conn->query(sql);

            if (rs->size() == 0) {
                code = 200;
                message = "message not found";
                goto label_19;
            }
            else {
                std::string article_id = rs->next()->getValue("article_id");
                std::string del_sql = "delete from reply_comment where id = " + reply_comment_id +
                                      " and main_comment_id = " + main_comment_id;
                if (!conn->update(del_sql)) {
                    code = 200;
                    message = "delete message failed";
                    goto label_19;
                }
                else {
                    std::string update_sql =
                        "update article set comments = comments - 1 where id = " + article_id;
                    if (!conn->update(update_sql)) {
                        code = 200;
                        message = "update article comments failed";
                        goto label_19;
                    }
                }
            }
        }
    }
label_19:

    Json::Value resp_json;
    resp_json["code"] = code;
    resp_json["message"] = message;
    resp_json["data"] = data;

    resp->set_body(jsonToStr(resp_json));
}

void getArticleList(const request& req, response& resp, Application* app) {
    auto conn = app->getConnection();
    int code = 0;
    std::string message = "success";
    Json::Value data = {};


    int page = std::stoi(req->get_param("pageNum"));
    int page_size = std::stoi(req->get_param("pageSize"));
    std::string keyword = req->get_param("keyword");
    std::string tag_id = req->get_param("tag_id");
    std::string title = req->get_param("title");
    std::string category_id = req->get_param("category_id");

    keyword = url_decode(keyword);
    title = url_decode(title);

    std::string sql = " select * from article where keyword like '%" + keyword + "%'" +
                      " and tags like '%" + tag_id + "%' and category like '%" + category_id +
                      "%' and title like '%" + title + "%' order by id desc limit " +
                      std::to_string((page - 1) * page_size) + "," + std::to_string(page_size);
    ResultSet* rs = conn->query(sql);
    data["count"] = rs->size();
    Result* res = nullptr;
    while ((res = rs->next()) != nullptr) {
        Json::Value article;
        article["_id"] = res->getValue("id");
        article["title"] = res->getValue("title");
        article["desc"] = res->getValue("description");
        article["meta"]["views"] = std::stoi(res->getValue("views"));
        article["meta"]["comments"] = std::stoi(res->getValue("comments"));
        article["create_time"] = res->getValue("created_at");
        article["img_url"] = res->getValue("img_url");
        data["list"].append(article);
    }

    Json::Value resp_json;
    resp_json["code"] = code;
    resp_json["message"] = message;
    resp_json["data"] = data;

    resp->set_body(jsonToStr(resp_json));
}

void addComment(const request& req, response& resp, Application* app) {
    auto conn = app->getConnection();
    int code = 0;
    std::string message = "success";
    Json::Value data = {};

    Json::Value body;
    if (!strToJson(req->get_body(), body)) {
        code = 200;
        message = "invalid request";
        goto label_21;
    }
    else {
        std::string article_id = body["article_id"].asString();
        Json::Value comment_data = body["data"];

        std::string comment_content = comment_data["content"].asString();
        std::string comment_username = comment_data["username"].asString();
        std::string comment_email = comment_data["email"].asString();

        std::string user_id = get_user_id(comment_email, comment_username);
        if (user_id == "") {
            code = 200;
            message = "user not found";
            goto label_21;
        }
        std::string sql =
            "insert into main_comment(article_id,user_id,content,status,created_at,updated_at) "
            "values(" +
            article_id + "," + user_id + ",'" + comment_content + "',1,now(),now())";
        if (!conn->update(sql)) {
            code = 200;
            message = "add comment failed";
            goto label_21;
        }
        else {
            std::string update_sql =
                "update article set comments = comments + 1 where id = " + article_id;
            if (!conn->update(update_sql)) {
                code = 200;
                message = "update article comments failed";
                goto label_21;
            }
        }
    }
label_21:

    Json::Value resp_json;
    resp_json["code"] = code;
    resp_json["message"] = message;
    resp_json["data"] = data;

    resp->set_body(jsonToStr(resp_json));
}

void addThirdComment(const request& req, response& resp, Application* app) {
    auto conn = app->getConnection();
    int code = 0;
    std::string message = "success";
    Json::Value data = {};

    Json::Value body;
    if (!strToJson(req->get_body(), body)) {
        code = 200;
        message = "invalid request";
        goto label_22;
    }
    else {
        std::string article_id = body["article_id"].asString();
        std::string comment_id = body["comment_id"].asString();
        std::string content = body["content"].asString();

        Json::Value from_user, to_user;
        if (!strToJson(body["from_user"].asString(), from_user) ||
            !strToJson(body["to_user"].asString(), to_user)) {
            code = 200;
            message = "invalid request";
            goto label_22;
        }

        // query from_user id from users
        std::string from_user_id =
            get_user_id(from_user["email"].asString(), from_user["username"].asString());
        // get to_user id from users table
        std::string to_user_name = to_user["name"].asString();
        std::string query_sql =
            "select id from user where name = '" + to_user_name + "' and role = 0";
        ResultSet* rs = conn->query(query_sql);
        Result* res = nullptr;
        if ((res = rs->next()) == nullptr) {
            code = 200;
            message = "to_user not found";
            goto label_22;
        }
        std::string to_user_id = res->getValue("id");
        // insert int reply_comment
        std::string sql = "insert into "
                          "reply_comment(article_id,main_comment_id,from_user_id,to_user_id,"
                          "content,created_at) "
                          "values(" +
                          article_id + "," + comment_id + "," + from_user_id + "," + to_user_id +
                          ",'" + content + "',now())";
        if (!conn->update(sql)) {
            code = 200;
            message = "add third comment failed";
            goto label_22;
        }
        else {
            std::string update_sql =
                "update article set comments = comments + 1 where id = " + article_id;
            if (!conn->update(update_sql)) {
                code = 200;
                message = "update article comments failed";
                goto label_22;
            }
        }
    }
label_22:

    Json::Value resp_json;
    resp_json["code"] = code;
    resp_json["message"] = message;
    resp_json["data"] = data;

    resp->set_body(jsonToStr(resp_json));
}