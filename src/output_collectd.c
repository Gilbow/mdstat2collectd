#include <stdbool.h>
#include <stdio.h> // printf,fprintf
#include <stdlib.h> // getenv
#include <string.h> // strdup
#include <unistd.h> // sleep
#include <time.h> // time

#include "mdstat2collectd.h"

#include "mdstat_reader.h"

#include "utils.h"

/**
 * In seconds !
 * return environment variable : COLLECTD_INTERVAL
 * if not defined, return parameter : interval_param
 * if <= 0, return #defined : DEFAULT_INTERVAL
 */
int get_interval(int interval_param)
{
  int to_return;
  char * collectd_interval = getenv("COLLECTD_INTERVAL");
  if (collectd_interval)
    {
      sscanf(collectd_interval, "%d", &to_return);
    }
  else if (interval_param > 0)
    {
      to_return = interval_param;
    }
  else
    {
      to_return = DEFAULT_INTERVAL;
    }

  return to_return;
}

/**
 * return environment variable : COLLECTD_HOSTNAME
 * if not defined, return parameter : hostname_param
 * if null, return "localhost"
 *
 * must be freed
 */
char * get_hostname(const char * hostname_param)
{
  char * to_return = NULL;
  char * collectd_hostname = getenv("COLLECTD_HOSTNAME");
  if (collectd_hostname)
    {
      to_return = strdup(collectd_hostname);
    }
  else if (hostname_param)
    {
      to_return = strdup(hostname_param);
    }
  else
    {
      to_return = strdup("localhost");
    }

  return to_return;
}

void collectd_write_activity(struct array_data * current_array,
			     const char * hostname,
			     int interval,
			     const char * process_name,
			     const char * activity,
			     time_t now)
{

  bool send_key = (current_array->percent || current_array->speed || current_array->eta); // one value not 0 or 0.0
  
  const char * format_percent_done  = "%s/%s-%s/percent%s";//\" interval=%d %ld:%.2f\n";
  const char * format_current_speed = "%s/%s-%s/bitrate%s";//\" interval=%d %ld:%d\n";
  const char * format_current_eta   = "%s/%s-%s/timeleft%s";//\" interval=%d %ld:%.2f\n";

  const char * format_key_float  = "PUTVAL \"%s\" interval=%d %ld:%.2f\n";
  const char * format_key_int  = "PUTVAL \"%s\" interval=%d %ld:%d\n";

  char * key_name __attribute__((__cleanup__(free_if_not_null))) = malloc(sizeof(char) * (strlen(format_current_eta) + strlen(hostname) + strlen(process_name) + strlen(activity) + 15));
  
  sprintf(key_name, format_percent_done, hostname, process_name, current_array->array_name, activity);
  if (is_collectd_contains_key(&conf->collectd, key_name) == false || send_key)
    {
      printf(format_key_float,
	     key_name,
	     interval,
	     now,
	     current_array->percent);
    }

  sprintf(key_name, format_current_speed, hostname, process_name, current_array->array_name, activity);
  if (is_collectd_contains_key(&conf->collectd, key_name) == false || send_key)
    {
      printf(format_key_int,
	     key_name,  
	     interval,
	     now,
	     current_array->speed);
    }

  sprintf(key_name, format_current_eta, hostname, process_name, current_array->array_name, activity);
  if (is_collectd_contains_key(&conf->collectd, key_name) == false || send_key)
    {
      printf(format_key_float,
	     key_name,
	     interval,
	     now,
	     current_array->eta);
    }
}

void collectd_write(struct array_data * current_array, const char * hostname, int interval)
{
  
  /*
    const char * format_percent_done  = "PUTVAL \"%s/%s-%s/percent-%s\" interval=%d meta:KEY=percent_done %ld:%.2f\n";
    const char * format_current_speed = "PUTVAL \"%s/%s-%s/bitrate-%s\" interval=%d meta:KEY=current_speed %ld:%d\n";
    const char * format_current_eta   = "PUTVAL \"%s/%s-%s/timeleft-%s\" interval=%d meta:KEY=current_eta %ld:%.2f\n";
  */

  time_t now = time(NULL);
  
  const char * process_name = "mdstat";

  char activity[10] = { '\0' };
  if (conf->output_type == ALL)
    {
      // this represents the progress status for all other activity than current one
      // all is nulled, and name is copied
      struct array_data not_the_current_activity_array;
      memset(&not_the_current_activity_array, 0, sizeof(struct array_data));
      memcpy(not_the_current_activity_array.array_name, current_array->array_name, sizeof(not_the_current_activity_array.array_name));
      
      for (enum array_activity i = 0; i <= RECOVER; i++)
	{
	  sprintf(activity, "-%s", array_activity_names[i]);
	  if (i == current_array->activity)
	    { // prints values for the current activity
	      collectd_write_activity(current_array, hostname, interval, process_name, activity, now);
	    }
	  else
	    { // prints 0 values 
	      collectd_write_activity(&not_the_current_activity_array, hostname, interval, process_name, activity, now);
	    }
	}
    }
  else
    {
      collectd_write_activity(current_array, hostname, interval, process_name, activity, now);
    }
}

/**
 *
 *
 */
void collectd_main_loop(const char * hostname_param, int interval_param)
{
  char * hostname __attribute__((__cleanup__(free_if_not_null))) = get_hostname(hostname_param);
  int interval = get_interval(interval_param);

  // source : https://github.com/collectd/collectd/wiki/Plugin-Exec
  /* This needs to be done before *anything* is written to STDOUT! */
  int status = setvbuf (stdout,
			/* buf  = */ NULL,
			/* mode = */ _IONBF, /* unbuffered */
			/* size = */ 0);
  if (status != 0)
    {
      perror ("setvbuf");
      exit (EXIT_FAILURE);
    }

  do
    {
      struct arrays_data data;
      data.array_count = 0;
      data.array = NULL;

      if (process_mdstat(&data))
	{
	  break;
	}

      for (int i = 0; i < data.array_count; i++)
	{
	  collectd_write(data.array[i], hostname, interval);
	  free(data.array[i]);
	}
      
      free(data.array);
   
    }
  while (sleep(interval) == 0);

}
