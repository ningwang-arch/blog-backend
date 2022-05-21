#include "session_filter.h"

void SessionFilter::doFilter(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res,
                             std::shared_ptr<pico::FilterChain> chain) {
    auto session = pico::SessionManager::getInstance()->getRequestSession(req, res);
    if (!session || !session->has("username")) {
        Json::Value json;
        json["code"] = 200;
        json["message"] = "please login";
        json["data"] = {};
        res->set_body(json.toStyledString());
        return;
    }

    chain->doFilter(req, res);
}