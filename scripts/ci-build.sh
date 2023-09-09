#!/bin/bash

echo "Running CI build"

set -e

./scripts/process_strings.sh
./scripts/process_all_maps.sh

rm -rfv ./ci-build/
mkdir ci-build
cd ci-build

docker run --rm -v $PWD/..:/m68k --user ${UID} -t registry.gitlab.com/doragasu/docker-sgdk:v1.90 debug

mv ../out .
