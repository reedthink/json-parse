#include "leptjson.h"
#include <assert.h> /* assert() */
#include <errno.h>  /*错误代码头文件，定义了通过错误码来回报错误信息的宏*/
#include <math.h>   /* HUGE_VAL */
#include <stdlib.h> /* NULL,malloc(), realloc(), free(), strtod() */
#include <string.h> /*memcpy()*/

#ifndef LEPT_PARSE_STACK_INIT_SIZE
#define LEPT_PARSE_STACK_INIT_SIZE 256
#endif

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

#define PUTC(c, ch)                                         \
    do                                                      \
    {                                                       \
        *(char *)lept_context_push(c, sizeof(char)) = (ch); \
    } while (0)

typedef struct
{
    /* const char*是指向常量的指针,而不是指针本身为常量,可以不被初始化.该指针可以指向常量也可以指向变量,只是从该指针的角度而言,它所指向的是常量。
    通过该指针不能修改它所指向的数据. 是这里使用const cahr*的根本原因*/
    const char *json;
    char *stack; /*缓冲区。*/
    size_t size, top;
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
    v->u.n = strtod(c->json, NULL); /*字符串转换成浮点数*/
    if (errno == ERANGE && (v->u.n == HUGE_VAL || v->u.n == -HUGE_VAL))
        return LEPT_PARSE_NUMBER_TOO_BIG;
    c->json = p;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}
static void *lept_context_push(lept_context *c, size_t size)
{
    void *ret;
    assert(size > 0);
    if (c->top + size >= c->size)
    {
        if (c->size == 0)
            c->size = LEPT_PARSE_STACK_INIT_SIZE;
        while (c->top + size >= c->size)
        {
            c->size += c->size >> 1; /* 1.5倍 */
        }
        c->stack = (char *)realloc(c->stack, c->size);
    }
    ret = c->stack + c->top;
    c->top += size;
    return ret;
}
static void *lept_context_pop(lept_context *c, size_t size)
{
    assert(c->top >= size);
    return c->stack + (c->top -= size); /*返回pop后的顶部地址*/
}
/*对于字符串处理，有几点比较重要，一个是转义字符的处理*/
static int lept_parse_string(lept_context *c, lept_value *v)
{
    size_t head = c->top, len;
    const char *p;
    EXPECT(c, '\"');
    p = c->json;
    for (;;)
    {
        char ch = *p++;
        switch (ch)
        {
        case '\"':
            len = c->top - head;
            lept_set_string(v, (const char *)lept_context_pop(c, len), len);
            c->json = p;
            return LEPT_PARSE_OK;
        case '\0':
            c->top = head;
            return LEPT_PARSE_MISS_QUOTATION_MARK;
        case '\\':
            switch (*p++)
            {
            case '\"':
                PUTC(c, '\"');
                break;
            case '\\':
                PUTC(c, '\\');
                break;
            case '/':
                PUTC(c, '/');
                break;
            case 'b':
                PUTC(c, '\b');
                break;
            case 'f':
                PUTC(c, '\f');
                break;
            case 'n':
                PUTC(c, '\n');
                break;
            case 'r':
                PUTC(c, '\r');
                break;
            case 't':
                PUTC(c, '\t');
                break;
            default:
                c->top = head;
                return LEPT_PARSE_INVALID_STRING_ESCAPE;
            }
            break;
        default:
            if ((unsigned char)ch < 0x20)
            {
                c->top = head;
                return LEPT_PARSE_INVALID_STRING_CHAR;
            }
            PUTC(c, ch);
        }
    }
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
    case '\"':
        return lept_parse_string(c, v);
    default:
        return lept_parse_number(c, v);
    }
}
void lept_set_string(lept_value *v, const char *s, size_t len)
{
    assert(v != NULL && (s != NULL || len == 0)); /*非空指针或零长度字符串都是合法的*/
    lept_free(v);
    v->u.s.s = (char *)malloc(len + 1);
    memcpy(v->u.s.s, s, len);
    v->u.s.s[len] = '\0';
    v->u.s.len = len;
    v->type = LEPT_STRING;
}
void lept_free(lept_value *v)
{
    assert(v != NULL);
    if (v->type == LEPT_STRING)
        free(v->u.s.s);
    v->type = LEPT_NULL;
}

lept_type lept_get_type(const lept_value *v)
{
    assert(v != NULL);
    return v->type;
}

int lept_get_boolean(const lept_value *v)
{
    assert(v != NULL && (v->type == LEPT_TRUE || v->type == LEPT_FALSE));
    return v->type == LEPT_TRUE;
}
void lept_set_boolean(lept_value *v, int b)
{
    lept_free(v);
    v->type = b ? LEPT_TRUE : LEPT_FALSE;
}
double lept_get_number(const lept_value *v)
{
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->u.n;
}
void lept_set_number(lept_value *v, double n)
{
    lept_free(v);
    v->u.n = n;
    v->type = LEPT_NUMBER;
}

const char *lept_get_string(const lept_value *v)
{
    assert(v != NULL && v->type == LEPT_STRING);
    return v->u.s.s;
}
size_t lept_get_string_length(const lept_value *v)
{
    assert(v != NULL && v->type == LEPT_STRING);
    return v->u.s.len;
}

/*功能：启动解析*/
int lept_parse(lept_value *v, const char *json)
{
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    c.stack = NULL;
    c.size = c.top = 0;

    lept_init(v);
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
    assert(c.top == 0);
    /*释放时加入断言确保所有数据都被弹出*/
    free(c.stack);
    return ret;
}