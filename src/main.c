#include <stddef.h>
#include <stdio.h>

#include <signal.h> // sighandler_t

#include <stdlib.h> // getenv, malloc

#include <string.h> // strdup

#include <unistd.h> // getopt
#include <getopt.h> // getopt

#include "mdstat2collectd.h"

#include "output_collectd.h"

  // https://github.com/collectd/collectd/wiki/Plugin-Exec
  // https://github.com/collectd/collectd/wiki/Plain-text-protocol
  // https://github.com/collectd/collectd/blob/main/src/types.db
  // https://www.collectd.org/documentation/manpages/collectd-exec.html
  // https://www.collectd.org/documentation/manpages/collectd-unixsock.html


struct config * conf = NULL;

void termination_handler (int signum __attribute__((unused)))
{
  conf->signal_recieved = true;
}

void set_mdstat_file_path(const char * new_path)
{
  if (conf->mdstat_file_path)
    {
      free(conf->mdstat_file_path);
    }
  conf->mdstat_file_path = strdup(new_path);
}

void output_all_keys()
{
  conf->output_type = ALL;
}
void output_single_key()
{
  conf->output_type = SINGLE;
}
void incr_verbose_level()
{
  conf->verbose_level++;
  if ( conf->verbose_level > 3 )
    {
      conf->verbose_level = 3;
    }
}

void usage(const char * arg0)
{
  fprintf(stderr, "Programme qui fourni à collectd l'avancement de tâches en cours sur des grappes raids (pourcentage, vitesse et temps restant estimé).\n\n");

  fprintf(stderr, "Utilisation\t: %s [options]\n", arg0);
  fprintf(stderr, "Options :\n");
  fprintf(stderr, "  -h, --help\t\tAfficher ce message et quitter.\n");
  fprintf(stderr, "  -f FILE, --file=FILE\tIndique l'emplacement du fichier mdstat à lire, (Défaut: "MDSTAT_FILE").\n");
  fprintf(stderr, "  -a, --activity-keys\tFourni à collectd des clés suffixée par l'action en cours sur chaque grappe raid.\n");
  fprintf(stderr, "  -s, --single-key\tFourni à collectd des clés pour chaque grappe raid en action. (Comportement par défaut)\n");
  fprintf(stderr, "  -v, --verbose\t\tAjuste le niveau de log, peut être indiqué plusieurs fois.\n");
  fprintf(stderr, "  \n");
  
  fprintf(stderr, "Ce programme est construit pour durer.\n"); //x86_64-pc-linux-gnu
  fprintf(stderr, "Signaler les anomalies à <bug-mdstat2collectd@gilbow.org>.\n");

 
}

int parse_opts(int argc, char ** argv){
  static const char * short_options = "hf:acsv";

  static struct option long_options[] = {
    {"help",    required_argument, 0, 'h' },
    {"file",    required_argument, 0, 'f' },
    {"activity-keys", no_argument, 0, 'a' },
    {"single-key",    no_argument, 0, 's' },
    {"verbose",       no_argument, 0, 'v' },
    {0,         0,                 0, 0 }
  };

  int cr = 0;
  
  int c;
  
  while (1)
    {
      int option_index = 0;
  
      c = getopt_long(argc, argv, short_options,
		      long_options, &option_index);
      if (c == -1)
	{
	  break;
	}

      switch (c)
	{
	case 'h':
	  cr = 1; // quit
	  break;
	case 1:
	case 'f':
	  set_mdstat_file_path(optarg);
	  break;
	case 'a':
	  output_all_keys();
	  break;
	case 's':
	  output_single_key();
	  break;
	case 'v':
	  incr_verbose_level();
	  break;
	case '?':
	  cr = -1;
	  break;
	default:
	  break;
	}
    }

  return cr;
}


int check_opts()
{
  return 0;
}

/**
 *
 *
 *
 */
int main(int argc, char ** argv)
{

  conf = malloc(sizeof(struct config));
  memset(conf, 0, sizeof(struct config));
  conf->output_type = SINGLE;

  signal (SIGTERM, termination_handler);
  signal (SIGINT , termination_handler);
  
  switch (parse_opts(argc, argv))
    {
    case -1: 
      fprintf(stderr, "when wrongs options are passed, the program doesn't work.\n");
      __attribute__((fallthrough)); // https://gcc.gnu.org/onlinedocs/gcc/Statement-Attributes.html
    case 1:
      usage(argv[0]);
      break;
    default: // 0
      collectd_main_loop(NULL, 60);
    }

  if (conf->mdstat_file_path)
    {
      free(conf->mdstat_file_path);
    }
  for (int i = 0; i < conf->collectd.key_count; i++)
    {
      free(conf->collectd.existing_keys[i]);
    }
  if (conf->collectd.key_count)
    {
      free(conf->collectd.existing_keys);
    }

  free(conf);
  return 0;
}
