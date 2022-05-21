#include "root_servlet.h"

#include <fstream>
#include <json/json.h>
#include <sstream>

#include "util.h"

#include "pico/config.h"


static pico::ConfigVar<std::string>::Ptr g_blog_log_path =
    pico::Config::Lookup<std::string>("other.log.blog.path", std::string(), "path of blog log");

static pico::ConfigVar<std::string>::Ptr g_blog_log_file =
    pico::Config::Lookup<std::string>("other.log.blog.file", std::string(), "filename of blog log");

static pico::ConfigVar<std::string>::Ptr g_nginx_log_path =
    pico::Config::Lookup<std::string>("other.log.nginx.path", std::string(), "path of nginx log");

void RootServlet::doGet(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& resp) {
    resp->set_status(pico::HttpStatus::OK);
    resp->set_body("Hello, world!");
}

void CurrentUserServlet::doGet(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& resp) {
    int code = 0;
    std::string message = "success";
    Json::Value data = {};

    data["name"] = "eclipse";
    data["avatar"] = "https://gw.alipayobjects.com/zos/rmsportal/BiazfanxmamNRoxxVxka.png";
    data["userid"] = "00000001";
    data["email"] = "ningwang778@gmail.com";
    data["signature"] = "海纳百川，有容乃大";
    data["title"] = "交互专家";
    data["group"] = "eclipse";
    data["tags"] = Json::Value(Json::arrayValue);
    data["notifyCount"] = 12;
    data["country"] = "China";
    data["geographic"] = Json::Value(Json::objectValue);
    data["address"] = "";
    data["phone"] = "";


    Json::Value resp_json;
    resp_json["code"] = code;
    resp_json["message"] = message;
    resp_json["data"] = data;

    resp->set_body(jsonToStr(resp_json));
}

void GetLogServlet::doGet(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& resp) {
    std::string path = req->get_path();
    std::string content;

    int pos = path.find("/log/");

    path = path.substr(pos + 5);

    std::string file;
    if (path == g_blog_log_file->getValue()) {
        std::string blog_path = g_blog_log_path->getValue();
        file = blog_path + (blog_path[blog_path.size() - 1] == '/' ? "" : "/") + path;
    }
    else {
        std::string nginx_log = g_nginx_log_path->getValue();
        file = nginx_log + (nginx_log[nginx_log.size() - 1] == '/' ? "" : "/") + path;
    }
    // open file
    std::ifstream ifs(file);
    if (!ifs.is_open()) { goto label_22; }
    // read file
    {
        std::stringstream buffer;
        buffer << ifs.rdbuf();
        content = buffer.str();
    }
    // close file
    ifs.close();

label_22:
    resp->set_status(pico::HttpStatus::OK);
    resp->set_header("Content-Type", "text/plain");
    resp->set_body(content);
}