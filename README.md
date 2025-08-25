# NVIDIA ROM Parser

A professional C++ tool for parsing NVIDIA GPU BIOS ROM files according to the official NVIDIA BIOS Information Table (BIT) specification. This tool extracts and displays comprehensive information from `.rom` files in a human-readable format.

## Features

- **Complete BIT Specification Compliance**: Implements parsing according to the official NVIDIA BIOS Information Table specification
- **Comprehensive Data Extraction**: Parses all major BIT tokens including BIOS data, string pointers, memory pointers, performance tables, and more
- **Human-Readable Output**: Displays all extracted information in an organized, easy-to-read format
- **Flexible Output Options**: Display to console and/or save to text file
- **Robust Error Handling**: Safe parsing with boundary checks and validation
- **Professional Code Quality**: Well-structured, documented, and maintainable C++ code

## Supported BIT Tokens

The parser recognizes and extracts data from the following BIT tokens:

- `BIT_TOKEN_I2C_PTRS` (0x32) - I2C Script Pointers
- `BIT_TOKEN_DAC_PTRS` (0x41) - DAC Data Pointers
- `BIT_TOKEN_BIOSDATA` (0x42) - BIOS Data (versions 1 & 2)
- `BIT_TOKEN_CLOCK_PTRS` (0x43) - Clock Script Pointers
- `BIT_TOKEN_DFP_PTRS` (0x44) - DFP/Panel Data Pointers
- `BIT_TOKEN_NVINIT_PTRS` (0x49) - Initialization Table Pointers
- `BIT_TOKEN_LVDS_PTRS` (0x4C) - LVDS Table Pointers
- `BIT_TOKEN_MEMORY_PTRS` (0x4D) - Memory Control/Programming Pointers
- `BIT_TOKEN_NOP` (0x4E) - No Operation
- `BIT_TOKEN_PERF_PTRS` (0x50) - Performance Table Pointers
- `BIT_TOKEN_BRIDGE_FW_DATA` (0x52) - Bridge Firmware Data
- `BIT_TOKEN_STRING_PTRS` (0x53) - String Pointers (versions 1 & 2)
- `BIT_TOKEN_TMDS_PTRS` (0x54) - TMDS Table Pointers
- `BIT_TOKEN_DISPLAY_PTRS` (0x55) - Display Control/Programming Pointers
- `BIT_TOKEN_VIRTUAL_PTRS` (0x56) - Virtual Field Pointers
- `BIT_TOKEN_32BIT_PTRS` (0x63) - 32-bit Pointer Data
- `BIT_TOKEN_DP_PTRS` (0x64) - DP Table Pointers
- `BIT_TOKEN_FALCON_DATA` (0x70) - Falcon Ucode Data
- `BIT_TOKEN_UEFI_DATA` (0x75) - UEFI Driver Data
- `BIT_TOKEN_MXM_DATA` (0x78) - MXM Configuration Data

## Requirements

- **Compiler**: GCC 4.8+ or Clang 3.4+ with C++11 support
- **Operating System**: Linux (tested), macOS, Windows (with MinGW/MSYS2)
- **Memory**: Sufficient RAM to load the ROM file (typically 512KB - 2MB)

## Compilation

### Quick Start

```bash
# Clone or download the source files
# Compile using the Makefile
make

# Or compile manually
g++ -std=c++11 -Wall -Wextra -O2 -o nvidia_rom_parser nvidia_rom_parser.cpp
```

### Compilation Options

```bash
# Release build (optimized)
make release

# Debug build (with debugging symbols)
make debug

# Clean build artifacts
make clean

# System-wide installation
make install

# Remove from system
make uninstall
```

### Manual Compilation Examples

```bash
# Standard release build
g++ -std=c++11 -Wall -Wextra -O2 -DNDEBUG -o nvidia_rom_parser nvidia_rom_parser.cpp

# Debug build
g++ -std=c++11 -Wall -Wextra -g -DDEBUG -o nvidia_rom_parser nvidia_rom_parser.cpp

# With additional warnings
g++ -std=c++11 -Wall -Wextra -Wpedantic -O2 -o nvidia_rom_parser nvidia_rom_parser.cpp
```

## Usage

### Basic Usage

```bash
# Parse ROM file and display to console
./nvidia_rom_parser filename.rom

# Parse ROM file and save output to text file
./nvidia_rom_parser filename.rom output.txt

# Parse ROM file with both console and file output
./nvidia_rom_parser filename.rom analysis_report.txt
