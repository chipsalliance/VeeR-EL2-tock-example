This document presents the steps to run a Tock application on VeeR EL2 in simulation.

It was tested with Ubuntu 23.04 which provides Verilator 5.006.
When using an older version or a different OS, you may need to install Verilator from source, as explained [in the doc](https://verilator.org/guide/latest/install.html).

# Prerequisites

Install build dependencies and Verilator for simulation:

    apt install curl make build-essential gcc-riscv64-unknown-elf wget unzip libbit-vector-perl python3-pip verilator

The Rust toolchain installer called `rustup` is needed to compile Tock:

    curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh

Update the submodules:

    git submodule update --init --recursive

# Building software

## Tock

In order to compile Tock and convert it to the format used by Verilator, run:

    make -C tock/boards/veer_el2_sim debug -j$(nproc)
    riscv64-unknown-elf-objcopy -O verilog tock/target/riscv32imc-unknown-none-elf/debug/veer_el2_sim.elf kernel.hex

## Application

Build the sample application:

    make -C app TOCK_TARGETS='rv32imc|rv32imc.0x80030080.0x80070000|0x80030080|0x80070000' -j$(nproc)
    riscv64-unknown-elf-objcopy -I binary -O verilog app/build/rv32imc/rv32imc.0x80030080.0x80070000.tbf hello_world.hex

## Output files
As the testbench for Verilator requires a single file with the program (`program.hex`), the kernel and the application need to be combined:

    sed -i 's/@00000000/@80030000/g' hello_world.hex
    cat kernel.hex hello_world.hex > program.hex

Now `program.hex` is ready to be used in simulation.

# Running simulation in Verilator

There's a testbench that can be built and run using these commands:

    cd cores-veer-el2
    export RV_ROOT=$(pwd)
    make -C tools USER_MODE=1 verilator-build
    cp ../program.hex .
    ./tools/obj_dir/Vtb_top

The execution trace will be located in `exec.log`.

# Troubleshooting

When building the Tock application, you may encounter warnings related to downloads from inactive mirrors which make the process much longer (until an alternative mirror is used). You can remove the entry with the problematic mirror using `sed`:

    sed -i '/cs\.virginia\.edu/d' libtock-c/lib/fetch-newlib.sh libtock-c/lib/fetch-libc++.sh
