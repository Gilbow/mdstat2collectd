
#ifndef __MDSTAT_DATA_H_
#define __MDSTAT_DATA_H_

enum array_activity {
  IDLE,
  FROZEN,
  RESHAPE,
  RESYNC,
  CHECK,
  REPAIR,
  RECOVER
};

extern const char * array_activity_names[];

enum array_type {
  RAID0,
  RAID1,
  RAID2,
  RAID3,
  RAID4,
  RAID5,
  RAID6,
  RAID7,
  RAID8,
  RAID9,
  RAID10,
  LINEAR
};

struct array_data {
  char array_name[8];
  enum array_type type;
  int device_count;
  int device_used;
  int count_device_down;

  float percent;
  float eta;
  int speed;
  enum array_activity activity;
};

struct arrays_data {
  int array_count;
  struct array_data ** array;
};

#endif // __MDSTAT_DATA_H_
