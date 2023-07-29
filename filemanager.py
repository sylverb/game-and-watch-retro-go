import argparse
import os
import shutil
import hashlib

from collections import namedtuple

from pyocd.core.helpers import ConnectHelper
from littlefs import LittleFS, LittleFSError

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
    "program_erase":           Variable(0x2000_0014, 4),
    "program_erase_bytes":     Variable(0x2000_0018, 4),
    "program_chunk_idx":       Variable(0x2000_001c, 4),
    "program_chunk_count":     Variable(0x2000_0020, 4),
    "program_expected_sha256": Variable(0x2000_0024, 65),
    "boot_magic":              Variable(0x2000_0068, 4),
    "lfs_cfg":                 Variable(0x2000_0070, 4),
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
# fmt: on

def read_int(key, signed=False):
    return int.from_bytes(target.read_memory_block8(*variables[key]), byteorder='little', signed=signed)

def disable_debug():
    """Disables the Debug block, reducing battery consumption."""
    target.halt()
    target.write32(0x5C001004, 0x00000000)
    target.resume()


def start_flashapp():
    target.reset_and_halt()
    target.write32(variables["boot_magic"].address, 0xf1a5f1a5)
    target.write32(variables["program_chunk_idx"].address, 1)
    target.write32(variables["program_chunk_count"].address, 100)
    target.resume()


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

        block_size = read_int("lfs_cfg_block_size")
        block_count = read_int("lfs_cfg_block_count")

        breakpoint()


if __name__ == "__main__":
    main()
