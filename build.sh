#!/bin/sh
cd src
DOCKER=1 make bot
DOCKER=1 make almostclean
