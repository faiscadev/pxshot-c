/*
  Copyright (c) 2009-2017 Dave Gamble and cJSON contributors

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#ifndef cJSON__h
#define cJSON__h

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>

/* cJSON Types: */
#define cJSON_Invalid (0)
#define cJSON_False  (1 << 0)
#define cJSON_True   (1 << 1)
#define cJSON_NULL   (1 << 2)
#define cJSON_Number (1 << 3)
#define cJSON_String (1 << 4)
#define cJSON_Array  (1 << 5)
#define cJSON_Object (1 << 6)
#define cJSON_Raw    (1 << 7) /* raw json */

#define cJSON_IsReference 256
#define cJSON_StringIsConst 512

/* The cJSON structure: */
typedef struct cJSON
{
    struct cJSON *next;
    struct cJSON *prev;
    struct cJSON *child;
    int type;
    char *valuestring;
    int valueint;
    double valuedouble;
    char *string;
} cJSON;

typedef struct cJSON_Hooks
{
      void *(*malloc_fn)(size_t sz);
      void (*free_fn)(void *ptr);
} cJSON_Hooks;

typedef int cJSON_bool;

#define cJSON_IsInvalid(item) ((item) == NULL || ((item)->type & 0xFF) == cJSON_Invalid)
#define cJSON_IsFalse(item) (((item) != NULL) && (((item)->type & 0xFF) == cJSON_False))
#define cJSON_IsTrue(item) (((item) != NULL) && (((item)->type & 0xFF) == cJSON_True))
#define cJSON_IsBool(item) (((item) != NULL) && ((((item)->type & 0xFF) == cJSON_True) || (((item)->type & 0xFF) == cJSON_False)))
#define cJSON_IsNull(item) (((item) != NULL) && (((item)->type & 0xFF) == cJSON_NULL))
#define cJSON_IsNumber(item) (((item) != NULL) && (((item)->type & 0xFF) == cJSON_Number))
#define cJSON_IsString(item) (((item) != NULL) && (((item)->type & 0xFF) == cJSON_String))
#define cJSON_IsArray(item) (((item) != NULL) && (((item)->type & 0xFF) == cJSON_Array))
#define cJSON_IsObject(item) (((item) != NULL) && (((item)->type & 0xFF) == cJSON_Object))
#define cJSON_IsRaw(item) (((item) != NULL) && (((item)->type & 0xFF) == cJSON_Raw))

/* Minimal cJSON implementation inline for header-only builds */
#ifndef CJSON_IMPLEMENTATION
#define CJSON_IMPLEMENTATION

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <float.h>

static void *cJSON_malloc(size_t size) { return malloc(size); }
static void cJSON_free(void *ptr) { free(ptr); }

static cJSON *cJSON_New_Item(void)
{
    cJSON* node = (cJSON*)cJSON_malloc(sizeof(cJSON));
    if (node) memset(node, 0, sizeof(cJSON));
    return node;
}

static unsigned char *cJSON_strdup(const unsigned char *str)
{
    size_t len = strlen((const char*)str) + 1;
    unsigned char *copy = (unsigned char*)cJSON_malloc(len);
    if (copy) memcpy(copy, str, len);
    return copy;
}

void cJSON_Delete(cJSON *item)
{
    cJSON *next = NULL;
    while (item != NULL)
    {
        next = item->next;
        if (!(item->type & cJSON_IsReference) && (item->child != NULL))
            cJSON_Delete(item->child);
        if (!(item->type & cJSON_IsReference) && (item->valuestring != NULL))
            cJSON_free(item->valuestring);
        if (!(item->type & cJSON_StringIsConst) && (item->string != NULL))
            cJSON_free(item->string);
        cJSON_free(item);
        item = next;
    }
}

/* Parse functions */
static const unsigned char *skip_whitespace(const unsigned char *in)
{
    while (in && *in && (*in <= 32)) in++;
    return in;
}

