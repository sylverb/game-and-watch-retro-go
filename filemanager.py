import argparse
import hashlib
from time import sleep, time

from collections import namedtuple

from pyocd.core.helpers import ConnectHelper
from littlefs import LittleFS, LittleFSError


_EMPTY_HASH_HEXDIGEST = hashlib.sha256(b"").hexdigest()
Variable = namedtuple("Variable", ['address', 'size'])

# fmt: off
# These addresses are fixed via a carefully crafted linker script.
variables = {
    "framebuffer1":            Variable(0x2400_0000, 320 * 240 * 2),
    "framebuffer2":            Variable(0x2402_5800, 320 * 240 * 2),
    "flashapp_state":          Variable(0x2000_0000, 4),
    "program_start":           Variable(0x2000_0004, 4),
    "program_status":          Variable(0x2000_0008, 4),
    "program_size":            Variable(0x2000_000c, 4),
    "program_address":         Variable(0x2000_0010, 4),
    "program_erase":           Variable(0x2000_0014, 4),  # Basically a bool; erase on True; erase start at program_address
    "program_erase_bytes":     Variable(0x2000_0018, 4),  # Number of bytes to erase; 0 for the entire chip.
    "program_chunk_idx":       Variable(0x2000_001c, 4),
    "program_chunk_count":     Variable(0x2000_0020, 4),
    "program_expected_sha256": Variable(0x2000_0024, 65),
    "boot_magic":              Variable(0x2000_0068, 4),
    "log_idx":                 Variable(0x2000_0070, 4),
    "logbuf":                  Variable(0x2000_0074, 4096),
    "lfs_cfg":                 Variable(0x2000_1078, 4),
}

# littlefs config struct elements
variables["lfs_cfg_context"]      = Variable(variables["lfs_cfg"].address + 0,  4)
variables["lfs_cfg_read"]         = Variable(variables["lfs_cfg"].address + 4,  4)
variables["lfs_cfg_prog"]         = Variable(variables["lfs_cfg"].address + 8,  4)
variables["lfs_cfg_erase"]        = Variable(variables["lfs_cfg"].address + 12, 4)
variables["lfs_cfg_sync"]         = Variable(variables["lfs_cfg"].address + 16, 4)
variables["lfs_cfg_read_size"]    = Variable(variables["lfs_cfg"].address + 20, 4)
variables["lfs_cfg_prog_size"]    = Variable(variables["lfs_cfg"].address + 24, 4)
variables["lfs_cfg_block_size"]   = Variable(variables["lfs_cfg"].address + 28, 4)
variables["lfs_cfg_block_count"]  = Variable(variables["lfs_cfg"].address + 32, 4)
# TODO: too lazy to add the other attributes

# alias the transfer buffer
variables["flash_buffer"] = variables["framebuffer2"]

_flashapp_state_enum_to_str = {
    0x00000000: "FLASHAPP_INIT",
    0x00000001: "FLASHAPP_IDLE",
    0x00000002: "FLASHAPP_START",
    0x00000003: "FLASHAPP_CHECK_HASH_RAM_NEXT",
    0x00000004: "FLASHAPP_CHECK_HASH_RAM",
    0x00000005: "FLASHAPP_ERASE_NEXT",
    0x00000006: "FLASHAPP_ERASE",
    0x00000007: "FLASHAPP_PROGRAM_NEXT",
    0x00000008: "FLASHAPP_PROGRAM",
    0x00000009: "FLASHAPP_CHECK_HASH_FLASH_NEXT",
    0x0000000a: "FLASHAPP_CHECK_HASH_FLASH",
    0x0000000b: "FLASHAPP_TEST_NEXT",
    0x0000000c: "FLASHAPP_TEST",
    0x0000000d: "FLASHAPP_FINAL",
    0x0000000e: "FLASHAPP_ERROR",
}
# fmt: on

class TimeoutError(Exception):
    """Some operation timed out."""


def read_int(key, signed=False):
    return int.from_bytes(target.read_memory_block8(*variables[key]), byteorder='little', signed=signed)


def read_flashapp_state() -> str:
    return _flashapp_state_enum_to_str.get(read_int("flashapp_state"), "UNKNOWN")


def disable_debug():
    """Disables the Debug block, reducing battery consumption."""
    target.halt()
    target.write32(0x5C001004, 0x00000000)
    target.resume()

def erase_block(address:int, erase_size):
    """
    address; offset into flash, NOT 0x900XXXXX

    1. Write data to ``flash_buffer``.
    2. Write the size of data to `program_size`
    3.
    """
    target.halt()
    target.write32(variables["program_address"].address, address)
    target.write32(variables["program_erase"].address, 1)       # Perform an erase at `program_address`
    target.write32(variables["program_size"].address, 0)
    target.write32(variables["program_erase_bytes"].address, erase_size)    # an erase of this size

    target.write32(variables["program_chunk_idx"].address, 1)  # TODO; maybe do something better with this

    target.write_memory_block8(variables["program_expected_sha256"].address, _EMPTY_HASH_HEXDIGEST)

    target.write32(variables["program_start"].address, 1)       # move the flashapp state from IDLE to executing.
    target.resume()

def write_block(address:int, erase_size):
    """
    address: 0x900XXXXX

    1. Write data to ``flash_buffer``.
    2. Write the size of data to `program_size`
    3.
    """
    target.halt()
    target.write32(variables["program_erase"].address, 1)
    target.write32(variables["program_size"].address, address)
    target.write32(variables["program_start"].address, 1)      # start programming
    target.resume()

def read_logbuf():
    return bytes(target.read_memory_block8(*variables["logbuf"])[:read_int("log_idx")]).decode()

def start_flashapp():
    target.reset_and_halt()
    target.write32(variables["boot_magic"].address, 0xf1a5f1a5)
    target.write32(variables["flashapp_state"].address, 0)  # Just to make sure it's cleared
    target.write32(variables["program_chunk_idx"].address, 1)
    target.write32(variables["program_chunk_count"].address, 100)
    target.resume()

def wait_for_idle(timeout=10):
    t_deadline = time() + 10
    while read_flashapp_state() != "FLASHAPP_IDLE":
        if time() > t_deadline:
            raise TimeoutError
        sleep(0.25)

def main():
    parser = argparse.ArgumentParser()
    #parser.add_argument("action", choices=["ls"])
    args = parser.parse_args()

    with ConnectHelper.session_with_chosen_probe() as session:
        global target
        board = session.board
        assert board is not None
        target = board.target

        start_flashapp()
        wait_for_idle()

        block_size = read_int("lfs_cfg_block_size")
        block_count = read_int("lfs_cfg_block_count")

        breakpoint()


if __name__ == "__main__":
    main()
