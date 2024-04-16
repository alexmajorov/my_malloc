#!/bin/bash

gcc -Wall -Werror -Wpedantic -Wextra -Wfloat-conversion -Wfloat-equal -Wno-unused-result -O2 -c main.c
gcc -o app.exe main.o