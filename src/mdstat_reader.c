#include <stdlib.h>
#include <stdio.h> // fopen,fread,fclose,
#include <sys/stat.h> // stat

#include <sys/types.h> // regcomp,regexec
#include <regex.h>  // regcomp,regexec

#include <string.h> //strcmp

#include "mdstat2collectd.h"

#include "mdstat_data.h"

#include "mdstat_reader.h"

#include "utils.h"

const char * array_activity_names[] = {
  "idle",
  "frozen",
  "reshape",
  "resync",
  "check",
  "repair",
  "recover"
};

enum reader_step {
  BEFORE_START,
  READY,
  IN_ARRAY,
  STOP
};


struct reader_state
{
  enum reader_step step;
  struct arrays_data * arrays;
};


void print_reg_error(int error_code, const regex_t * regex)
{
  size_t required = regerror(error_code, regex, NULL, 0);
  char * buff = malloc(sizeof(char) * required);

  size_t filled = regerror(error_code, regex, buff, required);
  if (filled == required)
    {
      f_log(1, "%s\n", buff);
    }
  free(buff);
}

typedef int (*parsed_data_processor)(struct reader_state * data, const char * buff, const regmatch_t * pmatch);
struct process_parsed_data {
  struct reader_state * data;
  parsed_data_processor funct;
};


/**
 * 
 * return 0 on success and match, 1 on success and no-match, and -1 on error
 *
 */
int parse_line(const char * buff, const char * pattern, size_t nmatch, struct process_parsed_data * update_fnc)
{
  int cr;
  
  regex_t regex;
  int reg_code = regcomp(&regex, pattern, REG_EXTENDED);
  if (reg_code)
    {
      print_reg_error(reg_code, &regex);
      cr = -1;
    }
  else
    {
      regmatch_t pmatch[nmatch];
      reg_code = regexec(&regex, buff, nmatch, pmatch, 0);
      if (reg_code == REG_NOMATCH)
	{
	  cr = 1;
	}
      else if (reg_code == 0)
	{
	  if (update_fnc && update_fnc->funct)
	    {
	      cr = update_fnc->funct(update_fnc->data, buff, pmatch);
	      //cr = 0;
	    }
	  else
	    {
	      cr = 0;
	    }	  
	}
      else
	{
	  print_reg_error(reg_code, &regex);
	  cr = -1;
	}
    }

    regfree(&regex);
  
  return cr;
}

int process_activity_data(struct reader_state * reader_state, const char * buff, const regmatch_t * pmatch)
{
  struct array_data * current_array = reader_state->arrays->array[reader_state->arrays->array_count - 1];
  current_array->percent = to_float(buff, &pmatch[2]);
  current_array->eta = to_float(buff, &pmatch[5]);
  current_array->speed = to_int(buff, &pmatch[6]);

  char activity[20];
  to_string(buff, &pmatch[1], activity);
  current_array->activity = 0;
  for (long unsigned int i = 0; i <= sizeof(array_activity_names); i++)
    {
      if (strcmp(activity, array_activity_names[i]) == 0)
	{
	  current_array->activity = i;
	  break;
	}
    }
  if (current_array->activity == (enum array_activity) sizeof(array_activity_names))
    {
      current_array->activity = 0;
      f_log(1, "ERROR: activity '%s' not supported\n", activity);
    }
  
  return 0;
}

int process_new_array(struct reader_state * reader_state, const char * buff, const regmatch_t * pmatch)
{
  struct arrays_data * system_arrays = reader_state->arrays;
  system_arrays->array_count++;
  if (system_arrays->array)
    {
      system_arrays->array = realloc(system_arrays->array, sizeof(struct array_data *) * system_arrays->array_count);
    }
  else
    {
      system_arrays->array = malloc(sizeof(struct array_data *));
    }

  reader_state->arrays->array[reader_state->arrays->array_count - 1] = malloc(sizeof(struct array_data));

  struct array_data * current_array = reader_state->arrays->array[reader_state->arrays->array_count - 1];
  memset(current_array, 0, sizeof(struct array_data));

  to_string(buff, &pmatch[1], current_array->array_name);
  
  return 0;
}


int parse_first_line(const char * buff, struct reader_state * reader_state)
{
  struct process_parsed_data update_fnc;
  update_fnc.data = reader_state;
  update_fnc.funct = NULL;

  return parse_line(buff, FIRST_LINE_PATTERN, 3, &update_fnc);
}

int parse_block_first_line(const char * buff, struct reader_state * reader_state)
{
  struct process_parsed_data update_fnc;
  update_fnc.data = reader_state;
  update_fnc.funct = process_new_array;
  
  return parse_line(buff, BLOCK_FIRST_LINE, 2, &update_fnc);
}

int parse_block_status_line(const char * buff, struct reader_state * reader_state)
{
  struct process_parsed_data update_fnc;
  update_fnc.data = reader_state;
  update_fnc.funct = NULL;

  return parse_line(buff, BLOCK_STATUS_LINE, 2, &update_fnc);
}

int parse_block_activity_line(const char * buff, struct reader_state * reader_state)
{
  struct process_parsed_data update_fnc;
  update_fnc.data = reader_state;
  update_fnc.funct = process_activity_data;

  return parse_line(buff, BLOCK_ACTIVITY_LINE, 7, &update_fnc);
}

