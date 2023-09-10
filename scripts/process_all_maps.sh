#!/bin/bash

set -e
cd $(dirname $(realpath ${0}))
MP="$(realpath ./map_processor.py)"
cd ../art/map

for i in *.tmx ; do
    [[ $i = __* ]] && continue  # skip if starts with double underscore
    $MP $i __$i
done

touch ../../res/resources.res
touch ../../src/registry.c
