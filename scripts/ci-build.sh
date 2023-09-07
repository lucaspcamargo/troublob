#!/bin/bash

echo "Running CI build"

set -e


rm -rfv ./ci-build/
mkdir ci-build
cd ci-build

python3 -m venv venv
source ./venv/bin/activate
pip install lxml
cd ..
./scripts/process_strings.sh
./scripts/process_all_maps.sh
cd ci-build

docker run --rm -v $PWD/..:/m68k -t registry.gitlab.com/doragasu/docker-sgdk:v1.90

