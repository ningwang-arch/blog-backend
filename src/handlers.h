#ifndef __HANDLERS_H__
#define __HANDLERS_H__

#include <memory>
#include <string>

#include "pico/http/http.h"
#include "src/ConnectionPool/CommonConnectionPool.h"
#include "src/oss/oss_client.h"
#include "src/util.h"


class Application;

using request = pico::HttpRequest::Ptr;
using response = pico::HttpResponse::Ptr;

void root(const request& req, response& resp, Application* app);

void login_handler(const request& req, response& resp, Application* app);

void getCategoryList(const request& req, response& resp, Application* app);

void delCategory(const request& req, response& resp, Application* app);

void addCategory(const request& req, response& resp, Application* app);

void getTagList(const request& req, response& resp, Application* app);

void delTag(const request& req, response& resp, Application* app);

void addTag(const request& req, response& resp, Application* app);

void getProjectList(const request& req, response& resp, Application* app);

void delProject(const request& req, response& resp, Application* app);

void addProject(const request& req, response& resp, Application* app);

void getProjectDetail(const request& req, response& resp, Application* app);

void updateProject(const request& req, response& resp, Application* app);

void getArticleListAdmin(const request& req, response& resp, Application* app);

void addArticle(const request& req, response& resp, Application* app);

void getArticleDetail(const request& req, response& resp, Application* app);

void updateArticle(const request& req, response& resp, Application* app);

void delArticle(const request& req, response& resp, Application* app);

void changeComment(const request& req, response& resp, Application* app);

void getMessageList(const request& req, response& resp, Application* app);

void getMessageDetail(const request& req, response& resp, Application* app);

void addReplyMessage(const request& req, response& resp, Application* app);

void delMessage(const request& req, response& resp, Application* app);

void changeThirdComment(const request& req, response& resp, Application* app);

void getArticleList(const request& req, response& resp, Application* app);

void addComment(const request& req, response& resp, Application* app);

void addThirdComment(const request& req, response& resp, Application* app);

void currentUser(const request& req, response& resp, Application* app);

void get_log(const request& req, response& resp);

#endif