static const unsigned char *parse_string(cJSON *item, const unsigned char *str)
{
    const unsigned char *ptr = str + 1;
    const unsigned char *end_ptr = ptr;
    unsigned char *ptr2 = NULL;
    unsigned char *out = NULL;
    size_t len = 0;

    if (*str != '\"') return NULL;

    while (*end_ptr != '\"' && *end_ptr) {
        if (*end_ptr++ == '\\') end_ptr++;
    }

    len = (size_t)(end_ptr - ptr);
    out = (unsigned char*)cJSON_malloc(len + 1);
    if (!out) return NULL;

    ptr2 = out;
    while (ptr < end_ptr) {
        if (*ptr != '\\') *ptr2++ = *ptr++;
        else {
            ptr++;
            switch (*ptr) {
                case 'b': *ptr2++ = '\b'; break;
                case 'f': *ptr2++ = '\f'; break;
                case 'n': *ptr2++ = '\n'; break;
                case 'r': *ptr2++ = '\r'; break;
                case 't': *ptr2++ = '\t'; break;
                case 'u': ptr += 4; break; /* Skip unicode for simplicity */
                default: *ptr2++ = *ptr; break;
            }
            ptr++;
        }
    }
    *ptr2 = 0;
    item->valuestring = (char*)out;
    item->type = cJSON_String;
    return end_ptr + 1;
}

static const unsigned char *parse_number(cJSON *item, const unsigned char *num)
{
    double n = 0, sign = 1, scale = 0;
    int subscale = 0, signsubscale = 1;

    if (*num == '-') { sign = -1; num++; }
    if (*num == '0') num++;
    if (*num >= '1' && *num <= '9') {
        do { n = (n * 10.0) + (*num++ - '0'); } while (*num >= '0' && *num <= '9');
    }
    if (*num == '.' && num[1] >= '0' && num[1] <= '9') {
        num++;
        do { n = (n * 10.0) + (*num++ - '0'); scale--; } while (*num >= '0' && *num <= '9');
    }
    if (*num == 'e' || *num == 'E') {
        num++;
        if (*num == '+') num++; else if (*num == '-') { signsubscale = -1; num++; }
        while (*num >= '0' && *num <= '9') subscale = (subscale * 10) + (*num++ - '0');
    }

    n = sign * n * pow(10.0, (scale + subscale * signsubscale));
    item->valuedouble = n;
    item->valueint = (int)n;
    item->type = cJSON_Number;
    return num;
}

static const unsigned char *parse_value(cJSON *item, const unsigned char *value);

static const unsigned char *parse_array(cJSON *item, const unsigned char *value)
{
    cJSON *child = NULL;
    if (*value != '[') return NULL;
    item->type = cJSON_Array;
    value = skip_whitespace(value + 1);
    if (*value == ']') return value + 1;

    item->child = child = cJSON_New_Item();
    if (!item->child) return NULL;
    value = skip_whitespace(parse_value(child, skip_whitespace(value)));
    if (!value) return NULL;

    while (*value == ',') {
        cJSON *new_item = cJSON_New_Item();
        if (!new_item) return NULL;
        child->next = new_item;
        new_item->prev = child;
        child = new_item;
        value = skip_whitespace(parse_value(child, skip_whitespace(value + 1)));
        if (!value) return NULL;
    }

    if (*value == ']') return value + 1;
    return NULL;
}

static const unsigned char *parse_object(cJSON *item, const unsigned char *value)
{
    cJSON *child = NULL;
    if (*value != '{') return NULL;
    item->type = cJSON_Object;
    value = skip_whitespace(value + 1);
    if (*value == '}') return value + 1;

    item->child = child = cJSON_New_Item();
    if (!item->child) return NULL;
    value = skip_whitespace(parse_string(child, skip_whitespace(value)));
    if (!value) return NULL;
    child->string = child->valuestring;
    child->valuestring = NULL;
    if (*value != ':') return NULL;
    value = skip_whitespace(parse_value(child, skip_whitespace(value + 1)));
    if (!value) return NULL;

    while (*value == ',') {
        cJSON *new_item = cJSON_New_Item();
        if (!new_item) return NULL;
        child->next = new_item;
        new_item->prev = child;
        child = new_item;
        value = skip_whitespace(parse_string(child, skip_whitespace(value + 1)));
        if (!value) return NULL;
        child->string = child->valuestring;
        child->valuestring = NULL;
        if (*value != ':') return NULL;
        value = skip_whitespace(parse_value(child, skip_whitespace(value + 1)));
        if (!value) return NULL;
    }

    if (*value == '}') return value + 1;
    return NULL;
}

