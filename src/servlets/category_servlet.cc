#include "category_servlet.h"

#include "pico/session.h"
#include "util.h"

void GetCategoryListServlet::doGet(const pico::HttpRequest::Ptr& req,
                                   pico::HttpResponse::Ptr& res) {
    auto conn = get_connection();

    std::string keyword = req->get_param("keyword");
    int page = req->get_param("pageNum").empty() ? 1 : std::stoi(req->get_param("pageNum"));
    int page_size = req->get_param("pageSize").empty() ? 10 : std::stoi(req->get_param("pageSize"));

    if (!CheckParameter(keyword)) { keyword = ""; }

    std::string sql = "select * from tag_category where name like '%" + keyword + "%' and t_c =1";

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

        Result* ret = nullptr;
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

void DelCategoryServlet::doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) {
    auto conn = get_connection();
    std::string body = req->get_body();
    Json::Value json;
    if (!strToJson(body, json)) {
        res->set_status(pico::HttpStatus::BAD_REQUEST);
        res->set_body("Bad Request");
        return;
    }

    std::string id = json["id"].asString();

    int code = 0;
    std::string message = "success";
    res->set_status(pico::HttpStatus::OK);

    auto session = pico::SessionManager::getInstance()->getRequestSession(req, res);
    if (!session || !session->has("username")) {
        code = 200;
        message = "not login";
    }
    else {
        if (!CheckParameter(id)) {
            code = 200;
            message = "invalid id";
        }
        else {
            std::string sql = "delete from tag_category where id = '" + id + "'";

            if (!conn->update(sql)) {
                code = 200;
                message = "database error";
            }
        }
    }

    Json::Value json_resp;
    json_resp["code"] = code;
    json_resp["message"] = message;
    json_resp["data"] = "";

    res->set_body(jsonToStr(json_resp));
}

void AddCategoryServlet::doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) {
    auto conn = get_connection();
    std::string body = req->get_body();
    Json::Value json;
    if (!strToJson(body, json)) {
        res->set_status(pico::HttpStatus::BAD_REQUEST);
        res->set_body("Bad Request");
        return;
    }

    std::string name = json["name"].asString();
    std::string desc = json["desc"].asString();

    int code = 0;
    std::string message = "success";
    res->set_status(pico::HttpStatus::OK);

    auto session = pico::SessionManager::getInstance()->getRequestSession(req, res);
    if (!session || !session->has("username")) {
        code = 200;
        message = "not login";
    }
    else {
        if (!CheckParameter(name) || !CheckParameter(desc)) {
            code = 200;
            message = "invalid name or desc";
        }
        else {
            std::string sql = "select * from tag_category where name = '" + name +
                              "' and t_c = 1 and `desc` = '" + desc + "'";
            auto rs = conn->query(sql);
            if (rs && rs->size() > 0) {
                code = 200;
                message = "category already exists";
            }
            else {
                sql = "insert into tag_category (name, `desc`, t_c, created_at,updated_at) values "
                      "('" +
                      name + "', '" + desc + "', 1, now(), now())";
                if (!conn->update(sql)) {
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
