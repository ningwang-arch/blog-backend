#include "tag_servlet.h"

#include "pico/session.h"
#include "util.h"

void GetTagListServlet::doGet(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) {
    auto conn = get_connection();

    std::string keyword = req->get_param("keyword");
    int page = req->get_param("pageNum").empty() ? 1 : std::stoi(req->get_param("pageNum"));
    int page_size = req->get_param("pageSize").empty() ? 10 : std::stoi(req->get_param("pageSize"));

    if (!CheckParameter(keyword)) { keyword = ""; }

    std::string sql = "select * from tag_category where name like '%" + keyword + "%' and t_c =0";

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
            Json::Value ca;
            ca["_id"] = ret->getValue("id");
            ca["name"] = ret->getValue("name");
            ca["desc"] = ret->getValue("desc");
            ca["create_time"] = ret->getValue("created_at");
            ca_list.append(ca);
        }
        data["list"] = ca_list;
    }

    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = data;

    res->set_body(jsonToStr(json_resp));
}

void DelTagServlet::doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) {
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
        message = "invalid id";
    }
    else {
        std::string sql = "delete from tag_category where id = " + id;
        if (!conn->update(sql)) {
            code = 200;
            message = "database error";
        }
    }


    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = "";

    res->set_body(jsonToStr(json_resp));
}

void AddTagServlet::doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) {
    auto conn = get_connection();
    std::string body = req->get_body();
    Json::Value json_req;
    if (!strToJson(body, json_req)) {
        res->set_status(pico::HttpStatus::BAD_REQUEST);
        res->set_body("invalid request");
        return;
    }

    std::string name = json_req["name"].asString();
    std::string desc = json_req["desc"].asString();

    int code = 0;
    std::string message = "success";
    res->set_status(pico::HttpStatus::OK);

    if (!CheckParameter(name)) {
        code = 200;
        message = "invalid name";
    }
    else {
        std::string sql = "select * from tag_category where name = '" + name +
                          "' and t_c = 0 and `desc` = '" + desc + "'";
        auto rs = conn->query(sql);
        if (!rs) {
            code = 200;
            message = "database error";
        }
        else {
            if (rs->size() > 0) {
                code = 200;
                message = "tag already exists";
            }
            else {
                std::string sql =
                    "insert into tag_category (name, `desc`, t_c, created_at) values ('" + name +
                    "', '" + desc + "', 0, now())";
                auto rs = conn->query(sql);
                if (!rs) {
                    code = 200;
                    message = "database error";
                }
            }
        }
    }

    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = "";

    res->set_body(jsonToStr(json_resp));
}