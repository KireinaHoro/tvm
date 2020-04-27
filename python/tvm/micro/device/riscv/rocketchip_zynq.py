# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
"""Compilation and config definitions for Rocket Chip on Zynq platforms"""
from collections import OrderedDict

from .. import create_micro_lib_base, register_device

DEVICE_ID = "riscv.rocketchip_zynq"
TOOLCHAIN_PREFIX = "riscv64-unknown-elf-"

def create_micro_lib(obj_path, src_path, lib_type, options=None):
    """Wrapper over `create_micro_lib_base` to add device-specific options

    Parameters
    ----------
    obj_path : str
        path to generated object file

    src_path : str
        path to source file

    lib_type : micro.LibType
        whether to compile a MicroTVM runtime or operator library

    options : Optional[List[str]]
        additional options to pass to GCC
    """
    if options is None:
        options = []
    options += [
        '-mcmodel=medany',
        '-mabi=lp64',
        '-march=rv64imac',
        '-O2',
        '-ffast-math',
        '-fno-inline',
    ]
    create_micro_lib_base(
        obj_path,
        src_path,
        TOOLCHAIN_PREFIX,
        DEVICE_ID,
        lib_type,
        options=options)


def default_config(base_addr, server_addr, server_port):
    """Generates a default configuration for Spike

    Parameters
    ----------
    base_addr : int
        base address of the simulator (for calculating the memory layout)

    server_addr : str
        address of OpenOCD server to connect to

    server_port : int
        port of OpenOCD server to connect to

    Return
    ------
    config : Dict[str, Any]
        MicroTVM config dict for this device
    """
    res = {
        "device_id": DEVICE_ID,
        "toolchain_prefix": TOOLCHAIN_PREFIX,
        "mem_layout": OrderedDict([
            ("text", {
                "size": 0x8000,
            }),
            ("rodata", {
                "size": 0x8000,
            }),
            ("data", {
                "size": 0x1000,
            }),
            ("bss", {
                "size": 0x1000,
            }),
            ("args", {
                "size": 0x1000,
            }),
            ("heap", {
                "size": 0x2000000,
            }),
            ("workspace", {
                "size": 0x8000,
            }),
            ("stack", {
                "size": 0x1000,
            }),
        ]),
        "word_size": 8,
        "thumb_mode": False,
        "comms_method": "zynq",
        "server_addr": server_addr,
        "server_port": server_port,
    }
    # offset `base_addr` by 0x100000 (1M) for monitor region
    base_addr += 0x100000
    # generate section start addresses from the given `base_addr`
    curr_offset = 0
    mem_layout = res["mem_layout"]
    for region_dict in mem_layout.values():
        region_dict["start"] = base_addr + curr_offset
        curr_offset += region_dict["size"]
    return res


register_device(DEVICE_ID, {
    "create_micro_lib": create_micro_lib,
    "default_config": default_config,
})
