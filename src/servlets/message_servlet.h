#ifndef __MESSAGE_SERVLET_H__
#define __MESSAGE_SERVLET_H__

#include "pico/class_factory.h"
#include "pico/http/servlet.h"

class GetMessageListServlet : public pico::Servlet
{
public:
    void doGet(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& resp) override;
};

class GetMessageDetailServlet : public pico::Servlet
{
public:
    void doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& resp) override;
};

class AddReplyMessageServlet : public pico::Servlet
{
public:
    void doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& resp) override;
};

class DelMessageServlet : public pico::Servlet
{
public:
    void doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& resp) override;
};

REGISTER_CLASS(GetMessageListServlet);
REGISTER_CLASS(GetMessageDetailServlet);
REGISTER_CLASS(AddReplyMessageServlet);
REGISTER_CLASS(DelMessageServlet);

#endif