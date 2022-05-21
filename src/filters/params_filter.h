#ifndef __PARAMS_FILTER_H__
#define __PARAMS_FILTER_H__

#include <string>

#include "pico/class_factory.h"
#include "pico/filter.h"
#include "src/util.h"

class ParamsFilter : public pico::Filter
{
public:
    void doFilter(const pico::HttpRequest::Ptr& request, pico::HttpResponse::Ptr& response,
                  std::shared_ptr<pico::FilterChain> chain) override;
};


REGISTER_CLASS(ParamsFilter);

#endif