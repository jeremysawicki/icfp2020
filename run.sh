#!/bin/sh
echo ======== cat /proc/meminfo ========
cat /proc/meminfo
echo ======== lscpu ========
lscpu
echo ======== cat /proc/cpuinfo ========
cat /proc/cpuinfo
cd src
exec ./app -v "$@"
