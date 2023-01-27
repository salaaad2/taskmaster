#!/bin/bash

if [ -z ${COOL_ENV_VAR} ]; then
    echo "$STARTED_BY"
    exit 0
else
    echo "no env"
    exit 1
fi
