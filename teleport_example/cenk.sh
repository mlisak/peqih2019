#!/bin/sh
simulaqron set network-config-file $(pwd)/cenk_network.json
simulaqron start --nodes Cenkovich --keep 
python3 bobTest.py
simulaqron stop 
