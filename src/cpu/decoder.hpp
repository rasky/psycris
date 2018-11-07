#pragma once
#include <cstdint>

namespace cpu {
	// clang-format off
	struct decoder {
      public:
		uint32_t ins;

      public:
        decoder& operator=(uint32_t i) {
            ins = i;
            return *this;
        }

        operator uint32_t() const {
            return ins;

        }

      public:
		uint8_t opcode() const { return ins >> 26; }

		uint8_t funct() const{ return ins & 0x3f; }

		uint8_t rs() const { return (ins >> 21) & 0x1F; }

		uint8_t rt() const { return (ins >> 16) & 0x1F; }

		uint8_t rd() const { return (ins >> 11) & 0x1F; }

        uint8_t shamt() const {
            return (ins & 0x7ff) >> 6;
        }

		int32_t imm() const {
			// Note: ALL arithmetic immediate values are sign-extended. After
			// that, they are handled as signed or unsigned 32 bit numbers,
			// depending upon the instruction. The only difference between
			// signed and unsigned instructions is that signed instructions can
			// generate an overflow exception and unsigned instructions can not.
			return static_cast<int32_t>((ins & 0xffff) << 16) >> 16;
		}

        uint32_t uimm() const {
			return ins & 0xffff;
		}

		uint32_t target() const {
            return ins & 0x3ff'ffff;
        }

        /**
         * \brief checks if it is a coprocessor instruction
         */
        bool is_cop() const {
            return ins & 0x4000'0000;
        }

        /**
         * \brief checks if it is a call to coprocessor function
         */
        bool is_cop_fn() const {
            return ins & 0x0200'0000;
        }

        /**
         * \brief the coprocessor number
         */
        uint8_t cop_n() const {
            return opcode() & 0x3;
        }

        /**
         * \brief Returns the coprocessor sub operation.
         *
         * For the return value to be meaningful `is_cop()` must be true and
         * `is_cop_fn()` must be false.
         */
        uint8_t cop_subop() const {
            return rs();
        }

        /**
         * \brief Returns the coprocessor function.
         *
         * For the return value to be meaningful both `is_cop()` and
         * `is_cop_fn()` must be true.
         */
        uint32_t cop_fn() const {
            return ins & 0x01ff'ffff;
        }
    };
	// clang-format on
}