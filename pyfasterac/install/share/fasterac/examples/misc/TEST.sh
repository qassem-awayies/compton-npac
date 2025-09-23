#!/bin/bash

echo "----------------------------------------------------------------------"
echo "make"
echo ""
make
echo ""


echo "----------------------------------------------------------------------"
echo "adc_info"
echo ""
./adc_info /Users/qassem.awayies/Projects/compton-npac/pyfasterac/install/share/fasterac/data/adc_100k.fast
echo ""


echo "----------------------------------------------------------------------"
echo "gamma_spectro"
echo ""
./gamma_spectro /Users/qassem.awayies/Projects/compton-npac/pyfasterac/install/share/fasterac/data/adc_100k.fast 4 5750 6750 500 histo.xy
echo ""


echo "----------------------------------------------------------------------"
echo "spectra plot"
echo ""
gnuplot -e  "plot './histo.xy' using 1:2 w line; pause 3.0"
echo ""



