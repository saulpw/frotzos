#ifndef FZ_STRING_H_
#define FZ_STRING_H_

#include <stdlib.h> // size_t

extern int strlen(const char *);
extern char *strchr(const char *s, int c);
extern char *strcat(char *dest, const char *src);
extern char *strdup(const char *src);
extern int strncmp(const char *s1, const char *s2, size_t n);
extern char *strcpy(char *dest, const char *src);
extern char *strncpy(char *dest, const char *src, size_t n);
extern void *memmove(void *dest, const void *src, size_t n);
extern void *memset(void *s, int c, size_t n);
extern void *memcpy(void *dest, const void *src, size_t n);
extern int memcmp(const void *s1, const void *s2, size_t n);

int snprintf(char *buf, unsigned int sz, const char *fmt, ...);



#endif
