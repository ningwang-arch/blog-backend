#ifndef __CATEGORY_SERVLET_H__
#define __CATEGORY_SERVLET_H__

#include "pico/class_factory.h"
#include "pico/http/servlet.h"

class GetCategoryListServlet : public pico::Servlet
{
public:
    void doGet(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) override;
};

class DelCategoryServlet : public pico::Servlet
{
public:
    void doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) override;
};

class AddCategoryServlet : public pico::Servlet
{
public:
    void doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) override;
};

REGISTER_CLASS(GetCategoryListServlet);
REGISTER_CLASS(DelCategoryServlet);
REGISTER_CLASS(AddCategoryServlet);

#endif