#!/bin/sh
simulaqron set network-config-file $(pwd)/cenk_network.json
simulaqron start --nodes Egemevo,Cenkovich -f
python3 aliceTest.py & python3 bobTest.py
simulaqron stop
