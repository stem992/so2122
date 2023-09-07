#!/bin/bash
pkill "node"
pkill "user"
ipcrm -a
make clean
make 
ulimit -c unlimited
./master