#!/bin/bash

if [ -z ${COOL_ENV_VAR} ]; then
    exit 0
else
    exit 1
fi
