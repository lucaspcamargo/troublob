#!/bin/bash
# USE VOL_SUFFIX=":z,shared --userns=keep-id" on podman :)

echo "Running CI build"

set -e

./scripts/process_strings.sh
./scripts/process_all_maps.sh

docker run --rm -v $PWD:/m68k${VOL_SUFFIX} --user ${UID} -t registry.gitlab.com/doragasu/docker-sgdk:v1.90 -d clean
docker run --rm -v $PWD:/m68k${VOL_SUFFIX} --user ${UID} -t registry.gitlab.com/doragasu/docker-sgdk:v1.90 -d release
