#!/bin/bash

if [ -z ${COOL_ENV_VAR} ]; then
    echo "no env"
    exit 0
else
    echo "$STARTED_BY"
    exit 1
fi
