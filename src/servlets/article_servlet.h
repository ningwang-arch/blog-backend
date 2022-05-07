#ifndef __ARTICLE_SERVLET_H__
#define __ARTICLE_SERVLET_H__

#include "pico/class_factory.h"
#include "pico/http/servlet.h"

class GetArticleListAdminServlet : public pico::Servlet
{
public:
    void doGet(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) override;
};

class AddArticleServlet : public pico::Servlet
{
public:
    void doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) override;
};

class GetArticleDetailServlet : public pico::Servlet
{
public:
    void doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) override;
};

class UpdateArticleServlet : public pico::Servlet
{
public:
    void doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) override;
};

class DelArticleServlet : public pico::Servlet
{
public:
    void doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) override;
};

class GetArticleListServlet : public pico::Servlet
{
public:
    void doGet(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) override;
};

REGISTER_CLASS(GetArticleListAdminServlet);
REGISTER_CLASS(AddArticleServlet);
REGISTER_CLASS(GetArticleDetailServlet);
REGISTER_CLASS(UpdateArticleServlet);
REGISTER_CLASS(DelArticleServlet);
REGISTER_CLASS(GetArticleListServlet);

#endif