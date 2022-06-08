#include <iostream>

#include "pico/env.h"
#include "pico/mapper/common/mapper.hpp"
#include "src/tables.hpp"



void test() {
    pico::Mapper<User> u_mapper;
    u_mapper.use("sql_1");

    pico::Base<User> base;
    auto criteria = base.createCriteria();
    criteria->andEqualTo(&User::name, "admin");

    auto results = u_mapper.select(base);
    std::cout << results.size() << std::endl;
    for (auto& result : results) { std::cout << result << std::endl; }
}

/**
SELECT i9.from_user_id AS i9_from_user_id, i9.article_id AS i9_article_id, d5.updated_at AS
d5_updated_at, d3.comments AS d3_comments, d3.author AS d3_author, i9.to_user_id AS i9_to_user_id,
i9.created_at AS i9_created_at, d3.content AS d3_content, d3.tags AS d3_tags, d3.img_url AS
d3_img_url, d5.content AS d5_content, d3.description AS d3_description, d5.status AS d5_status,
d3.views AS d3_views, d3.category AS d3_category, i9.id AS i9_id, d3.keyword AS d3_keyword,
d3.updated_at AS d3_updated_at, d3.created_at AS d3_created_at, d3.id AS d3_id, d5.created_at AS
d5_created_at, d3.title AS d3_title, d5.article_id AS d5_article_id, i9.content AS i9_content, d5.id
AS d5_id, d5.user_id AS d5_user_id, d5.is_handle AS d5_is_handle, i9.main_comment_id AS
i9_main_comment_id FROM article AS d3 LEFT OUTER JOIN reply_comment AS i9 ON d5.id =
i9.main_comment_id LEFT OUTER JOIN main_comment AS d5 ON d3.id = d5.article_id

*/

void test_article() {
    pico::Mapper<Article> a_mapper;
    a_mapper.use("sql_1");

    auto ret = a_mapper.selectAll();
    for (auto& result : ret) {
        auto comments = result.main_comments;
        for (auto comment : comments) { std::cout << comment.article_id << std::endl; }
    }
}

void test_xxx() {
    std::cout << pico::EntityHelper::getProperty(&Article::title) << std::endl;
    std::cout << pico::EntityHelper::getProperty(&Article::id) << std::endl;
    std::cout << pico::EntityHelper::getProperty(&tag_category::created_at) << std::endl;
}


int main(int argc, char* argv[]) {
    pico::EnvManager::getInstance()->init(argc, argv);
    pico::Config::LoadFromConfDir(pico::EnvManager::getInstance()->getConfigPath());
    // test();
    // test_article();
    test_xxx();
    return 0;
}
