#ifndef __TABLES_HPP__
#define __TABLES_HPP__

#include <string>
#include <vector>

#include "pico/mapper/common/mapper.hpp"
#include "pico/mapper/common/row_bounds_mapper.hpp"
#include "pico/mapper/entity/entity_column.hpp"
#include "pico/mapper/entity/entity_enum.h"
#include "pico/mapper/entity/entity_wrapper.h"
#include "src/tables.hpp"

/**
 * Tables are the main data structure used to map entities to database tables.
 */

struct main_comment;
struct reply_comment;

struct User
{
    int id;
    std::string name;
    std::string email;
    std::time_t created_at;
    std::string avatar;
    int role;

    User() = default;
    User(int id, const std::string& name, const std::string& email, std::time_t created_at,
         const std::string& avatar, int role)
        : id(id)
        , name(name)
        , email(email)
        , created_at(created_at)
        , avatar(avatar)
        , role(role) {}

    User(const std::string& name, const std::string& email, std::time_t created_at,
         const std::string& avatar, int role)
        : name(name)
        , email(email)
        , created_at(created_at)
        , avatar(avatar)
        , role(role) {}

    friend std::ostream& operator<<(std::ostream& os, const User& user) {
        os << "User(" << user.id << ", " << user.name << ", " << user.email << ", "
           << user.created_at << ", " << user.avatar << ", " << user.role << ")";
        return os;
    }
};

struct Article
{
    int id;
    std::string title;
    std::string keyword;   // string list, separated by comma
    std::string author;
    std::string description;
    std::string content;
    std::string img_url;
    std::string tags;       // tag-id list, separated by comma
    std::string category;   // category-id list, separated by comma
    int views;
    int comments;
    std::time_t created_at;
    std::time_t updated_at;

    std::vector<main_comment> main_comments;


    Article() = default;
    Article(int id, const std::string& title, const std::string& keyword, const std::string& author,
            const std::string& description, const std::string& content, const std::string& img_url,
            const std::string& tags, const std::string& category, int views, int comments,
            std::time_t created_at, std::time_t updated_at)
        : id(id)
        , title(title)
        , keyword(keyword)
        , author(author)
        , description(description)
        , content(content)
        , img_url(img_url)
        , tags(tags)
        , category(category)
        , views(views)
        , comments(comments)
        , created_at(created_at)
        , updated_at(updated_at) {}

    Article(const std::string& title, const std::string& keyword, const std::string& author,
            const std::string& description, const std::string& content, const std::string& img_url,
            const std::string& tags, const std::string& category, int views, int comments,
            std::time_t created_at, std::time_t updated_at)
        : title(title)
        , keyword(keyword)
        , author(author)
        , description(description)
        , content(content)
        , img_url(img_url)
        , tags(tags)
        , category(category)
        , views(views)
        , comments(comments)
        , created_at(created_at)
        , updated_at(updated_at) {}
};



struct tag_category
{
    int id;
    std::string name;
    std::string desc;
    std::time_t created_at = {};
    std::time_t updated_at = {};
    int t_c;

    tag_category() = default;
    tag_category(int id, const std::string& name, const std::string& desc, std::time_t created_at,
                 std::time_t updated_at, int t_c)
        : id(id)
        , name(name)
        , desc(desc)
        , created_at(created_at)
        , updated_at(updated_at)
        , t_c(t_c) {}

    tag_category(const std::string& name, const std::string& desc, std::time_t created_at,
                 std::time_t updated_at, int t_c)
        : name(name)
        , desc(desc)
        , created_at(created_at)
        , updated_at(updated_at)
        , t_c(t_c) {}
};


struct main_comment
{
    int id;
    int article_id;
    int user_id;
    std::string content;
    int status;
    int is_handle;
    std::time_t created_at;
    std::time_t updated_at;

    std::vector<reply_comment> reply_comments;

    main_comment() = default;
    main_comment(int id, int article_id, int user_id, const std::string& content, int status,
                 int is_handle, std::time_t created_at, std::time_t updated_at)
        : id(id)
        , article_id(article_id)
        , user_id(user_id)
        , content(content)
        , status(status)
        , is_handle(is_handle)
        , created_at(created_at)
        , updated_at(updated_at) {}

