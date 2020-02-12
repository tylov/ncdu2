#ifndef CVECTOR_H_
#define CVECTOR_H_

#include <malloc.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>


typedef uint32_t cvec_size_t;

#define _cvec_size(cv) ((cvec_size_t *)(cv).data)[-2]
#define _cvec_capacity(cv) ((cvec_size_t *)(cv).data)[-1]

static inline cvec_size_t* _cvec_alloced(void* data) {
    return data ? ((cvec_size_t *) data) - 2 : NULL;
}
static inline cvec_size_t _cvec_safe_size(const void* data) {
    return data ? ((const cvec_size_t *) data)[-2] : 0;
}
static inline cvec_size_t _cvec_safe_capacity(const void* data) {
    return data ? ((const cvec_size_t *) data)[-1] : 0;
}

#define CVector(tn) CVector_t_##tn
#define cvec_init() {NULL}
#define cvec_destr(cv) free(_cvec_alloced((cv).data))
#define cvec_size(cv)       _cvec_safe_size((cv).data)
#define cvec_capacity(cv)   _cvec_safe_capacity((cv).data)
#define cvec_empty(cv)      ((cv).data == NULL)

#define declare_CVector(tn, T) \
typedef T cvec##tn##_value_t; \
typedef struct CVector(tn) { \
    cvec##tn##_value_t* data; \
} CVector(tn); \
 \
 \
static inline CVector(tn) cvec##tn##_init(void) { \
    CVector(tn) cv = cvec_init(); \
    return cv; \
} \
 \
static inline CVector(tn) cvec##tn##_alloc(cvec_size_t len, cvec_size_t cap) { \
    CVector(tn) cv = cvec_init(); \
    assert(cap >= len); \
    if (cap > 0) { \
        cvec_size_t* rep = (cvec_size_t *) malloc(sizeof(cvec_size_t) * 2 + sizeof(cvec##tn##_value_t) * cap); \
        cv.data = (cvec##tn##_value_t *) (rep + 2); \
        rep[0] = len; \
        rep[1] = cap; \
    } \
    return cv; \
} \
 \
static inline CVector(tn) cvec##tn##_init_n(const cvec##tn##_value_t* data, cvec_size_t len) { \
    CVector(tn) cv = cvec##tn##_alloc(len, len); \
    memcpy(cv.data, data, sizeof(cvec##tn##_value_t) * len); \
    return cv; \
} \
 \
static inline CVector(tn) cvec##tn##_init_v(const CVector(tn) cv) { \
    return cvec##tn##_init_n(cv.data, cvec_size(cv)); \
} \
 \
static inline void cvec##tn##_clear(CVector(tn)* cv_ptr) { \
    CVector(tn) cv = cvec_init(); \
    cvec_destr(*cv_ptr); \
    *cv_ptr = cv; \
} \
 \
 \
static inline void cvec##tn##_reserve(CVector(tn)* cv_ptr, cvec_size_t cap) { \
    if (cap > cvec_capacity(*cv_ptr)) { \
        CVector(tn) cv = cvec##tn##_alloc(cvec_size(*cv_ptr), cap); \
        memcpy(cv.data, cv_ptr->data, sizeof(cvec##tn##_value_t) * cvec_size(*cv_ptr)); \
        cvec_destr(*cv_ptr); \
        *cv_ptr = cv; \
    } \
} \
 \
 \
static inline CVector(tn)* cvec##tn##_assign_n(CVector(tn)* cv_ptr, const cvec##tn##_value_t* data, cvec_size_t len) { \
    if (len > cvec_capacity(*cv_ptr) || len * 2 < cvec_capacity(*cv_ptr)) { \
        CVector(tn) cv = cvec##tn##_init_n(data, len); \
        cvec_destr(*cv_ptr); \
        *cv_ptr = cv; \
    } else if (len) { \
        memmove(cv_ptr->data, data, sizeof(cvec##tn##_value_t) * len); \
        _cvec_size(*cv_ptr) = len; \
    } \
    return cv_ptr; \
} \
 \
 \
static inline CVector(tn)* cvec##tn##_assign_v(CVector(tn)* cv_ptr, CVector(tn) cv2) { \
    return cvec##tn##_assign_n(cv_ptr, cv2.data, cvec_size(cv2)); \
} \
 \
 \
static inline CVector(tn)* cvec##tn##_append_n(CVector(tn)* cv_ptr, const cvec##tn##_value_t* data, cvec_size_t len) { \
    cvec_size_t newsize = cvec_size(*cv_ptr) + len; \
    if (newsize > cvec_capacity(*cv_ptr)) { \
        CVector(tn) cv = cvec##tn##_alloc(newsize, (newsize * 5) / 3); \
        memcpy(cv.data, cv_ptr->data, sizeof(cvec##tn##_value_t) * cvec_size(*cv_ptr)); \
        memcpy(&cv.data[cvec_size(*cv_ptr)], data, sizeof(cvec##tn##_value_t) * len); \
        cvec_destr(*cv_ptr); \
        *cv_ptr = cv; \
    } else if (len) { \
        memmove(&cv_ptr->data[cvec_size(*cv_ptr)], data, sizeof(cvec##tn##_value_t) * len); \
        _cvec_size(*cv_ptr) = newsize; \
    } \
    return cv_ptr; \
} \
 \
static inline CVector(tn)* cvec##tn##_push(CVector(tn)* cv_ptr, cvec##tn##_value_t value) { \
    return cvec##tn##_append_n(cv_ptr, &value, 1); \
} \
 \
 \
static inline cvec##tn##_value_t cvec##tn##_back(CVector(tn) cv) { \
    return cv.data[_cvec_size(cv) - 1]; \
} \
 \
 \
static inline cvec##tn##_value_t cvec##tn##_pop(CVector(tn)* cv_ptr) { \
    cvec##tn##_value_t value = cvec##tn##_back(*cv_ptr); \
    --_cvec_size(*cv_ptr); \
    return value; \
}


#endif
