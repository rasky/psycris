#pragma once
#include <cstdint>

namespace cpu {
	// clang-format off
	struct decoder {
		uint32_t ins;

		uint8_t opcode() const { return ins >> 26; }

		uint8_t funct() const{ return ins & 0x3f; }

		uint8_t rs() const { return (ins >> 21) & 0x1F; }

		uint8_t rt() const { return (ins >> 16) & 0x1F; }

		uint8_t rd() const { return (ins >> 11) & 0x1F; }

		int32_t imm16() const {
			// Note: ALL arithmetic immediate values are sign-extended. After
			// that, they are handled as signed or unsigned 32 bit numbers,
			// depending upon the instruction. The only difference between
			// signed and unsigned instructions is that signed instructions can
			// generate an overflow exception and unsigned instructions can not.
			return static_cast<int32_t>((ins & 0xffff) << 22) >> 22;
		}

        uint8_t imm5() const {
            return (ins & 0x7ff) >> 6;
        }
	};
	// clang-format on
}