# XGDemo

Demonstration of the 3D capabilities of XGSoC.

# Requirements

- OSS CAD Suite (https://github.com/YosysHQ/oss-cad-suite-build) (*)
- xPack RISC-V Embedded GCC (https://github.com/xpack-dev-tools/riscv-none-elf-gcc-xpack/releases) (*)
- Python3 with the following PIP3 packages installed: `pyserial`

(*) Extract and add the `bin` directory to the path.

# Getting Started

## Clone and Update

### Clone

```bash
git clone --recurse-submodules https://github.com/danodus/xgdemo.git
cd xgdemo
```

If the repository has been cloned without the `--recurse-submodules` option, do the following:
```
git submodule update --init --recursive
```

### Update

```bash
git pull
git submodule update --recursive
```

## Getting Started on ULX3S

- Copy the folder `sw/assets` to a FAT32 SD card
- Insert the SD card in your ULX3S
- Do the following:

```bash
make prog run
```
