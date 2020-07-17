#!/bin/sh
echo ======== cat /proc/meminfo ========
cat /proc/meminfo
echo ======== lscpu ========
lscpu
echo ======== cat /proc/cpuinfo ========
cat /proc/cpuinfo
cd src
DOCKER=1 make app
DOCKER=1 make almostclean
