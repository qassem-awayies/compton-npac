#!/bin/bash
echo "#Add pyfasterac to PYTHONPATH env variable" >> ~/.bashrc
echo "echo '--> Append to PYTHONPATH: /vol0/faster/LabWork/00_Tools/pyfasterac/install/lib'" >>  ~/.bashrc
echo "export PYTHONPATH=/vol0/faster/LabWork/00_Tools/pyfasterac/install/lib:$PYTHONPATH" >>  ~/.bashrc
