CREATE database IF NOT EXISTS `blog`;

use blog;

DROP TABLE IF EXISTS `user`;

CREATE TABLE `user` (
    `id` int(11) NOT NULL AUTO_INCREMENT,
    `name` varchar(255) NOT NULL,
    `email` varchar(255) NOT NULL,
    `created_at` datetime NOT NULL,
    `avatar` varchar(255) NOT NULL DEFAULT 'user',
    `role` int DEFAULT 0,
    PRIMARY KEY (`id`)
) ENGINE = InnoDB DEFAULT CHARSET = utf8;

DROP TABLE IF EXISTS `article`;

CREATE TABLE `article` (
    `id` int(11) NOT NULL AUTO_INCREMENT,
    `title` varchar(255) NOT NULL,
    `keyword` varchar(255) NOT NULL COMMENT 'string list, split by ,',
    `author` VARCHAR(64) NOT NULL,
    `description` varchar(255) NOT NULL,
    `content` VARCHAR(16384) NOT NULL,
    `img_url` varchar(255) NOT NULL,
    `tags` varchar(255) NOT NULL COMMENT 'tag-id list, split by ,',
    `category` varchar(255) NOT NULL COMMENT 'category-id list, split by ,',
    `views` int(11) NOT NULL DEFAULT 0,
    `comments` int(11) NOT NULL DEFAULT 0,
    `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
    `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (`id`)
) ENGINE = InnoDB DEFAULT CHARSET = utf8;

DROP TABLE IF EXISTS `tag_category`;

CREATE TABLE `tag_category` (
    `id` int(11) NOT NULL AUTO_INCREMENT,
    `name` varchar(255) NOT NULL,
    `desc` VARCHAR(255) NOT NULL,
    `created_at` datetime NOT NULL,
    `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    `t_c` int(11) NOT NULL,
    PRIMARY KEY (`id`)
) ENGINE = InnoDB DEFAULT CHARSET = utf8;

DROP TABLE IF EXISTS `main_comment`;

CREATE TABLE `main_comment` (
    `id` int(11) NOT NULL AUTO_INCREMENT,
    `article_id` int(11) NOT NULL,
    `user_id` int(11) NOT NULL,
    `content` VARCHAR(16384) NOT NULL,
    `status` int(11) NOT NULL COMMENT '0:unapproved, 1:approved, -1:deleted',
    `is_handle` int(11) NOT NULL DEFAULT 0 COMMENT '0:unhandle, 1:handle',
    `created_at` datetime NOT NULL,
    `updated_at` datetime NOT NULL,
    PRIMARY KEY (`id`)
) ENGINE = InnoDB DEFAULT CHARSET = utf8;

DROP TABLE IF EXISTS `reply_comment`;

CREATE TABLE `reply_comment` (
    `id` int(11) NOT NULL AUTO_INCREMENT,
    `article_id` int(11) NOT NULL,
    `main_comment_id` int(11) NOT NULL,
    `from_user_id` int(11) NOT NULL,
    `to_user_id` int(11) NOT NULL,
    `content` VARCHAR(16384) NOT NULL,
    `created_at` datetime NOT NULL,
    PRIMARY KEY (`id`)
) ENGINE = InnoDB DEFAULT CHARSET = utf8;

DROP TABLE IF EXISTS `project`;

CREATE TABLE `project` (
    `id` int(11) NOT NULL AUTO_INCREMENT,
    `title` VARCHAR(1024) NOT NULL,
    `description` varchar(255) NOT NULL,
    `img` varchar(255) NOT NULL,
    `url` varchar(255) NOT NULL,
    `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
    `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    `end_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
    `status` int(11) NOT NULL DEFAULT 0 COMMENT '1 finished ??? 2 doing ??? 3 to do',
    PRIMARY KEY (`id`)
) ENGINE = InnoDB DEFAULT CHARSET = utf8;