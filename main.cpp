#include <fstream>
#include <iomanip>
#include <sstream>
#include <cassert>

static uint8_t bios[512*1024];
static uint8_t ram[2*1024*1024];

class Bus {
public:
	template <typename T>
	T Read(uint32_t addr);

	template <typename T>
	void Write(uint32_t addr, T val);
};

template <typename T>
std::string hex(T i) {
  std::stringstream stream;
  stream << std::setfill ('0') << std::setw(sizeof(T)*2) << std::hex << i;
  return stream.str();
}

template <typename T> T Bus::Read(uint32_t addr) {
	switch (addr>>24) {
		case 0x1F: case 0x9F: case 0xBF:
			switch ((addr>>16)&0xFF) {
				case 0xC0: case 0xC1: case 0xC2: case 0xC3:
				case 0xC4: case 0xC5: case 0xC6: case 0xC7:
					// 0x1FC0-0x1FC7: BIOS
					return *(T*)(bios + (addr & 0x3FFFF));
				default:
					printf("[BUS] unmapped read: %08x", addr);
					return 0xFFFFFFFF;
			}
			assert(0);

		default:
			printf("[BUS] unmapped read: %08x", addr);
			return 0xFFFFFFFF;
	}
	assert(0);
}

template <typename T> void Bus::Write(uint32_t addr, T val) {
	switch (addr>>24) {
		case 0x1F: case 0x9F: case 0xBF:
			switch ((addr>>16)&0xFF) {
				case 0xC0: case 0xC1: case 0xC2: case 0xC3:
				case 0xC4: case 0xC5: case 0xC6: case 0xC7:
					// 0x1FC0-0x1FC7: BIOS
					printf("[BUS] skipping write into BIOS: %08x <- %s\n", addr, hex(val).c_str());
					return;

				default:
					printf("[BUS] unmapped write: %08x <- %s\n", addr, hex(val).c_str());
					return;
			}
			assert(0);

		default:
			printf("[BUS] unmapped write: %08x <- %s\n", addr, hex(val).c_str());
			return;
	}
	assert(0);
}


class Cpu {
public:
	uint32_t regs[32];
	uint32_t pc;
	uint64_t clock;

	Bus *bus;

public:
	Cpu(Bus *bus);
	void Reset(void);
	void Run(uint64_t until);
};

// MIPS opcode decoding helper
struct mipsop {
	uint32_t _op;
	Cpu *_cpu;

private:
	uint32_t _rs() { return (_op>>21)&0x1F; }
	uint32_t _rt() { return (_op>>16)&0x1F; }
	uint32_t _rd() { return (_op>>11)&0x1F; }

public:
	mipsop(Cpu *cpu, uint32_t op) : _cpu(cpu), _op(op) {}
	uint32_t op() { return _op>>26; }
	uint32_t* rs() { return &_cpu->regs[_rs()]; }
	uint32_t* rt() { return &_cpu->regs[_rt()]; }
	uint32_t* rd() { return &_cpu->regs[_rd()]; }
	uint32_t imm32() { return _op&0xffff; }
	uint32_t ea() { return *rs() + imm32(); }
};

Cpu::Cpu(Bus *bus_) : bus(bus_) {
	Reset();
}

void Cpu::Reset(void) {
	for (int i=0;i<32;i++) {
		regs[i] = 0;
	}
	clock = 0;
	pc = 0x1FC00000; // reset vector
}

void Cpu::Run(uint64_t until) {
	while (clock < until) {
		auto op = mipsop(this, bus->Read<uint32_t>(pc));
		switch (op.op()) {
		case 0x0F: *op.rt() = op.imm32()<<16; break;             // LUI
		case 0x0D: *op.rt() = *op.rs() | op.imm32(); break;      // ORI
		case 0x2B: bus->Write(op.ea(), *op.rt()); break;         // SW

		default:
			printf("[CPU] unimplemented opcode: %02x\n", op.op());
			assert(0);
		}

		pc += 4;
	}
}

int main(int argc, char *argv[]) {
	std::ifstream f("bios/SCPH7003.bin", std::ios::binary | std::ios::in);
	if (!f.read((char*)&bios[0], sizeof(bios))) {
		assert(!"cannot read BIOS");
	}

	Bus bus;
	Cpu cpu(&bus);

	cpu.Run(1000);
}
