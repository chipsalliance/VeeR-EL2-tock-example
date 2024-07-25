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

First increase the maximum number of cycles in simulation:

    sed -i 's/parameter MAX_CYCLES = 2_000_000;/parameter MAX_CYCLES = 10_000_000;/g' cores-veer-el2/testbench/tb_top.sv

There's a testbench that can be built and run using these commands:

    cd cores-veer-el2
    export RV_ROOT=$(pwd)
    make -C tools USER_MODE=1 verilator-build
    cp ../program.hex .
    ./tools/obj_dir/Vtb_top

The output should look like this:

```
VerilatorTB: Start of sim

mem_signature_begin = 00000000
mem_signature_end   = 00000000
mem_mailbox         = D0580000
VeeR EL2 initialisation complete.
Entering main loop.
Trying to read protected memory contents...

---| No debug queue found. You can set it with the DebugQueue component.

panicked at kernel/src/process_standard.rs:374:17:
Process app had a fault
        Kernel version 356f37ac7

---| RISC-V Machine State |---
Last cause (mcause): Load access fault (interrupt=0, exception code=0x00000005)
Last value (mtval):  0x00030000

System register dump:
 mepc:    0x8000B854    mstatus:     0x00000088
 mcycle:  0x0038E7AB    minstret:    0x00139DDF
 mtvec:   0x80000100
 mstatus: 0x00000088
  uie:    false  upie:   false
  sie:    false  spie:   false
  mie:    true   mpie:   true
  spp:    false
 mie:   0x00000888   mip:   0x00000000
  usoft:  false               false
  ssoft:  false               false
  msoft:  true                false
  utimer: false               false
  stimer: false               false
  mtimer: true                false
  uext:   false               false
  sext:   false               false
  mext:   true                false

---| App Status |---
: app   -   [Faulted]
 Events Queued: 0   Syscall Count: 7   Dropped Upcall Count: 0
 Restart Count: 0
 Last Syscall: Yield { which: 1, param_a: 0, param_b: 0 }
 Completion Code: None



   Address   Region Name    Used | Allocated (bytes)
 0x800715C4
              Grant Ptrs       16
              Upcalls         320
              Process         592
  0x80071224
               Grant          76
  0x800711D8
              Unused
  0x80070A20
               Heap            0 |   1976               S
  0x80070A20  R
              Data            544 |    544               A
  0x80070800  M
               Stack         112 |   2048
  0x80070790
              Unused
  0x80070000
             .....
  0x800311D8  F
              App Flash      4440                        L
  0x80030080  A
              Protected       128                        S
  0x80030000  H

 R0 : 0x00000000    R16: 0x00000000
 R1 : 0x80030138    R17: 0x00000000
 R2 : 0x800707D0    R18: 0x80070000
 R3 : 0x00000000    R19: 0x00000000
 R4 : 0x00000000    R20: 0x00000000
 R5 : 0x80070800    R21: 0x00000000
 R6 : 0x800707B0    R22: 0x00000000
 R7 : 0x00000000    R23: 0x00000000
 R8 : 0x80030000    R24: 0x00000000
 R9 : 0x00030000    R25: 0x00000000
 R10: 0x0000002D    R26: 0x00000000
 R11: 0x00000000    R27: 0x00000000
 R12: 0x00000000    R28: 0x00000000
 R13: 0x800300FC    R29: 0x00000000
 R14: 0x00000000    R30: 0x00000000
 R15: 0x00000000    R31: 0x00000000
 PC : 0x8003013C

 mcause: 0x00000005 (Load access fault)
 mtval:  0x00030000


 Total number of grant regions defined: 2
  Grant  0 : --          Grant  1 0x1: 0x800711d8
 PMPUserMPUConfig {
  id: 1,
  is_dirty: false,
  app_memory_region: Some(1),
  regions:
     #00: start=0x80030000, end=0x800311D8, cfg=0x0D (TOR) (-r-x)
     #01: start=0x80070000, end=0x80070A20, cfg=0x0B (TOR) (-rw-)
     #02: start=0x00000000, end=0x00000000, cfg=0x00 (OFF) (----)
     #03: start=0x00000000, end=0x00000000, cfg=0x00 (OFF) (----)
 }

To debug libtock-c apps, run `make lst` in the app's
folder and open the arch.0x80030080.0x80070000.lst file.

TEST_PASSED
```
The execution trace will be located in `exec.log`.

# Troubleshooting

When building the Tock application, you may encounter warnings related to downloads from inactive mirrors which make the process much longer (until an alternative mirror is used). You can remove the entry with the problematic mirror using `sed`:

    sed -i '/cs\.virginia\.edu/d' libtock-c/lib/fetch-newlib.sh libtock-c/lib/fetch-libc++.sh
