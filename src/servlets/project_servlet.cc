#include "project_servlet.h"

#include "../tables.hpp"
#include "pico/session.h"
#include "pico/util.h"
#include "util.h"

void GetProjectListServlet::doGet(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) {
    std::string keyword = req->get_param("keyword");
    int page = req->get_param("pageNum").empty() ? 1 : std::stoi(req->get_param("pageNum"));
    int page_size = req->get_param("pageSize").empty() ? 10 : std::stoi(req->get_param("pageSize"));
    std::string state = req->get_param("state");

    pico::RowBoundsMapper<project> mapper("sql_1");
    pico::Base<project> base;
    auto criteria = base.createCriteria();
    criteria->andLike(&project::title, "%" + keyword + "%");
    if (!state.empty()) { criteria->andEqualTo(&project::status, state); }

    pico::RowBounds row_bounds((page - 1) * page_size, page_size);
    auto ret = mapper.selectByRowBounds(base, row_bounds);


    int code = 0;
    std::string message = "success";
    res->set_status(pico::HttpStatus::OK);

    Json::Value data = {};
    data["list"] = Json::Value(Json::arrayValue);

    data["count"] = ret.size();
    for (auto& item : ret) {
        Json::Value item_data = {};
        item_data["_id"] = item.id;
        item_data["title"] = item.title;
        item_data["start_time"] = pico::Time2Str(item.created_at);
        item_data["end_time"] = pico::Time2Str(item.end_at);
        item_data["content"] = item.description;
        item_data["img"] = item.img;
        item_data["url"] = item.url;
        item_data["state"] = item.status;
        data["list"].append(item_data);
    }


    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = data;

    res->set_body(jsonToStr(json_resp));
}

void DelProjectServlet::doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) {
    std::string body = req->get_body();
    Json::Value json_req;
    if (!strToJson(body, json_req)) {
        res->set_status(pico::HttpStatus::BAD_REQUEST);
        res->set_body("invalid request");
        return;
    }

    std::string id = json_req.get("id", "").asString();

    int code = 0;
    std::string message = "success";
    res->set_status(pico::HttpStatus::OK);

    if (id.empty()) {
        code = 200;
        message = "invalid parameter";
    }
    else {
        pico::Mapper<project> mapper;
        mapper.use("sql_1");
        auto ret = mapper.deleteByPrimaryKey(id);
        if (!ret) {
            code = 200;
            message = "delete failed";
        }
    }

    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = {};

    res->set_body(jsonToStr(json_resp));
}

void AddProjectServlet::doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) {
    std::string body = req->get_body();
    Json::Value json;
    if (!strToJson(body, json)) {
        res->set_status(pico::HttpStatus::BAD_REQUEST);
        res->set_body("invalid request");
        return;
    }

    int state = std::stoi(json.get("state", "3").asString());
    std::string title = json.get("title", "").asString();
    std::string description = json.get("content", "").asString();
    std::string img = json.get("img", "").asString();
    std::string url = json.get("url", "").asString();
    std::string start_time = format_time(json.get("start_time", "").asString());
    std::string end_time = format_time(json.get("end_time", "").asString());

    int code = 0;
    std::string message = "success";
    res->set_status(pico::HttpStatus::OK);

    pico::Mapper<project> mapper;
    mapper.use("sql_1");

    project item(title,
                 description,
                 img,
                 url,
                 pico::Str2Time(start_time.data()),
                 time(0),
                 pico::Str2Time(end_time.data()),
                 state);

    auto ret = mapper.insert(item);
    if (!ret) {
        code = 200;
        message = "insert failed";
    }

    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = {};

    res->set_body(jsonToStr(json_resp));
}

void GetProjectDetailServlet::doPost(const pico::HttpRequest::Ptr& req,
                                     pico::HttpResponse::Ptr& res) {
    std::string body = req->get_body();
    Json::Value json;
    if (!strToJson(body, json)) {
        res->set_status(pico::HttpStatus::BAD_REQUEST);
        res->set_body("invalid request body");
        return;
    }

    std::string id = json.get("id", "").asString();

    int code = 0;
    std::string message = "success";
    res->set_status(pico::HttpStatus::OK);

    Json::Value data = {};

    if (id.empty()) {
        code = 200;
        message = "invalid parameter";
    }
    else {
        pico::Mapper<project> mapper;
        mapper.use("sql_1");
        if (mapper.existsWithPrimaryKey(id)) {
            auto ret = mapper.selectByPrimaryKey(id);
            data["_id"] = ret.id;
            data["title"] = ret.title;
            data["start_time"] = pico::Time2Str(ret.created_at);
            data["end_time"] = pico::Time2Str(ret.end_at);
            data["content"] = ret.description;
            data["img"] = ret.img;
            data["url"] = ret.url;
            data["state"] = std::to_string(ret.status);
        }
    }

    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = data;

    res->set_body(jsonToStr(json_resp));
}

void UpdateProjectServlet::doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) {
    std::string body = req->get_body();
    Json::Value json;
    if (!strToJson(body, json)) {
        res->set_status(pico::HttpStatus::BAD_REQUEST);
        res->set_body("invalid request body");
        return;
    }

    std::string id = json.get("id", "").asString();
    int state = std::stoi(json.get("state", "3").asString());
    std::string title = json.get("title", "").asString();
    std::string description = json.get("content", "").asString();
    std::string img = json.get("img", "").asString();
    std::string url = json.get("url", "").asString();
    std::time_t start_time = pico::Str2Time(json.get("start_time", "").asString().data());
    std::time_t end_time = pico::Str2Time(json.get("end_time", "").asString().data());

    int code = 0;
    std::string message = "success";
    res->set_status(pico::HttpStatus::OK);

    pico::Mapper<project> mapper;

    project item(title, description, img, url, start_time, time(0), end_time, state);

    if (!id.empty() || mapper.existsWithPrimaryKey(id)) {
        item.id = std::stoi(id);
        auto ret = mapper.updateByPrimaryKey(item);
        if (!ret) {
            code = 200;
            message = "update failed";
        }
    }
    else {
        code = 200;
        message = "invalid parameter";
    }

    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = {};

    res->set_body(jsonToStr(json_resp));
}