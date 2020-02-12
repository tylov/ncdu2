#include <stdio.h>
#include "cvector.h"

declare_CVector(Int, int);
declare_CVector(Dbl, double);

struct FileStat { int uid, items, size; };
declare_CVector(Fs, struct FileStat);

int main()
{
    struct FileStat s1 = {101, 233, 1239809},
                    s2 = {102, 3233, 3922809};
    CVector(Int) vec = cvec_init();
    unsigned int i, x;
    double dv;
    int test[] = {1, 2, 3, 4};
    cvecInt_append_n(&vec, test, 4);
    cvecInt_append_n(&vec, test + 2, 2);
    printf("sizeof vec: %ld\n", sizeof(vec));
    for (i = 100; i < 120; ++i)
        cvecInt_push(&vec, i);
    
    x = cvecInt_pop(&vec);
    cvecInt_pop(&vec);
    printf("size: %ud\n", cvec_size(vec));
    for (i = 0; i < cvec_size(vec); ++i)
        printf("%d: %d\n", i,  vec.data[i]);
    printf("x: %d\n", x);
    
    CVector(Dbl) real_vec = cvec_init();
    dv = 1.11;
    cvecDbl_push(&real_vec, dv);
    cvecDbl_push(&real_vec, 3.1415926);
    for (i = 0; i < cvec_size(real_vec); ++i)
        printf("%d: %g\n", i,  real_vec.data[i]);
    
    CVector(Fs) userCounter = cvec_init();
    cvecFs_push(&userCounter, s1);
    cvecFs_push(&userCounter, s2);
    for (i = 0; i < cvec_size(userCounter); ++i)
        printf("userCounter %d: %d %d %d\n", i,
               userCounter.data[i].uid,
               userCounter.data[i].items,
               userCounter.data[i].size);
    
    cvec_destr(vec);
    cvec_destr(real_vec);
    cvec_destr(userCounter);
}
