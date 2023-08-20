#!/bin/bash

set -e

cd $(dirname $(realpath ${0}))
cd ../art/map

for i in *.tmx ; do
    echo $i
done
