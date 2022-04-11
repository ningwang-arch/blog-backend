## User Interface

```
GET /getArticleList
request: keyword=&state=1&tag_id=&category_id=&pageNum=1&pageSize=10

response: {"data": {"list": articleList, "count": 1}, "code": 0,"message":""}

articleList: [
    {
        "_id": 1,
        "image_url": "",
        "title": "",
        "desc": "",
        "meta": {"views": 0, "comments":123},
        "create_time": "2022-01-07 21:39:17"
    }
]
```

```
GET /getTagList
request: keyword=&pageNum=1&pageSize=10
response: {"data": {"list": tagList, "count": 1}, "code": 0, "message": ""}

tagList: [
    {
        "_id": 0,
        "name": "",
        "create_time": 2022-01-07 21:39:17
    }
]
```

```
POST /getArticleDetail
request: {'id': 1, "type": 1}
response: {"data": data, "code": 0,"messsage": "msg"}

data: {
    "toc": ""
    "_id": "17"
    "author": "",
    "category": [""],
    comments: [
        commentItem,
    ],
    "create_time": "2022-01-27 16:24:39",
    "desc": "",
    "content": "",
    "img_url": "",
    "keyword": ["c"],
    "meta": {"views": 0, "comments": 0},
    "tags": ["c"],
    "title": "",
    "update_time": "2022-01-07 21:39:17"
}

commentItem: {
    "_id": "",
    "user": {"avatar":"", "name":"", "type": 1},
    "create_time": "2022-01-07 21:39:17"
    "content": ""
    "other_comments": [
        otherItem
    ]
}

otherItem: {
    "id": ""
    "from": {"user_id": "19", "name": "tc", "type": "0", "avatar": ""},
    "to": {"user_id": "19", "name": "tc", "type": "0", "avatar": ""},
    "content": "",
    "create_time": ""
}
```

```
POST /addComment
request: {'article_id': '17', 'data': {'username': 'user_01', 'email': 'xxx@xxx.com', 'content': 'abc'}}
response: {"data": "", "code": 0, "message": "msg"}
```

```
POST /addThirdComment
request: {
    'article_id': '17',
    'from_user': '{"username":"abc","email":"abc123@qq.com"}',
    'comment_id': '11',
    'to_user': '{"avatar":"","name":"user","type":1}',
    'content': 'comment'
}
response: {"data": "", "code": 0, "message": "msg"}
```

```
GET /getProjectList
request: keyword=&state=&pageNum=1&pageSize=10
response: {"data": {"list": project_list, "count": 1}, "code": 0}

project_list: [
    {
        "title": "",
        "content": "",
        "start_time": "",
        "end_time": "",
        "img": "",
        "url": "",
        "_id": "17",
    }
]
```

## Admin Interface

```
POST /loginAdmin
request: {'email': 'admin', 'password': 'user', 'type': 'account'}
response: {"data": {}, "code": 0}
```

```
GET /getCategoryList
request: keyword=&pageNum=1&pageSize=10
response: {"data": {"list": catagoryList, "count": 1}, "code": 0}

categoryList: [
    {
        "_id": "",
        "name": "",
        "desc": "",
        "create_time": ""
    }
]
```

```
POST /delCategory
request: {'id': 'id_1'}
response: {"data": {}, "code": 0, "message": "success"}
```

```
POST /addCategory
request: {'name': 'a', 'desc': 'desc'}
response: {"data": {}, "code": 0, "message": "success"}
```

```
POST /delTag
request: {'id': 1}
response: {"data": {}, "code": 0, "message": "success"}
```

```
POST /addTag
request: {'name': 'test', 'desc': 'desc'}
response: {"data": {}, "code": 0, "message": "success"}
```

```
POST /addProject
request: {
    'state': '',
    'title': 'test',
    'img': 'test',
    'url': 'test',
    'content': 'test',
    'start_time': '2022-04-09 14:39:47', 'end_time': '2022-04-09T14:39:47'
}
response: {"data": {}, "code": 0, "message": "success"}
```

```
POST /delProject
request: {'id':'17'}
response: {"data": {}, "code": 0, "message": "success"}
```

```
POST /getProjectDetail
request: {'id':'17'}
response: {"data": data, "code": 0, "message": "success"}

data: {
    "_id": "",
    "title": "",
    "content": "",
    "img": "",
    "url": "",
    "start_time": "",
    "end_time": ""
}
```

```
POST /updateProject
request: {'
    id': '17',
    'state': '1',
    'title': 'test',
    'img': '',
    'url': '',
    'content': 'test',
    'start_time': '2022-04-09 14:39:47', 'end_time': '2022-04-09 14:39:47'
}
response: {"data": {}, "code": 0, "message": "success"}
```

```
GET /getArticleListAdmin
request: keyword=&state=&pageNum=1&pageSize=10
response: {"data": {"list": articleList, "count": 1}, "code": 0}

articleList: same as articlelist in /getArticleList in user interface
```

```
POST /updateArticle
request: {
    'id': '17',
    'title': 'test',
    'author': 'biaochenxuying',
    'desc': '',
    'keyword': 'c',
    'content': '',
    'img_url': '',
    'tags': '',
    'category': ''
}
response: {"data": {}, "code": 0, "message": "success"}
```

```
POST /delArticle
request: {"id":""}
response: {"data": {}, "code": 0, "message": "success"}
```

```
POST /changeComment
request: {"id": "", "state": ""}
response: {"data": {}, "code": 0, "message": "success"}
```

```
POST /addArticle
request: {
    'title': '',
    'author': '',
    'desc': '',
    'keyword': '',
    'content': '',
    'img_url': '',
    'tags': '',
    'category': ''
}
response: {"data": {}, "code": 0, "message": "success"}
```

```
GET /getMessageList
request: keyword=&state=&pageNum=1&pageSize=10
response: {"data": {"list": res, "count": 1}, "code": 0, "message": "success"}

res = [
    {
        "avatar":"user",
        "content": "",
        // ReplyComment.id which ReplyComment.comment_id == _id && ReplyComment.to.id == user_id
        "reply_list": [
            {"_id": "123", "content": "abc"}
        ],
        "create_time": "",
        "email": "",
        "name": ""
        "state": 0,
        "update_time": "",
        "user_id": "",
        "_id": "" // MainComment.id
    }
]
```

```
POST /getMessageDetail
request: {'id': '5bdee076bc454f49bba03ab0'}
response: {"data": detail, "code": 0, "message": "success"}

detail: {
    "avatar":"user",
    "content": "",
    // ReplyComment.id which ReplyComment.comment_id == _id && ReplyComment.to.id == user_id
    "reply_list": [
        {"_id": "123", "content": "abc"}
    ],
    "create_time": "",
    "email": "",
    "name": ""
    "state": 0,
    "update_time": "",
    "user_id": "",
    "_id": "" // MainComment.id
}
```

```
POST /addReplyMessage
request: {'id': '5bdee076bc454f49bba03ab0', 'state': '', 'content': 'abc'}
response: {"data": {}, "code": 0, "message": "success"}

// from = current manager, to = replayComment.from / MainComment.user_id
```

```
POST /delMessage
request: {'id': '5bdee076bc454f49bba03ab0'}
response: {"data": {}, "code": 0, "message": "success"}

// search in MainComment & ReplyComment
```

```
POST /changeThirdComment
request: {
    "id": 11,  // comments._id
    "state": -1,
    "_id": 0  // comments._id.other_comments.id
}
response: return {"data": {}, "code": 0, "message": "success"}
```
