#!/usr/bin/sh

gcc $(find . -name "*.c") -Iinclude -Ofast -s -o rtbc
