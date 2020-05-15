#!/bin/sh

gcc -o mime-type-is-text-plain -Wall -Wextra `pkg-config --cflags --libs gio-2.0` mime-type-is-text-plain.c
