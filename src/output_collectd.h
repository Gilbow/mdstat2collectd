#ifndef __OUTPUT_COLLECTD_H_
#define __OUTPUT_COLLECTD_H_

#ifndef DEFAULT_INTERVAL
#define DEFAULT_INTERVAL 60
#endif // DEFAULT_INTERVAL

void collectd_main_loop(const char * hostname_param, int interval_param);

#endif // __OUTPUT_COLLECTD_H_
