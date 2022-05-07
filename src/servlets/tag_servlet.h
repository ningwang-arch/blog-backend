#ifndef __TAG_SERVLET_H__
#define __TAG_SERVLET_H__

#include "pico/class_factory.h"
#include "pico/http/servlet.h"

class GetTagListServlet : public pico::Servlet
{
public:
    void doGet(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) override;
};

class DelTagServlet : public pico::Servlet
{
public:
    void doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) override;
};

class AddTagServlet : public pico::Servlet
{
public:
    void doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) override;
};

REGISTER_CLASS(GetTagListServlet);
REGISTER_CLASS(DelTagServlet);
REGISTER_CLASS(AddTagServlet);

#endif