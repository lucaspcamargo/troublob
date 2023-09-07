#!/bin/sh
echo docker run --rm -v $PWD:/m68k --user ${UID} -t registry.gitlab.com/doragasu/docker-sgdk:v1.90 "$@"
docker run --rm -v $PWD:/m68k --user ${UID} -t registry.gitlab.com/doragasu/docker-sgdk:v1.90 "$@"
