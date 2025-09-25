#!/bin/bash

faster_file_name=/usr/share/fasterac/data/oscillo14_10k.fast
oscillo_label=14
delay=0.1  # delay in second (0.1s -> 10Hz)
n_max=50   # display 50 oscillo max

#  compilation
make

#  display program usage
./oscigraph -h

#  display 50 oscillos at 10Hz  (without error log)
./oscigraph -d $delay -n $n_max $faster_file_name $oscillo_label >& /dev/null


