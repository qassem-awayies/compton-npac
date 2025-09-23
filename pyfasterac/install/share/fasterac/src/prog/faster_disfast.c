
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include <string.h>

#include "fasterac/fasterac.h"
#include "fasterac/farray.h"
#include "fasterac/utils.h"


//--- TYPE & ALIAS CTRL -----------------------------------------------//

struct lt_list {
  int             val;
  struct lt_list* next;
} lt_list;

int lt_list_is_empty (struct lt_list* lst) {
  return (lst == NULL);
}

int lt_list_is_element (struct lt_list* lst, int val) {
  if (lt_list_is_empty (lst)) {
    return 0;
  } else {
    if (lst->val == val) {
      return 1;
    } else {
      return lt_list_is_element (lst->next, val);
    }
  }
}

void lt_list_free (struct lt_list* lst) {
  if (! lt_list_is_empty (lst)) {
    lt_list_free (lst->next);
    free (lst);
  }
}

struct lt_list* lt_list_append (struct lt_list* lst, int val) {
  struct lt_list* l = lst;
  if (! lt_list_is_element (lst, val)) {
    l = malloc (sizeof (struct lt_list));
    l->val = val;
    if (! lt_list_is_empty (lst)) {
      l->next = lst;
    }
  }
  return l;
}

void lt_list_display (struct lt_list* lst) {
  if (! lt_list_is_empty (lst)) {
    printf ("%d ", lst->val);
    lt_list_display (lst->next);
  }
}


//--- COMMAND LINE ARGS -----------------------------------------------//

void display_usage () {
  puts ("");
	puts ("faster_file_display -- display data from Faster files (.fast)");
  puts ("");
  puts ("usage :");
  puts ("  faster_file_display [OPTIONS] filename.fast");
  puts ("");
  puts ("options :");
  puts ("  -t  --type=TYPE            :  displays data of this TYPE  [default: all types],");
  puts ("  -l  --label=LABEL          :  displays data of this LABEL [default: all labels],");
  puts ("  -f  --first=FIRST          :  starts displaying at data num FIRST [default : 0],");
  puts ("  -c  --clock=FIRST_TIME     :  starts displaying at clock FIRST_TIME,");
  puts ("  -n  --nb-data=NB           :  amount of data to display [default: all file],");
  puts ("  -u  --time-unit=UNIT       :  time unit in 'ns', 'us', 'ms' or 's' [default : 'ns'],");
  puts ("  -d  --duration=DURATION    :  displays DURATION time units of the file [default : all file],");
  puts ("  -s  --short-fmt            :  use short data format,");
  puts ("  -r  --relative-to-first    :  time is relative to the first displayed data (except for groups),");
  puts ("  -R  --relative-to-previous :  time is relative to the previous displayed data (except for groups),");
  puts ("  -i  --info                 :  displays a summary of the file composition at the end of the list,");
  puts ("  -I  --info-only            :  like -i, without displaying the data list,");
  puts ("  -v  --version_num          :  displays fasterac version number,");
  puts ("  -V  --version              :  displays fasterac verbose version.");
  puts ("  -h  --help                 :  displays this help.");
  puts ("");
  puts ("about time in groups :");
  puts ("  time displayed inside a group is relative to the group time stamp");
  puts ("  independently of the options 'r' and 'R'.");
  puts ("");
  puts ("examples :");
  puts ("  faster_file_display bob.fast");
  puts ("  - displays on console every data from bob.fast.");
  puts ("");
  puts ("  faster_file_display bob.fast -I");
  puts ("  - displays a summary of bob.fast without displaying any data.");
  puts ("");
  puts ("  faster_file_display bob.fast -r -f 3 -n 5");
  puts ("  - displays 5 data, beginning at the third, with time elapsed from the first displayed data.");
  puts ("");
  puts ("  faster_file_display bob.fast -R");
  puts ("  - shows time's gap between the displayed data.");
  puts ("");
  puts ("  faster_file_display -t44 -t50 -t21 -l1 -l2 bob.fast");
  puts ("  - types and labels multiple selections (conjunction).");
  puts ("");
	exit (EXIT_FAILURE);
}

