#include "tag_servlet.h"

#include "../tables.hpp"
#include "pico/session.h"
#include "util.h"

void GetTagListServlet::doGet(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) {
    std::string keyword = req->get_param("keyword");
    int page = req->get_param("pageNum").empty() ? 1 : std::stoi(req->get_param("pageNum"));
    int page_size = req->get_param("pageSize").empty() ? 10 : std::stoi(req->get_param("pageSize"));


    pico::RowBoundsMapper<tag_category> mapper("sql_1");
    pico::Base<tag_category> base;
    auto criteria = base.createCriteria();
    criteria->andEqualTo(&tag_category::t_c, 0);
    if (!keyword.empty()) { criteria->andLike(&tag_category::name, "%" + keyword + "%"); }

    pico::RowBounds row_bounds((page - 1) * page_size, page_size);
    auto ret = mapper.selectByRowBounds(base, row_bounds);

    int code = 0;
    std::string message = "success";
    res->set_status(pico::HttpStatus::OK);

    Json::Value data = {};

    data["count"] = ret.size();
    data["list"] = Json::Value(Json::arrayValue);
    for (auto& item : ret) {
        Json::Value item_data = {};
        item_data["_id"] = item.id;
        item_data["name"] = item.name;
        item_data["desc"] = item.desc;
        item_data["create_time"] = pico::Time2Str(item.created_at);
        data["list"].append(item_data);
    }

    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = data;

    res->set_body(jsonToStr(json_resp));
}

void DelTagServlet::doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) {
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

    pico::Mapper<tag_category> mapper;
    mapper.use("sql_1");
    auto ret = mapper.deleteByPrimaryKey(id);
    if (!ret) {
        code = 200;
        message = "delete failed";
    }


    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = "";

    res->set_body(jsonToStr(json_resp));
}

void AddTagServlet::doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) {
    std::string body = req->get_body();
    Json::Value json;
    if (!strToJson(body, json)) {
        res->set_status(pico::HttpStatus::BAD_REQUEST);
        res->set_body("Bad Request");
        return;
    }

    std::string name = json.get("name", "").asString();
    std::string desc = json.get("desc", "").asString();

    int code = 0;
    std::string message = "success";
    res->set_status(pico::HttpStatus::OK);


    if (name.empty() || desc.empty()) {
        code = 200;
        message = "invalid name or desc";
    }
    else {
        pico::Mapper<tag_category> mapper;
        mapper.use("sql_1");
        // select
        pico::Base<tag_category> base;
        auto criteria = base.createCriteria();
        criteria->andEqualTo(&tag_category::name, name)
            ->andEqualTo(&tag_category::desc, desc)
            ->andEqualTo(&tag_category::t_c, 0);
        auto ret = mapper.select(base);
        if (ret.size() > 0) {
            code = 200;
            message = "tag already exists";
        }
        else {
            auto ret = mapper.insert(tag_category{name, desc, time(nullptr), time(nullptr), 0});
            if (!ret) {
                code = 200;
                message = "insert failed";
            }
        }
    }

    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = "";

    res->set_body(jsonToStr(json_resp));
}