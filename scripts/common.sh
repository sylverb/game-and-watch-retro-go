#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

function echo_green() {
    echo -e "${GREEN}${@}${NC}"
}

function echo_red() {
    echo -e "${RED}${@}${NC}"
}

if [[ "$VERBOSE" == "1" ]]; then
    set -ex
else
    set -e
fi

if [[ "${GCC_PATH}" != "" ]]; then
    DEFAULT_OBJDUMP=${GCC_PATH}/arm-none-eabi-objdump
    DEFAULT_GDB=${GCC_PATH}/arm-none-eabi-gdb
else
    DEFAULT_OBJDUMP=arm-none-eabi-objdump
    DEFAULT_GDB=arm-none-eabi-gdb
fi

FLASHAPP=scripts/flashapp.sh
FLASH_MULTI=scripts/flash_multi.sh

OBJDUMP=${OBJDUMP:-$DEFAULT_OBJDUMP}
GDB=${GDB:-$DEFAULT_GDB}

ADAPTER=${ADAPTER:-stlink}

RESET_DBGMCU=${RESET_DBGMCU:-1}

function get_symbol {
    name=$1
    objdump_cmd="${OBJDUMP} -t ${ELF}"
    size=$(${objdump_cmd} | grep " $name$" | cut -d " " -f1 | tr 'a-f' 'A-F' | head -n 1)
    printf "$((16#${size}))\n"
}

function get_number_of_saves {
    prefix=$1
    objdump_cmd="${OBJDUMP} -t ${ELF}"
    echo $(${objdump_cmd} | grep " $prefix" | wc -l)
}
