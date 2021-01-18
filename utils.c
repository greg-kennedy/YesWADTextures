#include "utils.h"

#include <stdlib.h>

/* Open a file.  Tests for failure and prints error messages. */
FILE * u_fopen_int(const char *fname, const char *mode, const char *call_file, unsigned int call_line)
{
	FILE *fp=NULL;
	fp = fopen(fname,mode);
	if(fp == NULL) {
		fprintf(stderr,"Could not open file '%s' with mode '%s'\n", fname, mode);
		perror("\tError was");
		fprintf(stderr,"Called from file %s, line %u\n", call_file, call_line);
		exit(EXIT_FAILURE);
	}
	return fp;
}

/* Seek in a file.  offset is always from start of file. */
void u_fseek_int(FILE *const stream, unsigned int const offset, const char *call_file, unsigned int call_line)
{
	if ( fseek(stream, offset, SEEK_SET) ) {
		fprintf(stderr,"utils.c: failed to fseek to offset %u\n", offset);
		perror("\tError was");
		fprintf(stderr,"Called from file %s, line %u\n", call_file, call_line);
		exit(EXIT_FAILURE);
	}
}

/* Read size bytes from stream into ptr. */
size_t u_fread_int(void *const ptr, const size_t count, FILE *const stream, const char *call_file, unsigned int call_line)
{
	size_t result;
	result = fread(ptr, 1, count, stream);
	if(result != count) {
		fprintf(stderr,"utils.c: failed to fread %zu bytes (got %zu)\n", count, result);
		if (ferror(stream)) {
			perror("\tError was");
		} else if (feof(stream)) {
			fprintf(stderr,"\tError was: end of file\n");
		} else {
			fprintf(stderr,"\tError unknown.\n");
		}
		fprintf(stderr,"Called from file %s, line %u\n", call_file, call_line);
		exit(EXIT_FAILURE);
	}
	return result;
}

/* Write size bytes from stream into ptr. */
size_t u_fwrite_int(void *const ptr, const size_t count, FILE *const stream, const char *call_file, unsigned int call_line)
{
	size_t result;
	result = fwrite(ptr, 1, count, stream);
	if(result != count) {
		fprintf(stderr,"utils.c: failed to fwrite %zu bytes (got %zu)\n", count, result);
		if (ferror(stream)) {
			perror("\tError was");
		} else if (feof(stream)) {
			fprintf(stderr,"\tError was: end of file\n");
		} else {
			fprintf(stderr,"\tError unknown.\n");
		}
		fprintf(stderr,"Called from file %s, line %u\n", call_file, call_line);
		exit(EXIT_FAILURE);
	}
	return result;
}

/* Allocate memory.  Exit if error. */
void * u_malloc_int(const size_t size, const char *call_file, unsigned int call_line)
{
	void *x = malloc(size);
	if (x == NULL) {
		fprintf(stderr,"utils.c: failed to malloc %zu bytes\n",size);
		perror("\tError was");
		fprintf(stderr,"Called from file %s, line %u\n", call_file, call_line);
		exit(EXIT_FAILURE);
	}

	return x;
}

/* Allocate (and clear) memory.  Exit if error. */
void * u_calloc_int(const size_t size, const char *call_file, unsigned int call_line)
{
	void *x = calloc(1, size);
	if (x == NULL) {
		fprintf(stderr,"utils.c: failed to calloc %zu bytes\n", size);
		perror("\tError was");
		fprintf(stderr,"Called from file %s, line %u\n", call_file, call_line);
		exit(EXIT_FAILURE);
	}

	return x;
}

/* Reallocate memory block. */
void * u_realloc_int(void *const x, const size_t size, const char *call_file, unsigned int call_line)
{
	void *tmp = realloc(x, size);

	if (tmp == NULL) {
		fprintf(stderr,"utils.c: failed to realloc %zu bytes\n",size);
		perror("\tError was");
		fprintf(stderr,"Called from file %s, line %u\n", call_file, call_line);
		exit(EXIT_FAILURE);
	}

	return tmp;
}

