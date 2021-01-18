#ifndef UTILS_H_
#define UTILS_H_

/* A collection of wrapper functions
   some of these also supply default args */

#include <stdio.h>

/* endianness */
#define parseU32(x) (*(x) | (*(x+1) << 8) | (*(x+2) << 16) | (*(x+3) << 24))
#define parseU16(x) (*(x) | (*(x+1) << 8))
#define packU32(x,y) { *(x) = (y & 0xFF); *(x+1) = ((y >> 8) & 0xFF); *(x+2) = ((y >> 16) & 0xFF); *(x+3) = ((y >> 24) & 0xFF); }
#define packU16(x,y) { *(x) = (y & 0xFF); *(x+1) = ((y >> 8) & 0xFF); }

/* file handling */
#define u_fopen(fname,mode) u_fopen_int(fname, mode, __FILE__, __LINE__)
FILE * u_fopen_int(const char *fname, const char *mode, const char *call_file, unsigned int call_line);

#define u_fseek(stream,offset) u_fseek_int(stream, offset, __FILE__, __LINE__)
void u_fseek_int(FILE *stream, unsigned int offset, const char *call_file, unsigned int call_line);

#define u_fread(ptr,count,stream) u_fread_int(ptr, count, stream, __FILE__, __LINE__)
size_t u_fread_int(void *ptr, size_t count, FILE *stream, const char *call_file, unsigned int call_line);

#define u_fwrite(ptr,count,stream) u_fwrite_int(ptr, count, stream, __FILE__, __LINE__)
size_t u_fwrite_int(void *ptr, size_t count, FILE *stream, const char *call_file, unsigned int call_line);

/* memory allocation */
#define u_malloc(size) u_malloc_int(size, __FILE__, __LINE__)
void * u_malloc_int(size_t size, const char *call_file, unsigned int call_line);

#define u_calloc(size) u_calloc_int(size, __FILE__, __LINE__)
void * u_calloc_int(size_t size, const char *call_file, unsigned int call_line);

#define u_realloc(x,size) u_realloc_int(x, size, __FILE__, __LINE__)
void * u_realloc_int(void *x, size_t size, const char *call_file, unsigned int call_line);

#endif

