#ifndef JSONPARSE_H__ //include防范
#define JSONPARSE_H__
typedef enum
{
    V_NULL,
    V_FALSE,
    V_TRUE,
    V_NUMBER,
    V_STRING,
    V_ARRAY,
    V_OBJECT
} V_type;//枚举类型
struct V_value
{
    V_type type;
};
enum
{
    V_PARSE_OK = 0,
    V_PARSE_EXPECT_VALUE,//json只含空白
    V_PARSE_ROOT_NOT_VALUE,
    V_PARSE_INVALID_VALUE,
};
//函数功能：未知
int v_parse(V_value *v, const char *json);
//函数功能未知

V_type v_get_type(const V_value *v);

#endif