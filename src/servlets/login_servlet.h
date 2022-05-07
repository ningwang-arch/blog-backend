#ifndef __LOGIN_SERVLET_H__
#define __LOGIN_SERVLET_H__

#include "pico/class_factory.h"
#include "pico/http/servlet.h"

class LoginServlet : public pico::Servlet
{
public:
    void doPost(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) override;
};

REGISTER_CLASS(LoginServlet);

#endif
