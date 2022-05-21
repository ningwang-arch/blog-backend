#include "params_filter.h"

void ParamsFilter::doFilter(const pico::HttpRequest::Ptr& request,
                            pico::HttpResponse::Ptr& response,
                            std::shared_ptr<pico::FilterChain> chain) {
    auto params = request->get_params();
    if (params.empty()) {
        chain->doFilter(request, response);
        return;
    }
    for (auto param : params) {
        if (!CheckParameter(param.second)) {
            Json::Value json;
            json["code"] = 200;
            json["message"] = "parameter error";
            json["data"] = {};
            response->set_body(json.toStyledString());
            return;
        }
    }

    chain->doFilter(request, response);
}