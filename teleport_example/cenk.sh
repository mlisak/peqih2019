#!/bin/sh
simulaqron set network-config-file $(pwd)/cenk_network.json
simulaqron start --keep
python3 aliceTest.py
simulaqron stop
