#ifndef CSTRING_H_
#define CSTRING_H_

#include <string.h>
#include <malloc.h>
#include <stdbool.h>


typedef struct CString {
    char* str;
} CString;


static inline CString cstr_init(void) { 
    CString cs = {"\0\1"};
    return cs;
}

static inline CString cstr_init_n(const char* str, size_t len) {
    CString cs = {"\0\1"};
    if (len) {
        cs.str = (char *) malloc(len + 1);
        strncpy(cs.str, str, len);
        cs.str[len] = 0;
    }
    return cs;
}

static inline CString cstr_init_c(const char* str) {
    return cstr_init_n(str, strlen(str));
}

static inline CString cstr_init_s(CString cs) {
    return cstr_init_n(cs.str, strlen(cs.str));
}

static inline void cstr_destr(CString cs) {
    if (cs.str[0] || cs.str[1] != 1)
        free(cs.str);
}

static inline void cstr_clear(CString* cs_ptr) {
    if (cs_ptr->str[0] || cs_ptr->str[1] != 1) {
        free(cs_ptr->str);
        cs_ptr->str = "\0\1";
    }
}

static inline CString* cstr_assign_n(CString* cs_ptr, const char* str, size_t len) {
    char* tmp = cs_ptr->str;
    if (len) {
        cs_ptr->str = (char *) malloc(len + 1);
        strncpy(cs_ptr->str, str, len);
        cs_ptr->str[len] = 0;
    } else
        cs_ptr->str = "\0\1";
    if (tmp[0] || tmp[1] != 1) free(tmp);
    return cs_ptr;
}

static inline CString* cstr_assign(CString* cs_ptr, const char* str) {
    return cstr_assign_n(cs_ptr, str, strlen(str));
}

static inline CString* cstr_append_n(CString* cs_ptr, const char* str, size_t len) {
    if (len) {
        size_t oldlen = strlen(cs_ptr->str);
        size_t newlen = oldlen + len;
        char* tmp = cs_ptr->str;
        cs_ptr->str = (char *) malloc(newlen + 1);
        memcpy(cs_ptr->str, tmp, oldlen);
        strncpy(cs_ptr->str + oldlen, str, len);
        cs_ptr->str[newlen] = 0;
        if (tmp[0] || tmp[1] != 1) free(tmp);
    }
    return cs_ptr;
}

static inline CString* cstr_append(CString* cs_ptr, const char* str) {
    return cstr_append_n(cs_ptr, str, strlen(str));
}

// append without allocate: use after other cstr_xxxx_n() funcs with oversized len.
static inline char* cstr_concat_n(CString* cs_ptr, const char* str, size_t len) {
    return strncat(cs_ptr->str, str, len);
}

static inline char* cstr_concat(CString* cs_ptr, const char* str) {
    return strcat(cs_ptr->str, str);
}

// readonly

static inline size_t cstr_length(CString cs) {
    return strlen(cs.str);
}

static inline bool cstr_empty(CString cs) {
    return cs.str[0] == 0;
}

static inline bool cstr_equals(CString cs1, const char* str) {
    return strcmp(cs1.str, str) == 0;
}
static inline bool cstr_equals_s(CString cs1, CString cs2) {
    return strcmp(cs1.str, cs2.str) == 0;
}

static inline int cstr_compare(CString cs1, CString cs2) {
    return strcmp(cs1.str, cs2.str);
}

static inline char* cstr_find(CString cs, const char* needle) {
    return strstr(cs.str, needle);
}

static inline char* cstr_split_first(const char* delimiters, CString cs) {
	return strtok(cs.str, delimiters);
}

static inline char* cstr_split_next(const char* delimiters) {
	return strtok(NULL, delimiters);
}

#endif
