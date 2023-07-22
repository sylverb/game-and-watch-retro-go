#!/bin/bash
# TODO: Currently the location and size is hardcoded; need to make dynamic
# Just performs a complete dump of the filesystem partition; needs to be read
# using something like littlefs-python
. ./scripts/common.sh

${OPENOCD} -f scripts/interface_${ADAPTER}.cfg \
    -c "init; halt; dump_image fs.bin 0x90300000 1048576; resume; exit;"
