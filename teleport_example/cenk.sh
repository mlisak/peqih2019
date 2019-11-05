#!/bin/sh
simulaqron set network-config-file $(pwd)/cenk_network.json
simulaqron get network-config-file 
simulaqron start --nodes Cenkovich --keep 
python3 bobTest.py
simulaqron stop 
