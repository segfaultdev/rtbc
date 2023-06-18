#!/usr/bin/sh

# gcc $(find . -name "*.c") -Iinclude -Ofast -s -o rtbc
gcc $(find . -name "*.c") -Iinclude -Og -g -fsanitize=address,undefined -o rtbc