int parse_block_bitmap_line(const char * buff, struct reader_state * reader_state)
{
  struct process_parsed_data update_fnc;
  update_fnc.data = reader_state;
  update_fnc.funct = NULL;

  return parse_line(buff, BLOCK_BITMAP_LINE, 2, &update_fnc);
}

int parse_empty_line(const char * buff, struct reader_state * reader_state)
{
  struct process_parsed_data update_fnc;
  update_fnc.data = reader_state;
  update_fnc.funct = NULL;

  return parse_line(buff, BLOCK_END_LINE, 2, &update_fnc);
}

int parse_last_line(const char * buff, struct reader_state * reader_state)
{
  struct process_parsed_data update_fnc;
  update_fnc.data = reader_state;
  update_fnc.funct = NULL;

  return parse_line(buff, END_LINE_PATTERN, 2, &update_fnc);
}

int step_before_start(const char * buff, struct reader_state * reader_state)
{
  f_log(2, "before_start -> ");
  int cr = parse_first_line(buff, reader_state);
  if (cr == 0)
    {
      f_log(2, "ready.");
      reader_state->step = READY;
    }
  else
    {
      f_log(2, "wrong format.");
      cr = 1;
    }
  f_log(2, "\n");
  return cr;
}

int step_ready(const char * buff, struct reader_state * reader_state)
{
  f_log(2, "ready -> ");
  int cr = parse_block_first_line(buff, reader_state);
  if (cr == 0)
    {
      f_log(2, "in_array.");
      reader_state->step = IN_ARRAY;
    }
  else
    {
      cr = parse_last_line(buff, reader_state);
      if (cr == 0)
	{
	  f_log(2, "stop.");
	  reader_state->step = STOP;
	}
      else
	{
	  cr = parse_empty_line(buff, reader_state);
	  if (cr == 0)
	    {
	      f_log(2, "ready. (empty line)");
	    }
	}
    }
  f_log(2, "\n");
  return cr;
}


int step_in_array(const char * buff, struct reader_state * reader_state)
{
  f_log(2, "in_array -> ");
  int cr = parse_block_status_line(buff, reader_state);
  if (cr == 0)
    {
      f_log(2, "in_array (status)");
    }
  else
    {
      cr = parse_block_activity_line(buff, reader_state);
      if (cr == 0)
	{
	  f_log(2, "in_array (activity)");
	}
      else
	{  
	  cr = parse_block_bitmap_line(buff, reader_state);
	  if (cr == 0)
	    {
	      f_log(2, "ready (bitmap)");
	      reader_state->step = READY;
	    }
	  else
	    {
	      cr = parse_empty_line(buff, reader_state);
	      if (cr == 0)
		{
		  f_log(2, "ready. (empty line)");
		  reader_state->step = READY;
		}
	    }
	}
    }
  f_log(2, "\n");
  return cr;
}

/**
 *
 *
 *
 *
 */
int read_line_mdstat(FILE * mdstat, struct reader_state * reader_state)
{
  int cr;
  
  char * line __attribute__ ((__cleanup__(free_if_not_null))) = NULL;
  size_t line_len = 0;

  ssize_t read_size = getline(&line, &line_len, mdstat);
  if (read_size == -1)
    {
      if (reader_state->step == STOP)
	{
	  f_log(1, "parse complete\n");
	  cr = 1;
	}
      else
	{
	  f_log(1, "getline: fail\n");
	  cr = -1;
	}
    }
  else
    {
      line[read_size-1] = '\0';
      f_log(2, "getline: read(%ld): '%s'\n", read_size, line);
      switch (reader_state->step)
	{
	case BEFORE_START:
	  cr = step_before_start(line, reader_state);
	  break;

	case READY:
	  cr = step_ready(line, reader_state);
	  break;
	  
	case IN_ARRAY:
	  cr = step_in_array(line, reader_state);
	  break;

	case STOP: 
	default:
	  cr = 1;
	}
      
    }

  return cr;
}


/**
 * return 0 on success
 * -1 in error
 */
int open_mdstat(struct arrays_data * arrays)
{
  int cr;

  const char * mdstat_file = MDSTAT_FILE;
  if (conf->mdstat_file_path)
    {
      mdstat_file = conf->mdstat_file_path;
    }
  
  FILE * mdstat = fopen(mdstat_file, "r");

  if (mdstat)
    {

      struct reader_state reader_state;
      reader_state.step = BEFORE_START;
      reader_state.arrays = arrays;
  
      f_log(2, "open_mdstat: '%s' opened\n", mdstat_file);
      
      while (read_line_mdstat(mdstat, &reader_state) == 0);
	
      fclose(mdstat);
      f_log(2, "open_mdstat: '%s' closed\n", mdstat_file);
      cr = 0;
    }
  else
    {
      char * fopen_perror = malloc(sizeof(char) * (strlen(mdstat_file) + 10)) ;
      f_log(1, "fopen '%s'", mdstat_file);
      perror(fopen_perror);
      free(fopen_perror);
      cr = -1;
    }
  
  return cr;
}


/**
 *
 *
 *
 *
 */
int process_mdstat(struct arrays_data * data)
{

  if (open_mdstat(data) == -1)
    {
      f_log(1, "process_mdstat: open_mdstat : -1\n");
      return -1;
    }

  return 0;
}
