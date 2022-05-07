#ifndef __ROOT_SERVLET_H__
#define __ROOT_SERVLET_H__

#include "pico/class_factory.h"
#include "pico/http/servlet.h"

class RootServlet : public pico::Servlet
{
public:
    void doGet(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& resp) override;
};

class CurrentUserServlet : public pico::Servlet
{
public:
    void doGet(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& resp) override;
};

class GetLogServlet : public pico::Servlet
{
public:
    void doGet(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& resp) override;
};

REGISTER_CLASS(RootServlet);
REGISTER_CLASS(CurrentUserServlet);
REGISTER_CLASS(GetLogServlet);

#endif