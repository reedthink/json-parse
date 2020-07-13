#ifndef LEPTJSON_H__
#define LEPTJSON_H__

//枚举类型
typedef enum
{
    LEPT_NULL,   //默认空
    LEPT_FALSE,  //布尔值，错
    LEPT_TRUE,   //布尔值，对
    LEPT_NUMBER, //数字
    LEPT_STRING, //字符串
    LEPT_ARRAY,  //数组
    LEPT_OBJECT  //对象
} lept_type;

typedef struct
{
    lept_type type;
} lept_value;

enum
{
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE,
    LEPT_PARSE_INVALID_VALUE,
    LEPT_PARSE_ROOT_NOT_SINGULAR
};

int lept_parse(lept_value *v, const char *json);

lept_type lept_get_type(const lept_value *v);

#endif /* LEPTJSON_H__ */
