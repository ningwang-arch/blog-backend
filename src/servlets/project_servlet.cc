#include "project_servlet.h"

#include "pico/session.h"
#include "util.h"

void GetProjectListServlet::doGet(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) {
    auto conn = get_connection();

    std::string keyword = req->get_param("keyword");
    int page = req->get_param("pageNum").empty() ? 1 : std::stoi(req->get_param("pageNum"));
    int page_size = req->get_param("pageSize").empty() ? 10 : std::stoi(req->get_param("pageSize"));
    std::string state = req->get_param("state");

    std::string sql;
    if (state.empty()) { sql = "select * from project where title like '%" + keyword + "%'"; }
    else {
        sql = "select * from project where title like '%" + keyword + "%' and status = " + state;
    }

    int code = 0;
    std::string message = "success";
    res->set_status(pico::HttpStatus::OK);

    Json::Value data = {};

    auto rs = conn->query(sql);
    if (!rs) {
        code = 200;
        message = "database error";
    }
    else {
        rs->offset((page - 1) * page_size, page_size);
        data["count"] = rs->size();

        Result::Ptr ret = nullptr;
        Json::Value ca_list = {};
        while ((ret = rs->next())) {
            Json::Value item = {};
            item["_id"] = ret->getValue("id");
            item["title"] = ret->getValue("title");
            item["start_time"] = ret->getValue("created_at");
            item["end_time"] = ret->getValue("end_at");
            item["content"] = ret->getValue("description");
            item["img"] = ret->getValue("img");
            item["url"] = ret->getValue("url");
            item["state"] = ret->getValue("status");
            ca_list.append(item);
        }
        data["list"] = ca_list;
    }

    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = data;

    res->set_body(jsonToStr(json_resp));
}

void DelProjectServlet::doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) {
    auto conn = get_connection();
    std::string body = req->get_body();
    Json::Value json_req;
    if (!strToJson(body, json_req)) {
        res->set_status(pico::HttpStatus::BAD_REQUEST);
        res->set_body("invalid request");
        return;
    }

    std::string id = json_req["id"].asString();

    int code = 0;
    std::string message = "success";
    res->set_status(pico::HttpStatus::OK);


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


    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = {};

    res->set_body(jsonToStr(json_resp));
}

void AddProjectServlet::doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) {
    auto conn = get_connection();
    std::string body = req->get_body();
    Json::Value json;
    if (!strToJson(body, json)) {
        res->set_status(pico::HttpStatus::BAD_REQUEST);
        res->set_body("invalid request");
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
    res->set_status(pico::HttpStatus::OK);


    if (!CheckParameter(title) || !CheckParameter(description) || !CheckParameter(img) ||
        !CheckParameter(url) || !CheckParameter(start_time) || !CheckParameter(end_time)) {
        code = 200;
        message = "invalid parameter";
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

    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = {};

    res->set_body(jsonToStr(json_resp));
}

void GetProjectDetailServlet::doPost(const pico::HttpRequest::Ptr& req,
                                     pico::HttpResponse::Ptr& res) {
    auto conn = get_connection();

    std::string body = req->get_body();
    Json::Value json;
    if (!strToJson(body, json)) {
        res->set_status(pico::HttpStatus::BAD_REQUEST);
        res->set_body("invalid request body");
        return;
    }

    std::string id = json["id"].asString();

    int code = 0;
    std::string message = "success";
    res->set_status(pico::HttpStatus::OK);

    Json::Value data = {};


    if (!CheckParameter(id)) {
        code = 200;
        message = "invalid parameter";
    }
    else {
        std::string sql = "select * from project where id = " + id;
        auto rs = conn->query(sql);
        if (!rs) {
            code = 200;
            message = "database error";
        }
        else if (rs->size() == 0) {
            code = 200;
            message = "not found";
        }
        else {
            Result::Ptr ret = nullptr;
            if ((ret = rs->next())) {
                data["_id"] = ret->getValue("id");
                data["title"] = ret->getValue("title");
                data["start_time"] = ret->getValue("created_at");
                data["end_time"] = ret->getValue("end_at");
                data["content"] = ret->getValue("description");
                data["img"] = ret->getValue("img");
                data["url"] = ret->getValue("url");
                data["state"] = ret->getValue("status");
            }
        }
    }

    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = data;

    res->set_body(jsonToStr(json_resp));
}

void UpdateProjectServlet::doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) {
    auto conn = get_connection();
    std::string body = req->get_body();
    Json::Value json;
    if (!strToJson(body, json)) {
        res->set_status(pico::HttpStatus::BAD_REQUEST);
        res->set_body("invalid request body");
        return;
    }

    std::string id = json["id"].asString();
    std::string state = json["state"].asString();
    std::string title = json["title"].asString();
    std::string description = json["content"].asString();
    std::string img = json["img"].asString();
    std::string url = json["url"].asString();
    std::string start_time = format_time(json["start_time"].asString());
    std::string end_time = format_time(json["end_time"].asString());

    int code = 0;
    std::string message = "success";
    res->set_status(pico::HttpStatus::OK);


    if (!CheckParameter(id) || !CheckParameter(state) || !CheckParameter(title) ||
        !CheckParameter(description) || !CheckParameter(img) || !CheckParameter(url) ||
        !CheckParameter(start_time) || !CheckParameter(end_time)) {
        code = 200;
        message = "invalid parameter";
    }
    else {
        std::string sql = "select * from project where id = " + id;
        std::shared_ptr<ResultSet> rs = conn->query(sql);
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

    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = {};

    res->set_body(jsonToStr(json_resp));
}