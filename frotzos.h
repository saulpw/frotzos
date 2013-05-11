#ifndef FROTZOS_H_
#define FROTZOS_H_

#define NULL 0
#define printf(args...)
#define fprintf(args...)
#define sprintf(args...)
#define fputs(args...)

#define assert(X) do { if (!(X)) { os_fatal("assert failed: " #X); } \
    } while (0)

int putchar(int c);
typedef signed int time_t;
typedef unsigned int size_t;
extern int strlen(const char *);
extern char *strchr(const char *s, int c);
extern char *strcat(char *dest, const char *src);
extern char *strcpy(char *dest, const char *src);
extern char *strncpy(char *dest, const char *src, size_t n);
extern void *memmove(void *dest, const void *src, size_t n);
extern void *memset(void *s, int c, size_t n);
extern void *memcpy(void *dest, const void *src, size_t n);
extern void *malloc(size_t size);
extern void free(void *ptr);
extern void *realloc(void *ptr, size_t size);

extern int os_random_seed(void);

typedef struct
{
    struct header {
       char status; // 0=empty; 1=writing; 2=finished
       char path[59];
       int length;
    } *hdr;
    char *data;
    long fpos;
} FILE;

extern FILE *fopen(const char *path, const char *mode);
extern int fclose(FILE *fp);
extern int fgetc(FILE *fp);
extern int fputc(int c, FILE *fp);
extern int fseek(FILE *stream, long offset, int whence);

extern int ftell(FILE *fp);
extern size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
extern size_t fwrite(const void *ptr, size_t size, size_t nmemb,
                     FILE *stream);
extern int ferror(FILE *stream);
extern int ungetc(int c, FILE *stream);

#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

#define EOF -1

extern int getchar(void);
extern time_t time(time_t *t);

extern volatile char _TEXTMODE_BUFFER[];

#ifdef DEBUG

#define T(X) do {  \
    _TEXTMODE_BUFFER[158] = #X[0]; \
    _TEXTMODE_BUFFER[159]++; \
} while (0);

#else
#define T(X)

#endif

extern void os_fatal(); // noreturn
extern void yield();    // hlt, block until interrupt
extern void halt();     // hlt forever

extern unsigned long long rdtsc(void);

extern void trace(const char *funcname, const char *filename, int lineno, const char *note);


#define NOTIMPL TRACE(NOTIMPL)
#define TRACE(X) trace(__FUNCTION__, __FILE__, __LINE__, #X)

#endif