struct arguments {
  char*              filename;
  struct lt_list*    types;
  struct lt_list*    labels;
  int                short_fmt;
  int                first;
  unsigned long long first_clock_ns;
  int                number;
  time_unit          tunit;
  long double        duration;
  int                info;
  int                info_only;
  int                rel_first;
  int                rel_prev;
  int                vernum;
  int                version;
  unsigned long long z;
};


struct arguments get_args (int argc, char** argv) {
  char                info [256];
  static const char   *optString        = "t:f:c:u:l:d:n:z:iIrRsvVh?";
  static const struct option longOpts[] = {
                                            {"type",                 required_argument, NULL, 't'},
                                            {"label",                required_argument, NULL, 'l'},
                                            {"short_fmt",            required_argument, NULL, 's'},
                                            {"first",                required_argument, NULL, 'f'},
                                            {"first_clock_ns",       required_argument, NULL, 'c'},
                                            {"time-unit",            required_argument, NULL, 'u'},
                                            {"duration",             required_argument, NULL, 'd'},
                                            {"nb-data",              required_argument, NULL, 'n'},
                                            {"info",                 no_argument,       NULL, 'i'},
                                            {"info_only",            no_argument,       NULL, 'I'},
                                            {"relative-to-first",    no_argument,       NULL, 'r'},
                                            {"Relative-to-previous", no_argument,       NULL, 'R'},
                                            {"version_num",          no_argument,       NULL, 'v'},
                                            {"version",              no_argument,       NULL, 'V'},
                                            {"help",                 no_argument,       NULL, 'h'},
                                            {"test",                 required_argument, NULL, 'z'},
                                            { NULL,       no_argument,       NULL,  0 }
                                          };
  struct arguments args;
  int opt        = 0;
  args.filename  = "";
  args.types     = NULL;
  args.labels    = NULL;
  args.number    = INT_MAX;
  args.first     = 0;
  args.first_clock_ns = 0;
  args.tunit     = TUNIT_NS;
  args.duration  = -1.0;
  args.info      = 0;
  args.info_only = 0;
  args.short_fmt = 0;
  args.rel_first = 0;
  args.rel_prev  = 0;
  args.vernum    = 0;
  args.version   = 0;
  args.z         = 0;
	opt = getopt_long (argc, argv, optString, longOpts, NULL);
	while (opt != -1) {
		switch (opt) {
			case 't':
				args.types = lt_list_append (args.types, atoi (optarg));
        break;
			case 'l':
				args.labels = lt_list_append (args.labels, atoi (optarg));
				break;
			case 'n':
				args.number = atoi (optarg);
				break;
			case 's':
				args.short_fmt = 1;
				break;
			case 'u':
             if (strcmp (optarg,  "s") == 0) args.tunit = TUNIT_S;
        else if (strcmp (optarg, "ms") == 0) args.tunit = TUNIT_MS;
        else if (strcmp (optarg, "us") == 0) args.tunit = TUNIT_US;
        else                                 args.tunit = TUNIT_NS;
				break;
			case 'f':
				args.first = atoi (optarg);
				break;
                        case 'c':
                                args.first_clock_ns = atoll (optarg);
                                break;
			case 'd':
        args.duration = strtold (optarg, NULL);
				break;
			case 'i':
				args.info = 1;
				break;
			case 'I':
				args.info      = 1;
				args.info_only = 1;
				break;
			case 'r':
				args.rel_first = 1;
				break;
			case 'R':
				args.rel_prev = 1;
				break;
			case 'v':
				args.vernum = 1;
				break;
			case 'V':
				args.version = 1;
				break;
			case 'z':
				args.z = strtoull (optarg, NULL, 10);
				break;
//			case 1:
//        printf ("\n  BOB !\n\n");
//				break;
			case 'h':
			case '?':
				display_usage ();
				break;
			default:
				break;
		}
		opt = getopt_long (argc, argv, optString, longOpts, NULL);
	}
  if (args.version) {
    fasterac_verbose_version (info);
    printf ("%s\n", info);
    exit (EXIT_SUCCESS);
  }
  if (args.vernum) {
    fasterac_version_number (info);
    printf ("%s\n", info);
    exit (EXIT_SUCCESS);
  }
	args.filename = argv [optind];
  if (args.filename == NULL) display_usage ();
  return args;
}

