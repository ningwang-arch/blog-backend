#ifndef __COMMENT_SERVLET_H__
#define __COMMENT_SERVLET_H__

#include "pico/class_factory.h"
#include "pico/http/servlet.h"

class ChangeCommentServlet : public pico::Servlet
{
public:
    void doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& resp) override;
};

class ChangeThirdCommentServlet : public pico::Servlet
{
public:
    void doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& resp) override;
};

class AddCommentServlet : public pico::Servlet
{
public:
    void doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& resp) override;
};

class AddThirdCommentServlet : public pico::Servlet
{
public:
    void doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& resp) override;
};

REGISTER_CLASS(ChangeCommentServlet);
REGISTER_CLASS(ChangeThirdCommentServlet);
REGISTER_CLASS(AddCommentServlet);
REGISTER_CLASS(AddThirdCommentServlet);

#endif