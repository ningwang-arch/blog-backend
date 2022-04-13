#### Manager

```sql
id ------ int # auto increament; primary key
username ------ varchar(255)
passwd ------ varchar(255)
avatar ------ varchar(255) # default 'user'
type ------ 0
```

#### User

```sql
id ------ int # auto increament; primary key
username ------ varchar(255)
email ------ varchar(255)
avatar ------ varchar(255) # default 'user'
type ------ 1
```

#### Article

```sql
id ------ int # auto increament; primary key
title ------ varchar(255)
keyword ------ varchar(255) #string list, split by ','
author ------ varchar(64)
desc ----- varchar(255)
content ------ varchar(255) # oss-url
img_url ------ varchar(255)
tags ------ varchar(64) # tag-id list, split by ','
category ------ varchar(64) #category-id list, split by ','

views ------ int # default 0
comments ------ int # default 0
create_time ------ datetime # 2022-01-23 21:36:20
update_time ------ datetime # 2022-01-23 21:36:20
```

#### Tag&Category

```sql
id ------ int # auto increament primary key
name ------ varchar(255)
description ------ varchar(255)
create_time ------ datetime # 2022-01-23 21:36:20
update_time ------ datetime # 2022-01-23 21:36:20
type ------ int # 0 - tag  1 - category
```

#### MainComment

```sql
id ------ int # auto inc; primary
article_id ------ int # Article.id
content ------ MEDIUMTEXT
user_id ------ int # user.id/ manager.id
state ------ int # default 1; 0 待审核 / 1 通过正常 / -1 已删除
is_handle ------ int # default 2; 1 是 / 2 否
create_time ------ datetime # 2022-01-23 21:36:20
update_time ------ datetime # 2022-01-23 21:36:20
```

#### ReplyComment

```sql
id ------ int # auto inc; primary
articel_id ------ int # Article.id
comment_id ------ int # MainComment.id
from ------ int # user.id
to ------ int # user_id
content ------ MEDIUMTEXT
create_time ------ datetime # 2022-01-23 21:36:20
```

# Project

```sql
id ------ int # auto inc; primary
title ------ TEXT
content ------ TEXT
img ------ varchar(512)
url ------ varchar(512)
state ------ int # default 1; 1 是已经完成 ，2 是正在进行，3 是没完成

start_time ------ datetime # 2022-01-23 21:36:20
end_time ------ datetime # 2022-01-23 21:36:20
update_time ------ datetime # 2022-01-23 21:36:20
```
