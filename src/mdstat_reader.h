
#ifndef __MDADM_READER_H_
#define __MDADM_READER_H_

#include "mdstat_data.h"

#define BLOCK_BEGIN "      "
#define ARRAY_NAME "md[0-9]+"
#define ARRAY_DEVICE_NAME "sd[a-z][0-9]*\\[[0-9]+\\]"
#define ARRAY_DEVICE_STATUS_LIST "(\\([WJFSR]\\))?"
#define ARRAY_DEVICE_LIST "( "ARRAY_DEVICE_NAME""ARRAY_DEVICE_STATUS_LIST")+"
//#define ARRAY_RAID_TYPE_LIST "(raid0|raid1|raid2|raid3|raid4|raid5|raid6|raid7|raid8|raid9|raid10)"
#define ARRAY_STATUS_LIST "(clear|inactive|suspended|readonly|read-auto|clean|active|write-pending|active-idle|broken)"
#define ARRAY_ACTION_LIST "(idle|frozen|reshape|resync|check|repair|recover)"

#define FIRST_LINE_PATTERN "^Personalities : (\\[(raid[0-9]+|linear|multipath)\\] *)+$"
#define END_LINE_PATTERN "^unused devices: .*$"

//#define BLOCK_START_PATTERN "^"ARRAY_NAME" : "ARRAY_STATUS_LIST" "ARRAY_RAID_TYPE_LIST" "ARRAY_DEVICE_LIST"$"
#define BLOCK_FIRST_LINE "^("ARRAY_NAME") : "ARRAY_STATUS_LIST" raid[0-9]+"ARRAY_DEVICE_LIST"$"
#define BLOCK_STATUS_LINE "^"BLOCK_BEGIN"[0-9]+ blocks .* \\[[0-9]+/[0-9]+\\] \\[[_U]+\\]$"
#define BLOCK_ACTIVITY_LINE "^"BLOCK_BEGIN"\\[[=>\\.]+\\]  "ARRAY_ACTION_LIST" = +([\\.0-9]+)% \\(([0-9]+)/([0-9]+)\\) finish=([\\.0-9]+)min speed=([0-9]+)K/sec$"
#define BLOCK_BITMAP_LINE "^"BLOCK_BEGIN"bitmap: .*$"
#define BLOCK_END_LINE "^ *$"


/**
 *
 *
 *
 */
int process_mdstat(struct arrays_data * data);

#endif // __MDADM_READER_H_
