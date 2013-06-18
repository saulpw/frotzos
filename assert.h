#ifndef ASSERT_H_
#define ASSERT_H_

extern void __assert_failure(const char *msg);

#define assert(X) do { \
        if (!(X)) {  \
            __assert_failure("assert failed: " #X); \
        } \
    } while (0)

#endif
