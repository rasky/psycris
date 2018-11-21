#include "bus.hpp"
#include <unordered_map>

namespace {
	struct io_port {
		uint32_t addr;
		uint32_t mask;

		bool operator==(io_port o) const {
			return addr == o.addr && mask == o.mask;
		}

		bool operator==(uint32_t a) const {
			return (addr & ~mask) == a;
		}
	};
}

namespace std {
	template <>
	struct hash<io_port> {
		size_t operator()(io_port const& p) const {
			uint32_t val = p.addr & ~p.mask;
			return std::hash<uint32_t>{}(val);
		}
	};
}

namespace {
	// this map is extracted from https://problemkaputt.de/psx-spx.htm#iomap
	std::unordered_map<io_port, std::string> io_map{
	    // Memory Control 1
	    {{0x1F801'000, 0}, "Expansion 1 Base Address (usually 1F000000h)"},
	    {{0x1F801'004, 0}, "Expansion 2 Base Address (usually 1F802000h)"},
	    {{0x1F801'008, 0}, "Expansion 1 Delay/Size (usually 0013243Fh; 512Kbytes 8bit-bus)"},
	    {{0x1F801'00C, 0}, "Expansion 3 Delay/Size (usually 00003022h; 1 byte)"},
	    {{0x1F801'010, 0}, "BIOS ROM    Delay/Size (usually 0013243Fh; 512Kbytes 8bit-bus)"},
	    {{0x1F801'014, 0}, "SPU_DELAY   Delay/Size (usually 200931E1h)"},
	    {{0x1F801'018, 0}, "CDROM_DELAY Delay/Size (usually 00020843h or 00020943h)"},
	    {{0x1F801'01C, 0}, "Expansion 2 Delay/Size (usually 00070777h; 128-bytes 8bit-bus)"},
	    {{0x1F801'020, 0}, "COM_DELAY / COMMON_DELAY (00031125h or 0000132Ch or 00001325h)"},
	    // Peripheral I/O Ports
	    {{0x1F801'040, 0}, "JOY_DATA Joypad/Memory Card Data (R/W)"},
	    {{0x1F801'044, 0}, "JOY_STAT Joypad/Memory Card Status (R)"},
	    {{0x1F801'048, 0}, "JOY_MODE Joypad/Memory Card Mode (R/W)"},
	    {{0x1F801'04A, 0}, "JOY_CTRL Joypad/Memory Card Control (R/W)"},
	    {{0x1F801'04E, 0}, "JOY_BAUD Joypad/Memory Card Baudrate (R/W)"},
	    {{0x1F801'050, 0}, "SIO_DATA Serial Port Data (R/W)"},
	    {{0x1F801'054, 0}, "SIO_STAT Serial Port Status (R)"},
	    {{0x1F801'058, 0}, "SIO_MODE Serial Port Mode (R/W)"},
	    {{0x1F801'05A, 0}, "SIO_CTRL Serial Port Control (R/W)"},
	    {{0x1F801'05C, 0}, "SIO_MISC Serial Port Internal Register (R/W)"},
	    {{0x1F801'05E, 0}, "SIO_BAUD Serial Port Baudrate (R/W)"},
	    // Memory Control 2
	    {{0x1F801'060, 0}, "RAM_SIZE (usually 00000B88h; 2MB RAM mirrored in first 8MB)"},
	    // Interrupt Control
	    {{0x1F801'070, 0}, "I_STAT - Interrupt status register"},
	    {{0x1F801'074, 0}, "I_MASK - Interrupt mask register"},
	    // DMA Registers
	    {{0x1F801'080, 0xf}, "DMA0 channel 0 - MDECin"},
	    {{0x1F801'090, 0xf}, "DMA1 channel 1 - MDECout"},
	    {{0x1F801'0A0, 0xf}, "DMA2 channel 2 - GPU (lists + image data)"},
	    {{0x1F801'0B0, 0xf}, "DMA3 channel 3 - CDROM"},
	    {{0x1F801'0C0, 0xf}, "DMA4 channel 4 - SPU"},
	    {{0x1F801'0D0, 0xf}, "DMA5 channel 5 - PIO (Expansion Port)"},
	    {{0x1F801'0E0, 0xf}, "DMA6 channel 6 - OTC (reverse clear OT) (GPU related)"},
	    {{0x1F801'0F0, 0}, "DPCR - DMA Control register"},
	    {{0x1F801'0F4, 0}, "DICR - DMA Interrupt register"},
	    {{0x1F801'0F8, 0}, "unknown"},
	    {{0x1F801'0FC, 0}, "unknown"},
	    // Timers (aka Root counters)
	    {{0x1F801'100, 0xf}, "Timer 0 Dotclock"},
	    {{0x1F801'110, 0xf}, "Timer 1 Horizontal Retrace"},
	    {{0x1F801'120, 0xf}, "Timer 2 1/8 system clock"},
	    // CDROM Registers (Address.Read/Write.Index)
	    {{0x1F801'800, 0}, "CD Index/Status Register (Bit0-1 R/W, Bit2-7 Read Only)"},
	    {{0x1F801'801, 0}, "CD Response Fifo (R) (usually with Index1)"},
	    {{0x1F801'802, 0}, "CD Data Fifo - 8bit/16bit (R) (usually with Index0..1)"},
	    {{0x1F801'803, 0}, "CD Interrupt Enable Register (R)"},
	    {{0x1F801'803, 0}, "CD Interrupt Flag Register (R/W)"},
	    {{0x1F801'803, 0}, "CD Interrupt Enable Register (R) (Mirror)"},
	    {{0x1F801'803, 0}, "CD Interrupt Flag Register (R/W) (Mirror)"},
	    {{0x1F801'801, 0}, "CD Command Register (W)"},
	    {{0x1F801'802, 0}, "CD Parameter Fifo (W)"},
	    {{0x1F801'803, 0}, "CD Request Register (W)"},
	    {{0x1F801'801, 0}, "Unknown/unused"},
	    {{0x1F801'802, 0}, "CD Interrupt Enable Register (W)"},
	    {{0x1F801'803, 0}, "CD Interrupt Flag Register (R/W)"},
	    {{0x1F801'801, 0}, "Unknown/unused"},
	    {{0x1F801'802, 0}, "CD Audio Volume for Left-CD-Out to Left-SPU-Input (W)"},
	    {{0x1F801'803, 0}, "CD Audio Volume for Left-CD-Out to Right-SPU-Input (W)"},
	    {{0x1F801'801, 0}, "CD Audio Volume for Right-CD-Out to Right-SPU-Input (W)"},
	    {{0x1F801'802, 0}, "CD Audio Volume for Right-CD-Out to Left-SPU-Input (W)"},
	    {{0x1F801'803, 0}, "CD Audio Volume Apply Changes (by writing bit5=1)"},
	    // GPU Registers
	    {{0x1F801'810, 0}, "GP0 Send GP0 Commands/Packets (Rendering and VRAM Access)"},
	    {{0x1F801'814, 0}, "GP1 Send GP1 Commands (Display Control)"},
	    {{0x1F801'810, 0}, "GPUREAD Read responses to GP0(C0h) and GP1(10h) commands"},
	    {{0x1F801'814, 0}, "GPUSTAT Read GPU Status Register"},
	    // MDEC Registers
	    {{0x1F801'820, 0}, "MDEC Command/Parameter Register (W)"},
	    {{0x1F801'820, 0}, "MDEC Data/Response Register (R)"},
	    {{0x1F801'824, 0}, "MDEC Control/Reset Register (W)"},
	    {{0x1F801'824, 0}, "MDEC Status Register (R)"},
	    // SPU Voice 0..23 Registers
	    {{0x1F801'C00 + 0 * 0x10, 0}, "Voice 1 Volume Left/Right"},
	    {{0x1F801'C04 + 0 * 0x10, 0}, "Voice 1 ADPCM Sample Rate"},
	    {{0x1F801'C06 + 0 * 0x10, 0}, "Voice 1 ADPCM Start Address"},
	    {{0x1F801'C08 + 0 * 0x10, 0}, "Voice 1 ADSR Attack/Decay/Sustain/Release"},
	    {{0x1F801'C0C + 0 * 0x10, 0}, "Voice 1 ADSR Current Volume"},
	    {{0x1F801'C0E + 0 * 0x10, 0}, "Voice 1 ADPCM Repeat Address"},
	    //
	    {{0x1F801'C00 + 1 * 0x10, 0}, "Voice 2 Volume Left/Right"},
	    {{0x1F801'C04 + 1 * 0x10, 0}, "Voice 2 ADPCM Sample Rate"},
	    {{0x1F801'C06 + 1 * 0x10, 0}, "Voice 2 ADPCM Start Address"},
	    {{0x1F801'C08 + 1 * 0x10, 0}, "Voice 2 ADSR Attack/Decay/Sustain/Release"},
	    {{0x1F801'C0C + 1 * 0x10, 0}, "Voice 2 ADSR Current Volume"},
	    {{0x1F801'C0E + 1 * 0x10, 0}, "Voice 2 ADPCM Repeat Address"},
	    //
	    {{0x1F801'C00 + 2 * 0x10, 0}, "Voice 3 Volume Left/Right"},
	    {{0x1F801'C04 + 2 * 0x10, 0}, "Voice 3 ADPCM Sample Rate"},
	    {{0x1F801'C06 + 2 * 0x10, 0}, "Voice 3 ADPCM Start Address"},
	    {{0x1F801'C08 + 2 * 0x10, 0}, "Voice 3 ADSR Attack/Decay/Sustain/Release"},
	    {{0x1F801'C0C + 2 * 0x10, 0}, "Voice 3 ADSR Current Volume"},
	    {{0x1F801'C0E + 2 * 0x10, 0}, "Voice 3 ADPCM Repeat Address"},
	    //
	    {{0x1F801'C00 + 3 * 0x10, 0}, "Voice 4 Volume Left/Right"},
	    {{0x1F801'C04 + 3 * 0x10, 0}, "Voice 4 ADPCM Sample Rate"},
	    {{0x1F801'C06 + 3 * 0x10, 0}, "Voice 4 ADPCM Start Address"},
	    {{0x1F801'C08 + 3 * 0x10, 0}, "Voice 4 ADSR Attack/Decay/Sustain/Release"},
	    {{0x1F801'C0C + 3 * 0x10, 0}, "Voice 4 ADSR Current Volume"},
	    {{0x1F801'C0E + 3 * 0x10, 0}, "Voice 4 ADPCM Repeat Address"},
	    //
	    {{0x1F801'C00 + 4 * 0x10, 0}, "Voice 5 Volume Left/Right"},
	    {{0x1F801'C04 + 4 * 0x10, 0}, "Voice 5 ADPCM Sample Rate"},
	    {{0x1F801'C06 + 4 * 0x10, 0}, "Voice 5 ADPCM Start Address"},
	    {{0x1F801'C08 + 4 * 0x10, 0}, "Voice 5 ADSR Attack/Decay/Sustain/Release"},
	    {{0x1F801'C0C + 4 * 0x10, 0}, "Voice 5 ADSR Current Volume"},
	    {{0x1F801'C0E + 4 * 0x10, 0}, "Voice 5 ADPCM Repeat Address"},
	    //
	    {{0x1F801'C00 + 5 * 0x10, 0}, "Voice 6 Volume Left/Right"},
	    {{0x1F801'C04 + 5 * 0x10, 0}, "Voice 6 ADPCM Sample Rate"},
	    {{0x1F801'C06 + 5 * 0x10, 0}, "Voice 6 ADPCM Start Address"},
	    {{0x1F801'C08 + 5 * 0x10, 0}, "Voice 6 ADSR Attack/Decay/Sustain/Release"},
	    {{0x1F801'C0C + 5 * 0x10, 0}, "Voice 6 ADSR Current Volume"},
	    {{0x1F801'C0E + 5 * 0x10, 0}, "Voice 6 ADPCM Repeat Address"},
	    //
	    {{0x1F801'C00 + 6 * 0x10, 0}, "Voice 7 Volume Left/Right"},
	    {{0x1F801'C04 + 6 * 0x10, 0}, "Voice 7 ADPCM Sample Rate"},
	    {{0x1F801'C06 + 6 * 0x10, 0}, "Voice 7 ADPCM Start Address"},
	    {{0x1F801'C08 + 6 * 0x10, 0}, "Voice 7 ADSR Attack/Decay/Sustain/Release"},
	    {{0x1F801'C0C + 6 * 0x10, 0}, "Voice 7 ADSR Current Volume"},
	    {{0x1F801'C0E + 6 * 0x10, 0}, "Voice 7 ADPCM Repeat Address"},
	    //
	    {{0x1F801'C00 + 7 * 0x10, 0}, "Voice 8 Volume Left/Right"},
	    {{0x1F801'C04 + 7 * 0x10, 0}, "Voice 8 ADPCM Sample Rate"},
	    {{0x1F801'C06 + 7 * 0x10, 0}, "Voice 8 ADPCM Start Address"},
	    {{0x1F801'C08 + 7 * 0x10, 0}, "Voice 8 ADSR Attack/Decay/Sustain/Release"},
	    {{0x1F801'C0C + 7 * 0x10, 0}, "Voice 8 ADSR Current Volume"},
	    {{0x1F801'C0E + 7 * 0x10, 0}, "Voice 8 ADPCM Repeat Address"},
	    //
	    {{0x1F801'C00 + 8 * 0x10, 0}, "Voice 9 Volume Left/Right"},
	    {{0x1F801'C04 + 8 * 0x10, 0}, "Voice 9 ADPCM Sample Rate"},
	    {{0x1F801'C06 + 8 * 0x10, 0}, "Voice 9 ADPCM Start Address"},
	    {{0x1F801'C08 + 8 * 0x10, 0}, "Voice 9 ADSR Attack/Decay/Sustain/Release"},
	    {{0x1F801'C0C + 8 * 0x10, 0}, "Voice 9 ADSR Current Volume"},
	    {{0x1F801'C0E + 8 * 0x10, 0}, "Voice 9 ADPCM Repeat Address"},
	    //
	    {{0x1F801'C00 + 9 * 0x10, 0}, "Voice 10 Volume Left/Right"},
	    {{0x1F801'C04 + 9 * 0x10, 0}, "Voice 10 ADPCM Sample Rate"},
	    {{0x1F801'C06 + 9 * 0x10, 0}, "Voice 10 ADPCM Start Address"},
	    {{0x1F801'C08 + 9 * 0x10, 0}, "Voice 10 ADSR Attack/Decay/Sustain/Release"},
	    {{0x1F801'C0C + 9 * 0x10, 0}, "Voice 10 ADSR Current Volume"},
	    {{0x1F801'C0E + 9 * 0x10, 0}, "Voice 10 ADPCM Repeat Address"},
	    //
	    {{0x1F801'C00 + 10 * 0x10, 0}, "Voice 11 Volume Left/Right"},
	    {{0x1F801'C04 + 10 * 0x10, 0}, "Voice 11 ADPCM Sample Rate"},
	    {{0x1F801'C06 + 10 * 0x10, 0}, "Voice 11 ADPCM Start Address"},
	    {{0x1F801'C08 + 10 * 0x10, 0}, "Voice 11 ADSR Attack/Decay/Sustain/Release"},
	    {{0x1F801'C0C + 10 * 0x10, 0}, "Voice 11 ADSR Current Volume"},
	    {{0x1F801'C0E + 10 * 0x10, 0}, "Voice 11 ADPCM Repeat Address"},
	    //
	    {{0x1F801'C00 + 11 * 0x10, 0}, "Voice 12 Volume Left/Right"},
	    {{0x1F801'C04 + 11 * 0x10, 0}, "Voice 12 ADPCM Sample Rate"},
	    {{0x1F801'C06 + 11 * 0x10, 0}, "Voice 12 ADPCM Start Address"},
	    {{0x1F801'C08 + 11 * 0x10, 0}, "Voice 12 ADSR Attack/Decay/Sustain/Release"},
	    {{0x1F801'C0C + 11 * 0x10, 0}, "Voice 12 ADSR Current Volume"},
	    {{0x1F801'C0E + 11 * 0x10, 0}, "Voice 12 ADPCM Repeat Address"},
	    //
	    {{0x1F801'C00 + 12 * 0x10, 0}, "Voice 13 Volume Left/Right"},
	    {{0x1F801'C04 + 12 * 0x10, 0}, "Voice 13 ADPCM Sample Rate"},
	    {{0x1F801'C06 + 12 * 0x10, 0}, "Voice 13 ADPCM Start Address"},
	    {{0x1F801'C08 + 12 * 0x10, 0}, "Voice 13 ADSR Attack/Decay/Sustain/Release"},
	    {{0x1F801'C0C + 12 * 0x10, 0}, "Voice 13 ADSR Current Volume"},
	    {{0x1F801'C0E + 12 * 0x10, 0}, "Voice 13 ADPCM Repeat Address"},
	    //
	    {{0x1F801'C00 + 13 * 0x10, 0}, "Voice 14 Volume Left/Right"},
	    {{0x1F801'C04 + 13 * 0x10, 0}, "Voice 14 ADPCM Sample Rate"},
	    {{0x1F801'C06 + 13 * 0x10, 0}, "Voice 14 ADPCM Start Address"},
	    {{0x1F801'C08 + 13 * 0x10, 0}, "Voice 14 ADSR Attack/Decay/Sustain/Release"},
	    {{0x1F801'C0C + 13 * 0x10, 0}, "Voice 14 ADSR Current Volume"},
	    {{0x1F801'C0E + 13 * 0x10, 0}, "Voice 14 ADPCM Repeat Address"},
	    //
	    {{0x1F801'C00 + 14 * 0x10, 0}, "Voice 15 Volume Left/Right"},
	    {{0x1F801'C04 + 14 * 0x10, 0}, "Voice 15 ADPCM Sample Rate"},
	    {{0x1F801'C06 + 14 * 0x10, 0}, "Voice 15 ADPCM Start Address"},
	    {{0x1F801'C08 + 14 * 0x10, 0}, "Voice 15 ADSR Attack/Decay/Sustain/Release"},
	    {{0x1F801'C0C + 14 * 0x10, 0}, "Voice 15 ADSR Current Volume"},
	    {{0x1F801'C0E + 14 * 0x10, 0}, "Voice 15 ADPCM Repeat Address"},
	    //
	    {{0x1F801'C00 + 15 * 0x10, 0}, "Voice 16 Volume Left/Right"},
	    {{0x1F801'C04 + 15 * 0x10, 0}, "Voice 16 ADPCM Sample Rate"},
	    {{0x1F801'C06 + 15 * 0x10, 0}, "Voice 16 ADPCM Start Address"},
	    {{0x1F801'C08 + 15 * 0x10, 0}, "Voice 16 ADSR Attack/Decay/Sustain/Release"},
	    {{0x1F801'C0C + 15 * 0x10, 0}, "Voice 16 ADSR Current Volume"},
	    {{0x1F801'C0E + 15 * 0x10, 0}, "Voice 16 ADPCM Repeat Address"},
	    //
	    {{0x1F801'C00 + 16 * 0x10, 0}, "Voice 17 Volume Left/Right"},
	    {{0x1F801'C04 + 16 * 0x10, 0}, "Voice 17 ADPCM Sample Rate"},
	    {{0x1F801'C06 + 16 * 0x10, 0}, "Voice 17 ADPCM Start Address"},
	    {{0x1F801'C08 + 16 * 0x10, 0}, "Voice 17 ADSR Attack/Decay/Sustain/Release"},
	    {{0x1F801'C0C + 16 * 0x10, 0}, "Voice 17 ADSR Current Volume"},
	    {{0x1F801'C0E + 16 * 0x10, 0}, "Voice 17 ADPCM Repeat Address"},
	    //
	    {{0x1F801'C00 + 17 * 0x10, 0}, "Voice 18 Volume Left/Right"},
	    {{0x1F801'C04 + 17 * 0x10, 0}, "Voice 18 ADPCM Sample Rate"},
	    {{0x1F801'C06 + 17 * 0x10, 0}, "Voice 18 ADPCM Start Address"},
	    {{0x1F801'C08 + 17 * 0x10, 0}, "Voice 18 ADSR Attack/Decay/Sustain/Release"},
	    {{0x1F801'C0C + 17 * 0x10, 0}, "Voice 18 ADSR Current Volume"},
	    {{0x1F801'C0E + 17 * 0x10, 0}, "Voice 18 ADPCM Repeat Address"},
	    //
	    {{0x1F801'C00 + 18 * 0x10, 0}, "Voice 19 Volume Left/Right"},
	    {{0x1F801'C04 + 18 * 0x10, 0}, "Voice 19 ADPCM Sample Rate"},
	    {{0x1F801'C06 + 18 * 0x10, 0}, "Voice 19 ADPCM Start Address"},
	    {{0x1F801'C08 + 18 * 0x10, 0}, "Voice 19 ADSR Attack/Decay/Sustain/Release"},
	    {{0x1F801'C0C + 18 * 0x10, 0}, "Voice 19 ADSR Current Volume"},
	    {{0x1F801'C0E + 18 * 0x10, 0}, "Voice 19 ADPCM Repeat Address"},
	    //
	    {{0x1F801'C00 + 19 * 0x10, 0}, "Voice 20 Volume Left/Right"},
	    {{0x1F801'C04 + 19 * 0x10, 0}, "Voice 20 ADPCM Sample Rate"},
	    {{0x1F801'C06 + 19 * 0x10, 0}, "Voice 20 ADPCM Start Address"},
	    {{0x1F801'C08 + 19 * 0x10, 0}, "Voice 20 ADSR Attack/Decay/Sustain/Release"},
	    {{0x1F801'C0C + 19 * 0x10, 0}, "Voice 20 ADSR Current Volume"},
	    {{0x1F801'C0E + 19 * 0x10, 0}, "Voice 20 ADPCM Repeat Address"},
	    //
	    {{0x1F801'C00 + 20 * 0x10, 0}, "Voice 21 Volume Left/Right"},
	    {{0x1F801'C04 + 20 * 0x10, 0}, "Voice 21 ADPCM Sample Rate"},
	    {{0x1F801'C06 + 20 * 0x10, 0}, "Voice 21 ADPCM Start Address"},
	    {{0x1F801'C08 + 20 * 0x10, 0}, "Voice 21 ADSR Attack/Decay/Sustain/Release"},
	    {{0x1F801'C0C + 20 * 0x10, 0}, "Voice 21 ADSR Current Volume"},
	    {{0x1F801'C0E + 20 * 0x10, 0}, "Voice 21 ADPCM Repeat Address"},
	    //
	    {{0x1F801'C00 + 21 * 0x10, 0}, "Voice 22 Volume Left/Right"},
	    {{0x1F801'C04 + 21 * 0x10, 0}, "Voice 22 ADPCM Sample Rate"},
	    {{0x1F801'C06 + 21 * 0x10, 0}, "Voice 22 ADPCM Start Address"},
	    {{0x1F801'C08 + 21 * 0x10, 0}, "Voice 22 ADSR Attack/Decay/Sustain/Release"},
	    {{0x1F801'C0C + 21 * 0x10, 0}, "Voice 22 ADSR Current Volume"},
	    {{0x1F801'C0E + 21 * 0x10, 0}, "Voice 22 ADPCM Repeat Address"},
	    //
	    {{0x1F801'C00 + 22 * 0x10, 0}, "Voice 23 Volume Left/Right"},
	    {{0x1F801'C04 + 22 * 0x10, 0}, "Voice 23 ADPCM Sample Rate"},
	    {{0x1F801'C06 + 22 * 0x10, 0}, "Voice 23 ADPCM Start Address"},
	    {{0x1F801'C08 + 22 * 0x10, 0}, "Voice 23 ADSR Attack/Decay/Sustain/Release"},
	    {{0x1F801'C0C + 22 * 0x10, 0}, "Voice 23 ADSR Current Volume"},
	    {{0x1F801'C0E + 22 * 0x10, 0}, "Voice 23 ADPCM Repeat Address"},
	    //
	    {{0x1F801'C00 + 23 * 0x10, 0}, "Voice 24 Volume Left/Right"},
	    {{0x1F801'C04 + 23 * 0x10, 0}, "Voice 24 ADPCM Sample Rate"},
	    {{0x1F801'C06 + 23 * 0x10, 0}, "Voice 24 ADPCM Start Address"},
	    {{0x1F801'C08 + 23 * 0x10, 0}, "Voice 24 ADSR Attack/Decay/Sustain/Release"},
	    {{0x1F801'C0C + 23 * 0x10, 0}, "Voice 24 ADSR Current Volume"},
	    {{0x1F801'C0E + 23 * 0x10, 0}, "Voice 24 ADPCM Repeat Address"},
	    // SPU Control Registers
	    {{0x1F801'D80, 0}, "Main Volume Left/Right"},
	    {{0x1F801'D84, 0}, "Reverb Output Volume Left/Right"},
	    {{0x1F801'D88, 0}, "Voice 0..23 Key ON (Start Attack/Decay/Sustain) (W)"},
	    {{0x1F801'D8C, 0}, "Voice 0..23 Key OFF (Start Release) (W)"},
	    {{0x1F801'D90, 0}, "Voice 0..23 Channel FM (pitch lfo) mode (R/W)"},
	    {{0x1F801'D94, 0}, "Voice 0..23 Channel Noise mode (R/W)"},
	    {{0x1F801'D98, 0}, "Voice 0..23 Channel Reverb mode (R/W)"},
	    {{0x1F801'D9C, 0}, "Voice 0..23 Channel ON/OFF (status) (R)"},
	    {{0x1F801'DA0, 0}, "Unknown? (R) or (W)"},
	    {{0x1F801'DA2, 0}, "Sound RAM Reverb Work Area Start Address"},
	    {{0x1F801'DA4, 0}, "Sound RAM IRQ Address"},
	    {{0x1F801'DA6, 0}, "Sound RAM Data Transfer Address"},
	    {{0x1F801'DA8, 0}, "Sound RAM Data Transfer Fifo"},
	    {{0x1F801'DAA, 0}, "SPU Control Register (SPUCNT)"},
	    {{0x1F801'DAC, 0}, "Sound RAM Data Transfer Control"},
	    {{0x1F801'DAE, 0}, "SPU Status Register (SPUSTAT) (R)"},
	    {{0x1F801'DB0, 0}, "CD Volume Left/Right"},
	    {{0x1F801'DB4, 0}, "Extern Volume Left/Right"},
	    {{0x1F801'DB8, 0}, "Current Main Volume Left/Right"},
	    {{0x1F801'DBC, 0}, "Unknown? (R/W)"},
	    // SPU Reverb Configuration Area
	    {{0x1F801'DC0, 0}, "dAPF1  Reverb APF Offset 1"},
	    {{0x1F801'DC2, 0}, "dAPF2  Reverb APF Offset 2"},
	    {{0x1F801'DC4, 0}, "vIIR   Reverb Reflection Volume 1"},
	    {{0x1F801'DC6, 0}, "vCOMB1 Reverb Comb Volume 1"},
	    {{0x1F801'DC8, 0}, "vCOMB2 Reverb Comb Volume 2"},
	    {{0x1F801'DCA, 0}, "vCOMB3 Reverb Comb Volume 3"},
	    {{0x1F801'DCC, 0}, "vCOMB4 Reverb Comb Volume 4"},
	    {{0x1F801'DCE, 0}, "vWALL  Reverb Reflection Volume 2"},
	    {{0x1F801'DD0, 0}, "vAPF1  Reverb APF Volume 1"},
	    {{0x1F801'DD2, 0}, "vAPF2  Reverb APF Volume 2"},
	    {{0x1F801'DD4, 0}, "mSAME  Reverb Same Side Reflection Address 1 Left/Right"},
	    {{0x1F801'DD8, 0}, "mCOMB1 Reverb Comb Address 1 Left/Right"},
	    {{0x1F801'DDC, 0}, "mCOMB2 Reverb Comb Address 2 Left/Right"},
	    {{0x1F801'DE0, 0}, "dSAME  Reverb Same Side Reflection Address 2 Left/Right"},
	    {{0x1F801'DE4, 0}, "mDIFF  Reverb Different Side Reflection Address 1 Left/Right"},
	    {{0x1F801'DE8, 0}, "mCOMB3 Reverb Comb Address 3 Left/Right"},
	    {{0x1F801'DEC, 0}, "mCOMB4 Reverb Comb Address 4 Left/Right"},
	    {{0x1F801'DF0, 0}, "dDIFF  Reverb Different Side Reflection Address 2 Left/Right"},
	    {{0x1F801'DF4, 0}, "mAPF1  Reverb APF Address 1 Left/Right"},
	    {{0x1F801'DF8, 0}, "mAPF2  Reverb APF Address 2 Left/Right"},
	    {{0x1F801'DFC, 0}, "vIN    Reverb Input Volume Left/Right"},
	};
}

namespace cpu {
	std::string guess_io_port(uint32_t addr) {
		addr &= 0x1fff'ffff;
		for (auto& [port, d] : io_map) {
			if (port == addr) {
				return d;
			}
		}
		return {};
	}
}