//-------------------------------------------------------------------------------//

int is_selected (faster_data_p data, struct lt_list* types, struct lt_list* labels) {
  int empty_types  = lt_list_is_empty (types);
  int empty_labels = lt_list_is_empty (labels);
  if (empty_types && empty_labels) return 1;
  if ((!empty_types)  && (lt_list_is_element (types,  faster_data_type_alias (data)))) return 1;
  if ((!empty_labels) && (lt_list_is_element (labels, faster_data_label (data))))      return 1;
  return 0;
}




//--- MAIN PROGRAM ------------------------------------------------------------//

int main (int argc, char** argv) {

  struct arguments       args;
  char*                  data_space;
  size_t                 space_size;
  int                    alloc_error;
  farray*                far;  // faster data array
  faster_data_p          cur_data                = NULL;
  unsigned char          cur_type                = 0;
  unsigned short         cur_label               = 0;
  unsigned long long     first_ns                = 0;
  unsigned long long     last_ns                 = ULLONG_MAX;
  unsigned long long     duration_ns             = 0;
  unsigned long long     first_sel_ns            = 0;
  unsigned long long     previous_ns             = 0;
  int                    i_first                 = 0;
  int                    i_last                  = 0;
  int                    nb_all                  = 0;
  int                    nb_selected             = 0;
  int                    no_type                 = 1;
  int                    no_label                = 1;
  int                    type_cnt  [UCHAR_MAX+1] = {0};
  int                    label_cnt [USHRT_MAX+1] = {0};
  unsigned short         label_typ [USHRT_MAX+1] = {0};
  int                    i;
  int                    n = 0;

  //  command line parse //
  args = get_args (argc, argv);
  no_type  = lt_list_is_empty (args.types);
  no_label = lt_list_is_empty (args.labels);

  //  put data from file to memory and set far
  alloc_error = farray_data_file_to_memory (args.filename, &data_space, &space_size);
  if (alloc_error == 1) {
    printf ("error opening %s\n", args.filename);
    return 1;
  } else if (alloc_error == 2) {
    printf ("memory error\n");
    return 1;
  }
  far = farray_new (data_space, space_size);
  //

  i_first = 0;
  i_last  = far->nb_data - 1;
  if (args.first > 0) {
    i_first = args.first;
  }

  if (args.first_clock_ns > 0) {
    if (args.first > 0) {
      puts("-f and -c are mutually exclusive");
      return 1;
    }
    i_first = farray_nearest_idx (*far, args.first_clock_ns);
    while ( (i_first<i_last) && (faster_data_clock_ns (far->data_array [i_first]) < args.first_clock_ns))
      // nearest is before selected clock : use next event as first
      i_first++;
  }

  if (i_first > i_last) {
    printf ("first data num error\n");
    return 1;
  }
  first_ns = faster_data_clock_ns (far->data_array [i_first]);
  if (args.duration != -1.0) {
    duration_ns = args.duration * tunit_coef [args.tunit];
    last_ns     = first_ns + duration_ns;
    i_last      = farray_previous_idx (*far, last_ns);
  }
//  if (args.number < i_last - i_first + 1) {
//    i_last = i_first + args.number - 1;
//  }
  nb_all      = i_last - i_first + 1;
  last_ns     = faster_data_clock_ns (far->data_array [i_last]);
  duration_ns = last_ns - first_ns;
  previous_ns = 0;

  //  display data list header
  if (!args.info_only) printf ("\n  DATA NUM          TYPE     LABEL          TIME     \n\n");

  for (i=i_first; i<=i_last; i++) {
    cur_data = far->data_array [i];
    if (is_selected (cur_data, args.types, args.labels)) {
      nb_selected = nb_selected + 1;
      if (!args.info_only) {
        if (args.rel_prev) {
          relative_data_display (cur_data, i, 0, !args.short_fmt, previous_ns, args.tunit);
          previous_ns = faster_data_clock_ns (cur_data);
        } else if (args.rel_first) {
          if (first_sel_ns == 0) first_sel_ns = faster_data_clock_ns (cur_data);
          relative_data_display (cur_data, i, 0, !args.short_fmt, first_sel_ns, args.tunit);
        } else {
          relative_data_display (cur_data, i, 0, !args.short_fmt, 0, args.tunit);
        }
      }
    }
    if (args.info) {
      cur_type  = faster_data_type_alias (cur_data);
      cur_label = faster_data_label      (cur_data);
      type_cnt  [cur_type]++;
      label_cnt [cur_label]++;
      label_typ [cur_label] = cur_type;
    }
    if (nb_selected == args.number) {
       i_last      = i;
       nb_all      = i_last - i_first + 1;
       last_ns     = faster_data_clock_ns (far->data_array [i_last]);
       duration_ns = last_ns - first_ns;
       break;
    }
  }

  if (args.info) {
    printf ("\n");
    printf ("\n");
    printf ("DATA FILE\n");
    printf ("\n");
    printf ("  %s\n", args.filename);
    printf ("\n");
    printf ("TIME WINDOW\n");
    printf ("\n");
    if (args.tunit == TUNIT_NS) {
      printf ("  Selected time window : from t1=%Ldns to t2=%Ldns, duration=%Ldns\n", first_ns, last_ns, duration_ns);
    } else {
      printf ("  Selected time window : from t1=%.2Lf%s to t2=%.2Lf%s, duration=%.2Lf%s\n",
                        (long double)first_ns/tunit_coef [args.tunit],    tunit_str [args.tunit],
                        (long double)last_ns/tunit_coef [args.tunit],     tunit_str [args.tunit],
                        (long double)duration_ns/tunit_coef [args.tunit], tunit_str [args.tunit]);
    }
    printf ("\n");
    if (!no_type || !no_label) {
      printf ("TYPE AND LABEL SELECTION\n");
      printf ("\n");
      if (!no_type) {
        printf ("  Selected types  : "); lt_list_display (args.types); printf ("\n");
      }
      if (!no_label) {
        printf ("  Selected labels : "); lt_list_display (args.labels); printf ("\n");
      }
      printf("\n");
    }
    printf ("DATA COUNT IN TIME WINDOW\n");
    printf ("\n");
    printf ("                                count          %%          data/s\n\n");
    if (no_type && no_label) {
      printf ("                     all   %10d %10.3f %15.2lf\n", nb_all, 100.0*nb_all/nb_all, 1e9*nb_all/duration_ns);
    } else {
      printf ("                selected   %10d %10.3f %15.2lf\n", nb_selected, 100.0*nb_selected/nb_all, 1e9*nb_selected/duration_ns);
      printf ("                     all   %10d %10.3f %15.2lf\n", nb_all, 100.0*nb_all/nb_all, 1e9*nb_all/duration_ns);
    }
    printf("\n");
    printf ("COMPOSITION BY TYPES\n");
    printf ("\n");
    printf ("  type num       type name      count          %%          data/s\n\n");
    for (i=0; i<=UCHAR_MAX; i++) {
      if (type_cnt [i] != 0) printf("%10d %15s %10d %10.3f %15.2f\n", i, type_name (i), type_cnt[i], 100.0*type_cnt[i]/nb_all, 1e9*type_cnt[i]/duration_ns);
    }
    printf("\n");
    printf ("COMPOSITION BY LABELS\n");
    printf ("\n");
    printf ("     label            type      count          %%          data/s\n\n");
    for (i=0; i<=USHRT_MAX; i++) {
      if (label_cnt [i] != 0) printf("%10d %15s %10d %10.3f %15.2f\n", i, type_name (label_typ[i]), label_cnt[i], 100.0*label_cnt[i]/nb_all, 1e9*label_cnt[i]/duration_ns);
    }
    printf ("\n");
  }

  // ZZZZZZZZZZZZ
  if (args.z > 0)
  printf ("t=%Ldns  previous_idx=%d  nearest_idx=%d  next_idx=%d\n",
                                                           args.z,
                                                           farray_previous_idx (*far, args.z),
                                                           farray_nearest_idx  (*far, args.z),
                                                           farray_next_idx     (*far, args.z));
  // ZZZZZZZZZZZZ


  // dealloc data space
  farray_free (far);
  free (data_space);
  return EXIT_SUCCESS;
}


