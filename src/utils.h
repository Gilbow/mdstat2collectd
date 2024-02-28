#ifndef __UTILS_H_
#define __UTILS_H_

#include <stdbool.h>
#include <regex.h>

int to_int(const char * buff, const regmatch_t * pmatch);

float to_float(const char * buff, const regmatch_t * pmatch);

long to_long(const char * buff, const regmatch_t * pmatch);

void to_string(const char * buff, const regmatch_t * pmatch, char * dest);

void free_if_not_null(char ** ptr);

void add_collectd_key(struct collectd_keys * collectd, const char * key);

bool is_collectd_contains_key(struct collectd_keys * collectd, const char * key);

void f_log(int log_level, const char * format, ...);

#endif // __UTILS_H_
