#!/usr/bin/bash

gcc \
$(python3.11-config --cflags) \
-o testpython testpython.c