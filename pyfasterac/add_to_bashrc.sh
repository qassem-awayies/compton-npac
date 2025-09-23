#!/bin/bash
# Ask the user
echo "Which shell do you use? (bash/zsh[MacOS def])"
read shell_choice

# Determine config file
if [[ "$shell_choice" == "bash" ]]; then
    config_file="$HOME/.bashrc"
elif [[ "$shell_choice" == "zsh" ]]; then
    config_file="$HOME/.zshrc"
else
    echo "Invalid choice. Please type 'bash' or 'zsh'."
    exit 1
fi

current_dir=$(pwd)
echo "#Add pyfasterac to PYTHONPATH env variable">> $config_file
echo "export PYTHONPATH=$current_dir/install/lib:$PYTHONPATH" >> $config_file
