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

	enum class semi_trans_mode : uint8_t {
		half_back_half_front = 0,
		back_plus_front = 1,
		back_minus_front = 2,
		back_plus_front_quarter = 3,
	};

	enum class texture_page_colors : uint8_t {
		bit4 = 0,
		bit8 = 1,
		bit14 = 2,
		reserved = 3,
	};

	class draw_mode_decoder {
	  public:
		uint32_t ins;

		draw_mode_decoder(uint32_t i) : ins{i} {}
		draw_mode_decoder(decoder d) : ins{d.ins} {}

	  public:
		uint8_t x_base() const { return ins & 0x0000'0007; }

		uint8_t y_base() const { return (ins & 0x0000'0010) >> 4; }

		semi_trans_mode stm() const { return static_cast<semi_trans_mode>((ins & 0x0000'0060) >> 5); }

		texture_page_colors tpc() const { return static_cast<texture_page_colors>((ins & 0x0000'0180) >> 7); }

		bool dither() const { return ins & 0x0000'0200; }

		bool drawing_enabled() const { return ins & 0x0000'0400; }

		bool texture_disabled() const { return ins & 0x0000'0800; }

		bool x_flip() const { return ins & 0x0000'1000; }

		bool y_flip() const { return ins & 0x0000'2000; }
	};
}