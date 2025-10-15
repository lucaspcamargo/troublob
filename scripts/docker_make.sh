#!/bin/sh
# USE VOL_SUFFIX=":z,shared --userns=keep-id" on podman :)
echo docker run --rm -v $PWD:/m68k${VOL_SUFFIX} --user ${UID} -t registry.gitlab.com/doragasu/docker-sgdk:v1.90 "$@"
docker run --rm -v $PWD:/m68k${VOL_SUFFIX} --user ${UID} -t registry.gitlab.com/doragasu/docker-sgdk:v1.90 "$@"
