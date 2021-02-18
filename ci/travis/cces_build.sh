#!/bin/bash

set -e

export PLATFORM=aducm3029
cd projects/iio_demo
make -j3
