
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "cstring.h"

#define cnew(T) ((T*) malloc(sizeof(T)))

typedef struct Customer {
    CString firstName;
    CString lastName;
    CString streetAddress;
    int age;
} Customer;


Customer* cust_init(Customer* o) {
    o->firstName = cstr_init();
    o->lastName = cstr_init();
    o->streetAddress = cstr_init();
    o->age = 0;
    return o;
}

Customer* cust_init1(Customer* o, const char* fName, const char* lName, const char* addr, int age) {
    o->firstName = cstr_init_c(fName);
    o->lastName = cstr_init_c(lName);
    o->streetAddress = cstr_init_c(addr);
    o->age = age;
    return o;
}

Customer* cust_destr(Customer* o) {
    cstr_destr(o->firstName);
    cstr_destr(o->lastName);
    cstr_destr(o->streetAddress);
    return o;
}


int main()
{
    Customer* customer1 = cust_init(cnew (Customer));
    Customer* customer2 = cust_init1(cnew (Customer), "Tyge", "Løvset", "Fagertunet 14", 55);
    Customer customer3; cust_init1(&customer3, "Tyge", "Løvset", "Fagertunet 14", 55);
    
    CString text = cstr_init();
    cstr_assign(&text, "Initial value");

    CString hello = cstr_init_c("Hello, there is a new value.");
    cstr_append(&hello, " This is no trouble.");
    cstr_append(&hello, " This is the last sentence.");
    printf("%s\n", hello.str);
    
    cstr_assign(&text, "Hello, there is a new value.");
    cstr_append(&text, " This is no trouble.");
    printf("%s: %d\n", text.str, cstr_size(text));
    
    printf("%s, %s, %s, %d\n\n", customer2->firstName.str, customer2->lastName.str,
                                 customer2->streetAddress.str, customer2->age);
    free(cust_destr(customer1));
    free(cust_destr(customer2));
    cust_destr(&customer3);
    
    const char* p = cstr_find(hello, "This is the");
    if (p) printf("found: %s\n", p);

    CString name = cstr_init_n(hello.str, 12);

    printf("name1: %s\n", name.str);
    cstr_assign(&name, "Great");
    printf("name2: %s %zd\n", name.str, sizeof(size_t));

    int i, n = 100000;
    CString vec = cstr_null;
    cstr_reserve(&vec, n);
    for (i=0; i<n; ++i)
        cstr_push(&vec, 'a' + (rand() % 26));

    p = cstr_find(vec, "xyz");
    if (p) {
        cstr_assign_n(&text, p, 10);
        printf("%s: %d\n", text.str, (int) (p - vec.str));
    }

    cstr_destr(name);
    cstr_destr(hello);
    cstr_destr(text);
    cstr_destr(vec);
 }
