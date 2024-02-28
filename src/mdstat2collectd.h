#ifndef __MDSTAT2COLLECTD_H_
#define __MDSTAT2COLLECTD_H_

#include <stdbool.h>

#ifndef MDSTAT2COLLECTD_VERSION
#define MDSTAT2COLLECTD_VERSION "0.0.1"
#endif // MDSTAT2COLLECTD_VERSION



#ifndef MDSTAT_FILE
#define MDSTAT_FILE "/proc/mdstat"
#endif // MDSTAT_FILE


enum output_key_type {
  SINGLE,  // write only a single triplet (percent/bitrate/timeleft)
  ALL      // write every triplet for all activity types (idle/resync/...)
};

struct collectd_keys {
  int key_count;
  char ** existing_keys;
};

struct config {
  char * mdstat_file_path;
  enum output_key_type output_type; // 
  struct collectd_keys collectd;
  bool signal_recieved;
  int verbose_level;
};
extern struct config * conf;


#endif // __MDSTAT2COLLECTD_H_
