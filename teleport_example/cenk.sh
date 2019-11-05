#!/bin/sh
simulaqron set network-config-file $(pwd)/cenk_network.json
<<<<<<< HEAD
simulaqron start --keep
python3 aliceTest.py
simulaqron stop
=======
simulaqron get network-config-file 
simulaqron start --nodes Cenkovich --keep 
python3 bobTest.py
simulaqron stop 
>>>>>>> 5d364e5706729d7024c7ffa8e52867285179c50f
