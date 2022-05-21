#ifndef __SESSION_FILTER_H__
#define __SESSION_FILTER_H__

#include <json/json.h>
#include <string>

#include "pico/class_factory.h"
#include "pico/filter.h"
#include "pico/session.h"

class SessionFilter : public pico::Filter
{
public:
    void doFilter(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res,
                  std::shared_ptr<pico::FilterChain> chain) override;
};

REGISTER_CLASS(SessionFilter);

#endif