#!/bin/sh
simulaqron stop 
simulaqron set network-config-file $(pwd)/network.json
simulaqron start --nodes $(hostname | cut -d . -f 1) --keep 
python3 qkd.py $(hostname | cut -d . -f 1)
simulaqron stop 
