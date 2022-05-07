#ifndef __PROJECT_SERVLET_H__
#define __PROJECT_SERVLET_H__

#include "pico/class_factory.h"
#include "pico/http/servlet.h"

class GetProjectListServlet : public pico::Servlet
{
public:
    void doGet(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) override;
};

class DelProjectServlet : public pico::Servlet
{
public:
    void doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) override;
};

class AddProjectServlet : public pico::Servlet
{
public:
    void doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) override;
};

class GetProjectDetailServlet : public pico::Servlet
{
public:
    void doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) override;
};

class UpdateProjectServlet : public pico::Servlet
{
public:
    void doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) override;
};

REGISTER_CLASS(GetProjectListServlet);
REGISTER_CLASS(DelProjectServlet);
REGISTER_CLASS(AddProjectServlet);
REGISTER_CLASS(GetProjectDetailServlet);
REGISTER_CLASS(UpdateProjectServlet);

#endif