#!/bin/sh
simulaqron stop
simulaqron reset -f
simulaqron set network-config-file $(pwd)/network.json

name=$(hostname | cut -d . -f 1)
if [ "$name" == "cenkmac" ]; then
  echo "Starting!"
else
  name="bulut"
fi
simulaqron start --nodes "$name" --keep
python3 qkd.py "$name"
simulaqron stop