static const unsigned char *parse_value(cJSON *item, const unsigned char *value)
{
    if (!value) return NULL;
    if (!strncmp((const char*)value, "null", 4)) { item->type = cJSON_NULL; return value + 4; }
    if (!strncmp((const char*)value, "false", 5)) { item->type = cJSON_False; return value + 5; }
    if (!strncmp((const char*)value, "true", 4)) { item->type = cJSON_True; item->valueint = 1; return value + 4; }
    if (*value == '\"') return parse_string(item, value);
    if (*value == '-' || (*value >= '0' && *value <= '9')) return parse_number(item, value);
    if (*value == '[') return parse_array(item, value);
    if (*value == '{') return parse_object(item, value);
    return NULL;
}

cJSON *cJSON_Parse(const char *value)
{
    cJSON *c = cJSON_New_Item();
    if (!c) return NULL;
    if (!parse_value(c, skip_whitespace((const unsigned char*)value))) {
        cJSON_Delete(c);
        return NULL;
    }
    return c;
}

cJSON *cJSON_GetObjectItem(const cJSON *object, const char *string)
{
    cJSON *c = object ? object->child : NULL;
    while (c && strcmp(c->string, string)) c = c->next;
    return c;
}

/* Print functions */
static char *print_number(const cJSON *item)
{
    char *str = (char*)cJSON_malloc(64);
    if (!str) return NULL;
    if (item->valuedouble == 0) {
        strcpy(str, "0");
    } else if (fabs(((double)item->valueint) - item->valuedouble) <= DBL_EPSILON && 
               item->valuedouble <= INT_MAX && item->valuedouble >= INT_MIN) {
        sprintf(str, "%d", item->valueint);
    } else {
        sprintf(str, "%g", item->valuedouble);
    }
    return str;
}

static char *print_string(const char *str)
{
    size_t len = strlen(str) + 3;
    char *out = (char*)cJSON_malloc(len);
    if (!out) return NULL;
    sprintf(out, "\"%s\"", str);
    return out;
}

static char *print_value(const cJSON *item);

static char *print_array(const cJSON *item)
{
    cJSON *child = item->child;
    char *out = NULL, *ptr = NULL, *ret = NULL;
    size_t len = 5;

    /* Count length */
    while (child) {
        ret = print_value(child);
        if (!ret) return NULL;
        len += strlen(ret) + 2;
        cJSON_free(ret);
        child = child->next;
    }

    out = (char*)cJSON_malloc(len);
    if (!out) return NULL;
    *out = '[';
    ptr = out + 1;
    child = item->child;
    while (child) {
        ret = print_value(child);
        if (!ret) { cJSON_free(out); return NULL; }
        strcpy(ptr, ret);
        ptr += strlen(ret);
        cJSON_free(ret);
        if (child->next) { *ptr++ = ','; }
        child = child->next;
    }
    *ptr++ = ']';
    *ptr = 0;
    return out;
}

static char *print_object(const cJSON *item)
{
    cJSON *child = item->child;
    char *out = NULL, *ptr = NULL, *ret = NULL, *str = NULL;
    size_t len = 7;

    /* Count length */
    while (child) {
        str = print_string(child->string);
        ret = print_value(child);
        if (!str || !ret) { cJSON_free(str); cJSON_free(ret); return NULL; }
        len += strlen(str) + strlen(ret) + 3;
        cJSON_free(str);
        cJSON_free(ret);
        child = child->next;
    }

    out = (char*)cJSON_malloc(len);
    if (!out) return NULL;
    *out = '{';
    ptr = out + 1;
    child = item->child;
    while (child) {
        str = print_string(child->string);
        ret = print_value(child);
        if (!str || !ret) { cJSON_free(out); cJSON_free(str); cJSON_free(ret); return NULL; }
        strcpy(ptr, str);
        ptr += strlen(str);
        cJSON_free(str);
        *ptr++ = ':';
        strcpy(ptr, ret);
        ptr += strlen(ret);
        cJSON_free(ret);
        if (child->next) { *ptr++ = ','; }
        child = child->next;
    }
    *ptr++ = '}';
    *ptr = 0;
    return out;
}

