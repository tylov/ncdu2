#ifndef CSTRING_H_
#define CSTRING_H_

#include <malloc.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>


typedef uint32_t cstr_size_t;
typedef struct CString {
    char* str;
} CString;


static cstr_size_t  _cstr_null_rep[] = {0, 0, 0};
#define             _cstr_rep(cs)      (((cstr_size_t *) (cs).str) - 2)

#define              cstr_null         {((char* ) (_cstr_null_rep + 2))}
#define              cstr_size(cs)     ((cstr_size_t) _cstr_rep(cs)[0])
#define              cstr_capacity(cs) ((cstr_size_t) _cstr_rep(cs)[1])

static inline CString cstr_alloc(cstr_size_t len, cstr_size_t cap) {
    CString cs = cstr_null;
    assert(cap >= len);
    if (cap > 0) {
        cstr_size_t* rep = (cstr_size_t *) malloc(sizeof(cstr_size_t) * 2 + cap + 1);
        cs.str = (char *) (rep + 2);
        cs.str[len] = '\0';
        rep[0] = len;
        rep[1] = cap;
    }
    return cs;
}

static inline void cstr_destr(CString cs) {
    if (cstr_capacity(cs))
        free(_cstr_rep(cs));
}

static inline CString cstr_init() {
    CString cs = cstr_null;
    return cs;
}

static inline CString cstr_init_n(const char* str, cstr_size_t len) {
    CString cs = cstr_alloc(len, len);
    memcpy(cs.str, str, len);
    return cs;
}
static inline CString cstr_init_c(const char* str) {
    return cstr_init_n(str, strlen(str));
}
static inline CString cstr_init_s(CString cs) {
    return cstr_init_n(cs.str, cstr_size(cs));
}


static inline void cstr_clear(CString* cs_ptr) {
    CString cs = cstr_null;
    cstr_destr(*cs_ptr);
    *cs_ptr = cs;
}

static inline void cstr_reserve(CString* cs_ptr, cstr_size_t cap) {
    if (cap > cstr_capacity(*cs_ptr)) {
        CString cs = cstr_alloc(cstr_size(*cs_ptr), cap);
        memcpy(cs.str, cs_ptr->str, cstr_size(*cs_ptr));
        cstr_destr(*cs_ptr);
        *cs_ptr = cs;
    }
}


static inline CString* cstr_assign_n(CString* cs_ptr, const char* str, cstr_size_t len) {
    if (len > cstr_capacity(*cs_ptr) || len * 2 < cstr_capacity(*cs_ptr)) {
        CString cs = cstr_init_n(str, len);
        cstr_destr(*cs_ptr);
        *cs_ptr = cs;
    } else {
        memmove(cs_ptr->str, str, len);
        cs_ptr->str[len] = '\0';
        _cstr_rep(*cs_ptr)[0] = len;
    }
    return cs_ptr;
}

static inline CString* cstr_assign(CString* cs_ptr, const char* str) {
    return cstr_assign_n(cs_ptr, str, strlen(str));
}

static inline CString* cstr_assign_s(CString* cs_ptr, CString cs2) {
    return cstr_assign_n(cs_ptr, cs2.str, cstr_size(cs2));
}


static inline CString* cstr_append_n(CString* cs_ptr, const char* str, cstr_size_t len) {
    cstr_size_t newsize = cstr_size(*cs_ptr) + len;
    if (newsize > cstr_capacity(*cs_ptr))
        cstr_reserve(cs_ptr, (newsize * 5) / 3);
    memmove(&cs_ptr->str[cstr_size(*cs_ptr)], str, len);
    cs_ptr->str[newsize] = '\0';
    _cstr_rep(*cs_ptr)[0] = newsize;
    return cs_ptr;
}

static inline CString* cstr_append(CString* cs_ptr, const char* str) {
    return cstr_append_n(cs_ptr, str, strlen(str));
}
static inline CString* cstr_append_s(CString* cs_ptr, CString cs2) {
    return cstr_append_n(cs_ptr, cs2.str, cstr_size(cs2));
}


static inline char cstr_back(CString cs) {
    return cs.str[cstr_size(cs) - 1];
}

static inline CString* cstr_push(CString* cs_ptr, char value) {
    return cstr_append_n(cs_ptr, &value, 1);
}


static inline void cstr_pop(CString* cs_ptr) {
    --_cstr_rep(*cs_ptr)[0];
}

// readonly

static inline bool cstr_empty(CString cs) {
    return cstr_size(cs) == 0;
}

static inline bool cstr_equals(CString cs1, const char* str) {
    return strcmp(cs1.str, str) == 0;
}
static inline bool cstr_equals_s(CString cs1, CString cs2) {
    return strcmp(cs1.str, cs2.str) == 0;
}

static inline int cstr_compare_s(CString cs1, CString cs2) {
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
