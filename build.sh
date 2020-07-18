#!/bin/sh
cd src
DOCKER=1 make app
DOCKER=1 make almostclean
