#include "leptjson.h"
#include <assert.h> /* assert() */
#include <stdlib.h> /* NULL */
#include <errno.h>
#include <math.h>

/*这里的宏功能类似内联函数*/
/*功能：判断字符串的开头是不是ch，是的话就把字符串向后走一位*/
#define EXPECT(c, ch)             \
    do                            \
    {                             \
        assert(*c->json == (ch)); \
        c->json++;                \
    } while (0)

#define ISDIGIT(ch) ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')

typedef struct
{
    const char *json; /* const char*是指向常量的指针,而不是指针本身为常量,可以不被初始化.该指针可以指向常量也可以指向变量,只是从该指针的角度而言,它所指向的是常量，*/
    /*通过该指针不能修改它所指向的数据. 是这里使用const cahr*的根本原因*/
} lept_context;

/* static 可以用作函数和变量的前缀，对于函数来讲，static 的作用仅限于隐藏，可以避免与其他文件的同名函数冲突*/

/* 函数功能：跳过空白的部分*/
static void lept_parse_whitespace(lept_context *c)
{
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_literal(lept_context *c, lept_value *v, const char *literal, lept_type type)

{
    size_t i; /*注意在 C 语言中，数组长度、索引值最好使用 size_t 类型，而不是 int 或 unsigned。*/
    EXPECT(c, literal[0]);
    for (i = 0; literal[i + 1]; i++)
    {
        if (c->json[i] != literal[i + 1])
            return LEPT_PARSE_INVALID_VALUE;
    }
    c->json += i;
    v->type = type;
    return LEPT_PARSE_OK;
}

static int lept_parse_number(lept_context *c, lept_value *v)
{
    const char *p = c->json;
    /* 验证一个字符串表示的数字是否合法*/
    if (*p == '-')
        p++;
    if (*p == '0')
        p++;
    else
    {
        if (!ISDIGIT1TO9(*p))
            return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++)
            ; /*把所有连续的数字都处理掉*/
    }
    if (*p == '.')
    {
        p++;
        if (!ISDIGIT(*p))
            return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++)
            ; /*把所有连续的数字都处理掉*/
    }
    if (*p == 'e' || *p == 'E')
    {
        p++;
        if (*p == '+' || *p == '-')
            p++;
        if (!ISDIGIT(*p))
            return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++)
            ; /*把所有连续的数字都处理掉*/
    }
    errno = 0;

    v->n = strtod(c->json, NULL); /*字符串转换成浮点数*/
    if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL))
        return LEPT_PARSE_NUMBER_TOO_BIG;

    c->json = p;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

/*解析数字*/
double lept_get_number(const lept_value *v)
{
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}

/*功能：解析*/
static int lept_parse_value(lept_context *c, lept_value *v)
{
    switch (*c->json)
    {
    case 't':
        return lept_parse_literal(c, v, "true", LEPT_TRUE);
    case 'f':
        return lept_parse_literal(c, v, "false", LEPT_FALSE);
    case 'n':
        return lept_parse_literal(c, v, "null", LEPT_NULL);
    case '\0':
        return LEPT_PARSE_EXPECT_VALUE;
    default:
        return lept_parse_number(c, v);
    }
}
/*功能：启动解析*/
int lept_parse(lept_value *v, const char *json)
{
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);

    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK)
    {
        lept_parse_whitespace(&c);
        if (*c.json != '\0')
        {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value *v)
{
    assert(v != NULL);
    return v->type;
}