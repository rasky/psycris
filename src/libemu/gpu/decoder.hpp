#pragma once
#include <cstdint>
#include <utility>

namespace psycris::gpu {
	struct decoder {
	  public:
		uint32_t ins;

	  public:
		uint8_t opcode() const { return ins >> 24; }

	  public:
		// extract the color attribute
		uint32_t bgr() const { return ins & 0x00ff'ffff; }

		// extract the vertex(x, y) attribute
		std::pair<uint16_t, uint16_t> vertex() const {     //
			return {ins & 0x0000'03ff, ins & 0x03ff'0000}; //
		}                                                  //
	};
}