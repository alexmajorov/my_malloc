#!/bin/bash

gcc -Wall -Werror -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-variable -Wfloat-equal -Wfloat-conversion -O0 -g --coverage -c main.c
gcc -o app.exe --coverage main.o