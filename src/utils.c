#include <stdlib.h> // free, realloc
#include <stdio.h> // snprintf, sscanf
#include <string.h> // strdup
#include <stdarg.h>

#include "mdstat2collectd.h"
#include "utils.h"

int to_int(const char * buff, const regmatch_t * pmatch)
{
  char match[pmatch->rm_eo - pmatch->rm_so +1];
  snprintf(match, pmatch->rm_eo - pmatch->rm_so +1, "%s", &buff[pmatch->rm_so]);
  int value;
  if (sscanf(match, "%d", &value) == 1)
    {
      return value;
    }
  return -1;
}

float to_float(const char * buff, const regmatch_t * pmatch)
{
  char match[pmatch->rm_eo - pmatch->rm_so +1];
  snprintf(match, pmatch->rm_eo - pmatch->rm_so +1, "%s", &buff[pmatch->rm_so]);
  float value;
  if (sscanf(match, "%f", &value) == 1)
    {
      return value;
    }
  return -1.0;
}

long to_long(const char * buff, const regmatch_t * pmatch)
{
  char match[pmatch->rm_eo - pmatch->rm_so +1];
  snprintf(match, pmatch->rm_eo - pmatch->rm_so +1, "%s", &buff[pmatch->rm_so]);
  long value;
  if (sscanf(match, "%ld", &value) == 1)
    {
      return value;
    }
  return -1;
}

void to_string(const char * buff, const regmatch_t * pmatch, char * dest)
{
  snprintf(dest, pmatch->rm_eo - pmatch->rm_so +1, "%s", &buff[pmatch->rm_so]);
}

/**
 * 
 * https://gcc.gnu.org/onlinedocs/gcc/Common-Variable-Attributes.html
 */
void free_if_not_null(char ** ptr)
{
  if (*ptr)
    {
      free(*ptr);
    }
}

void add_collectd_key(struct collectd_keys * collectd, const char * key)
{
  collectd->key_count++;
  if (collectd->key_count == 1)
    {
      collectd->existing_keys = malloc(collectd->key_count * sizeof(char *));
    }
  else
    {
      collectd->existing_keys = realloc(collectd->existing_keys, collectd->key_count * sizeof(char *));
    }
  collectd->existing_keys[collectd->key_count-1] = strdup(key);

  f_log(1, "key added: %s\n", key);
}

bool is_collectd_contains_key(struct collectd_keys * collectd, const char * key)
{
  bool exists = false;
  for (int i = 0; i < collectd->key_count; i++)
    {
      if (strcmp(collectd->existing_keys[i], key) == 0)
	{
	  exists = true;
	  break;
	}
    }

  if (exists == false)
    {
      add_collectd_key(collectd, key);
    }

  return exists;
}

void f_log(int level, const char * format, ...)
{
  if (level <= conf->verbose_level)
    {
      va_list ap;
      va_start(ap, format);
      vfprintf(stderr, format, ap);
      va_end(ap);
    }
}