    main_comment(int article_id, int user_id, const std::string& content, int status, int is_handle,
                 std::time_t created_at, std::time_t updated_at)
        : article_id(article_id)
        , user_id(user_id)
        , content(content)
        , status(status)
        , is_handle(is_handle)
        , created_at(created_at)
        , updated_at(updated_at) {}
};

struct reply_comment
{
    int id;
    int article_id;
    int main_comment_id;
    int from_user_id;
    int to_user_id;
    std::string content;
    std::time_t created_at;

    reply_comment() = default;
    reply_comment(int id, int article_id, int main_comment_id, int from_user_id, int to_user_id,
                  const std::string& content, std::time_t created_at)
        : id(id)
        , article_id(article_id)
        , main_comment_id(main_comment_id)
        , from_user_id(from_user_id)
        , to_user_id(to_user_id)
        , content(content)
        , created_at(created_at) {}

    reply_comment(int article_id, int main_comment_id, int from_user_id, int to_user_id,
                  const std::string& content, std::time_t created_at)
        : article_id(article_id)
        , main_comment_id(main_comment_id)
        , from_user_id(from_user_id)
        , to_user_id(to_user_id)
        , content(content)
        , created_at(created_at) {}
};

struct project
{
    int id;
    std::string title;
    std::string description;
    std::string img;
    std::string url;
    std::time_t created_at;
    std::time_t updated_at;
    std::time_t end_at;
    int status;

    project() = default;
    project(int id, const std::string& title, const std::string& description,
            const std::string& img, const std::string& url, std::time_t created_at,
            std::time_t updated_at, const std::time_t& end_at, int status)
        : id(id)
        , title(title)
        , description(description)
        , img(img)
        , url(url)
        , created_at(created_at)
        , updated_at(updated_at)
        , end_at(end_at)
        , status(status) {}

    project(const std::string& title, const std::string& description, const std::string& img,
            const std::string& url, std::time_t created_at, std::time_t updated_at,
            const std::time_t& end_at, int status)
        : title(title)
        , description(description)
        , img(img)
        , url(url)
        , created_at(created_at)
        , updated_at(updated_at)
        , end_at(end_at)
        , status(status) {}
};



ResultMap(EntityMap(User, "user"), PropertyMap(id, pico::ColumnType::Id), PropertyMap(name),
          PropertyMap(email), PropertyMap(created_at), PropertyMap(avatar), PropertyMap(role));

ResultMap(EntityMap(Article, "article"), PropertyMap(id, pico::ColumnType::Id), PropertyMap(title),
          PropertyMap(keyword), PropertyMap(author), PropertyMap(description), PropertyMap(content),
          PropertyMap(img_url), PropertyMap(tags), PropertyMap(category), PropertyMap(views),
          PropertyMap(comments), PropertyMap(created_at), PropertyMap(updated_at),
          PropertyMap(main_comments, "id", pico::JoinType::OneToMany, &main_comment::article_id));


ResultMap(EntityMap(tag_category, "tag_category"), PropertyMap(id, pico::ColumnType::Id),
          PropertyMap(name), PropertyMap(desc), PropertyMap(created_at), PropertyMap(updated_at),
          PropertyMap(t_c));

ResultMap(EntityMap(main_comment, "main_comment"), PropertyMap(id, pico::ColumnType::Id),
          PropertyMap(article_id), PropertyMap(user_id), PropertyMap(content), PropertyMap(status),
          PropertyMap(is_handle), PropertyMap(created_at), PropertyMap(updated_at),
          PropertyMap(reply_comments, "id", pico::JoinType::OneToMany,
                      &reply_comment::main_comment_id));


ResultMap(EntityMap(reply_comment, "reply_comment"), PropertyMap(id, pico::ColumnType::Id),
          PropertyMap(article_id), PropertyMap(main_comment_id), PropertyMap(from_user_id),
          PropertyMap(to_user_id), PropertyMap(content), PropertyMap(created_at));

ResultMap(EntityMap(project, "project"), PropertyMap(id, pico::ColumnType::Id), PropertyMap(title),
          PropertyMap(description), PropertyMap(img), PropertyMap(url), PropertyMap(created_at),
          PropertyMap(updated_at), PropertyMap(end_at), PropertyMap(status));

#endif
