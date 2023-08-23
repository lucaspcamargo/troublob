#!/bin/bash

set -e
cd $(dirname $(realpath ${0}))
PROCESSOR="$(realpath ./str2progmem/str2progmem)"
cd ../src/strings

for i in *.txt ; do
    STR2PROGMEM_TARGET=generic $PROCESSOR $i ${i/.txt/}
done

touch ./*.c

