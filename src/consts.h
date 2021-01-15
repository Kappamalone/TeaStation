#pragma once
#include <types.h>
#include <array>

constexpr u32 KILOBYTE = 1024;

//Why can't auto deduce type here?
constexpr std::array <u32, 8> REGION_MASK = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, //KUSEG: 2048 MB
											   0x7fffffff, //KSEG0: 512MB
											   0x1fffffff, //KSEG1: 512MB
											   0xffffffff, 0xffffffff };  //KSEG2: 1024MB
constexpr u32 KERNEL_START = 0;
constexpr u32 KERNEL_END = 0xffff;
constexpr u32 KUSEG_START = 0x10000;
constexpr u32 KUSEG_END = 0x1fffff;
constexpr u32 EXPANSION1_START = 0x1f000000;
constexpr u32 EXPANSION1_END = 0x1f00ffff;
constexpr u32 SCRATCHPAD_START = 0x1f800000;
constexpr u32 SCRATCHPAD_END = 0x1f8003ff;
constexpr u32 MMIO_START = 0x1f801000;
constexpr u32 MMIO_END = 0x1f802fff;
constexpr u32 BIOS_START = 0x1fc00000;
constexpr u32 BIOS_END = 0x1fc7ffff;