static char *print_value(const cJSON *item)
{
    char *out = NULL;
    if (!item) return NULL;
    switch ((item->type) & 0xFF) {
        case cJSON_NULL: out = (char*)cJSON_malloc(5); if (out) strcpy(out, "null"); break;
        case cJSON_False: out = (char*)cJSON_malloc(6); if (out) strcpy(out, "false"); break;
        case cJSON_True: out = (char*)cJSON_malloc(5); if (out) strcpy(out, "true"); break;
        case cJSON_Number: out = print_number(item); break;
        case cJSON_String: out = print_string(item->valuestring); break;
        case cJSON_Array: out = print_array(item); break;
        case cJSON_Object: out = print_object(item); break;
    }
    return out;
}

char *cJSON_Print(const cJSON *item) { return print_value(item); }
char *cJSON_PrintUnformatted(const cJSON *item) { return print_value(item); }

/* Create functions */
cJSON *cJSON_CreateObject(void)
{
    cJSON *item = cJSON_New_Item();
    if (item) item->type = cJSON_Object;
    return item;
}

cJSON *cJSON_CreateArray(void)
{
    cJSON *item = cJSON_New_Item();
    if (item) item->type = cJSON_Array;
    return item;
}

cJSON *cJSON_CreateString(const char *string)
{
    cJSON *item = cJSON_New_Item();
    if (item) {
        item->type = cJSON_String;
        item->valuestring = (char*)cJSON_strdup((const unsigned char*)string);
        if (!item->valuestring) { cJSON_Delete(item); return NULL; }
    }
    return item;
}

cJSON *cJSON_CreateNumber(double num)
{
    cJSON *item = cJSON_New_Item();
    if (item) {
        item->type = cJSON_Number;
        item->valuedouble = num;
        item->valueint = (int)num;
    }
    return item;
}

cJSON *cJSON_CreateBool(cJSON_bool boolean)
{
    cJSON *item = cJSON_New_Item();
    if (item) item->type = boolean ? cJSON_True : cJSON_False;
    return item;
}

static void suffix_object(cJSON *prev, cJSON *item)
{
    prev->next = item;
    item->prev = prev;
}

static cJSON_bool add_item_to_object(cJSON *object, const char *string, cJSON *item, int constant)
{
    if (!object || !item || !string) return 0;
    if (constant) {
        item->string = (char*)string;
        item->type |= cJSON_StringIsConst;
    } else {
        item->string = (char*)cJSON_strdup((const unsigned char*)string);
        if (!item->string) return 0;
    }
    if (!object->child) {
        object->child = item;
    } else {
        cJSON *child = object->child;
        while (child->next) child = child->next;
        suffix_object(child, item);
    }
    return 1;
}

cJSON_bool cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item)
{
    return add_item_to_object(object, string, item, 0);
}

cJSON *cJSON_AddStringToObject(cJSON *object, const char *name, const char *string)
{
    cJSON *item = cJSON_CreateString(string);
    if (!cJSON_AddItemToObject(object, name, item)) { cJSON_Delete(item); return NULL; }
    return item;
}

cJSON *cJSON_AddNumberToObject(cJSON *object, const char *name, double number)
{
    cJSON *item = cJSON_CreateNumber(number);
    if (!cJSON_AddItemToObject(object, name, item)) { cJSON_Delete(item); return NULL; }
    return item;
}

cJSON *cJSON_AddBoolToObject(cJSON *object, const char *name, cJSON_bool boolean)
{
    cJSON *item = cJSON_CreateBool(boolean);
    if (!cJSON_AddItemToObject(object, name, item)) { cJSON_Delete(item); return NULL; }
    return item;
}

#endif /* CJSON_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif

#endif /* cJSON__h */
