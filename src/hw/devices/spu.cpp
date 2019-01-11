#include "spu.hpp"
#include "../../bitmask.hpp"
#include "../../logging.hpp"

namespace {
	using psycris::hw::data_reg;

	// SPU Volume and ADSR Generator
	//
	// This SPU has 24 voices, for each one 16 byte of address space are
	// reserved.
	//
	// On the PSX the SPU mapping starts at 0x1f80'1c00, the first 24 * 16
	// bytes are used to map the volumes and adsr for every voice.
	//            +-------------------------------+
	//     voice0 |L|L|R|R|x|x|x|x|A|A|A|A|V|V|x|x|
	//            +-------------------------------+
	// 1fb81c00 +  0   2   4       8       c   e
	//
	//          L = volume left
	//          R = volume right
	//          A = ADSR
	//          V = volume ADSR
	//          x = unused

	// clang-format off
    using v0_vol_left   = data_reg< 0 +  0 * 0x10, 2>;
    using v0_vol_right  = data_reg< 2 +  0 * 0x10, 2>;
    using v0_adsr       = data_reg< 8 +  0 * 0x10, 4>;
    using v0_vol_adsr   = data_reg<12 +  0 * 0x10, 2>;
    using v1_vol_left   = data_reg< 0 +  1 * 0x10, 2>;
    using v1_vol_right  = data_reg< 2 +  1 * 0x10, 2>;
    using v1_adsr       = data_reg< 8 +  1 * 0x10, 4>;
    using v1_vol_adsr   = data_reg<12 +  1 * 0x10, 2>;
    using v2_vol_left   = data_reg< 0 +  2 * 0x10, 2>;
    using v2_vol_right  = data_reg< 2 +  2 * 0x10, 2>;
    using v2_adsr       = data_reg< 8 +  2 * 0x10, 4>;
    using v2_vol_adsr   = data_reg<12 +  2 * 0x10, 2>;
    using v3_vol_left   = data_reg< 0 +  3 * 0x10, 2>;
    using v3_vol_right  = data_reg< 2 +  3 * 0x10, 2>;
    using v3_adsr       = data_reg< 8 +  3 * 0x10, 4>;
    using v3_vol_adsr   = data_reg<12 +  3 * 0x10, 2>;
    using v4_vol_left   = data_reg< 0 +  4 * 0x10, 2>;
    using v4_vol_right  = data_reg< 2 +  4 * 0x10, 2>;
    using v4_adsr       = data_reg< 8 +  4 * 0x10, 4>;
    using v4_vol_adsr   = data_reg<12 +  4 * 0x10, 2>;
    using v5_vol_left   = data_reg< 0 +  5 * 0x10, 2>;
    using v5_vol_right  = data_reg< 2 +  5 * 0x10, 2>;
    using v5_adsr       = data_reg< 8 +  5 * 0x10, 4>;
    using v5_vol_adsr   = data_reg<12 +  5 * 0x10, 2>;
    using v6_vol_left   = data_reg< 0 +  6 * 0x10, 2>;
    using v6_vol_right  = data_reg< 2 +  6 * 0x10, 2>;
    using v6_adsr       = data_reg< 8 +  6 * 0x10, 4>;
    using v6_vol_adsr   = data_reg<12 +  6 * 0x10, 2>;
    using v7_vol_left   = data_reg< 0 +  7 * 0x10, 2>;
    using v7_vol_right  = data_reg< 2 +  7 * 0x10, 2>;
    using v7_adsr       = data_reg< 8 +  7 * 0x10, 4>;
    using v7_vol_adsr   = data_reg<12 +  7 * 0x10, 2>;
    using v8_vol_left   = data_reg< 0 +  8 * 0x10, 2>;
    using v8_vol_right  = data_reg< 2 +  8 * 0x10, 2>;
    using v8_adsr       = data_reg< 8 +  8 * 0x10, 4>;
    using v8_vol_adsr   = data_reg<12 +  8 * 0x10, 2>;
    using v9_vol_left   = data_reg< 0 +  9 * 0x10, 2>;
    using v9_vol_right  = data_reg< 2 +  9 * 0x10, 2>;
    using v9_adsr       = data_reg< 8 +  9 * 0x10, 4>;
    using v9_vol_adsr   = data_reg<12 +  9 * 0x10, 2>;
    using v10_vol_left  = data_reg< 0 + 10 * 0x10, 2>;
    using v10_vol_right = data_reg< 2 + 10 * 0x10, 2>;
    using v10_adsr      = data_reg< 8 + 10 * 0x10, 4>;
    using v10_vol_adsr  = data_reg<12 + 10 * 0x10, 2>;
    using v11_vol_left  = data_reg< 0 + 11 * 0x10, 2>;
    using v11_vol_right = data_reg< 2 + 11 * 0x10, 2>;
    using v11_adsr      = data_reg< 8 + 11 * 0x10, 4>;
    using v11_vol_adsr  = data_reg<12 + 11 * 0x10, 2>;
    using v12_vol_left  = data_reg< 0 + 12 * 0x10, 2>;
    using v12_vol_right = data_reg< 2 + 12 * 0x10, 2>;
    using v12_adsr      = data_reg< 8 + 12 * 0x10, 4>;
    using v12_vol_adsr  = data_reg<12 + 12 * 0x10, 2>;
    using v13_vol_left  = data_reg< 0 + 13 * 0x10, 2>;
    using v13_vol_right = data_reg< 2 + 13 * 0x10, 2>;
    using v13_adsr      = data_reg< 8 + 13 * 0x10, 4>;
    using v13_vol_adsr  = data_reg<12 + 13 * 0x10, 2>;
    using v14_vol_left  = data_reg< 0 + 14 * 0x10, 2>;
    using v14_vol_right = data_reg< 2 + 14 * 0x10, 2>;
    using v14_adsr      = data_reg< 8 + 14 * 0x10, 4>;
    using v14_vol_adsr  = data_reg<12 + 14 * 0x10, 2>;
    using v15_vol_left  = data_reg< 0 + 15 * 0x10, 2>;
    using v15_vol_right = data_reg< 2 + 15 * 0x10, 2>;
    using v15_adsr      = data_reg< 8 + 15 * 0x10, 4>;
    using v15_vol_adsr  = data_reg<12 + 15 * 0x10, 2>;
    using v16_vol_left  = data_reg< 0 + 16 * 0x10, 2>;
    using v16_vol_right = data_reg< 2 + 16 * 0x10, 2>;
    using v16_adsr      = data_reg< 8 + 16 * 0x10, 4>;
    using v16_vol_adsr  = data_reg<12 + 16 * 0x10, 2>;
    using v17_vol_left  = data_reg< 0 + 17 * 0x10, 2>;
    using v17_vol_right = data_reg< 2 + 17 * 0x10, 2>;
    using v17_adsr      = data_reg< 8 + 17 * 0x10, 4>;
    using v17_vol_adsr  = data_reg<12 + 17 * 0x10, 2>;
    using v18_vol_left  = data_reg< 0 + 18 * 0x10, 2>;
    using v18_vol_right = data_reg< 2 + 18 * 0x10, 2>;
    using v18_adsr      = data_reg< 8 + 18 * 0x10, 4>;
    using v18_vol_adsr  = data_reg<12 + 18 * 0x10, 2>;
    using v19_vol_left  = data_reg< 0 + 19 * 0x10, 2>;
    using v19_vol_right = data_reg< 2 + 19 * 0x10, 2>;
    using v19_adsr      = data_reg< 8 + 19 * 0x10, 4>;
    using v19_vol_adsr  = data_reg<12 + 19 * 0x10, 2>;
    using v20_vol_left  = data_reg< 0 + 20 * 0x10, 2>;
    using v20_vol_right = data_reg< 2 + 20 * 0x10, 2>;
    using v20_adsr      = data_reg< 8 + 20 * 0x10, 4>;
    using v20_vol_adsr  = data_reg<12 + 20 * 0x10, 2>;
    using v21_vol_left  = data_reg< 0 + 21 * 0x10, 2>;
    using v21_vol_right = data_reg< 2 + 21 * 0x10, 2>;
    using v21_adsr      = data_reg< 8 + 21 * 0x10, 4>;
    using v21_vol_adsr  = data_reg<12 + 21 * 0x10, 2>;
    using v22_vol_left  = data_reg< 0 + 22 * 0x10, 2>;
    using v22_vol_right = data_reg< 2 + 22 * 0x10, 2>;
    using v22_adsr      = data_reg< 8 + 22 * 0x10, 4>;
    using v22_vol_adsr  = data_reg<12 + 22 * 0x10, 2>;
    using v23_vol_left  = data_reg< 0 + 23 * 0x10, 2>;
    using v23_vol_right = data_reg< 2 + 23 * 0x10, 2>;
    using v23_adsr      = data_reg< 8 + 23 * 0x10, 4>;
    using v23_vol_adsr  = data_reg<12 + 23 * 0x10, 2>;
	// clang-format on

	using main_volume_left = data_reg<384, 2>;
	using main_volume_right = data_reg<386, 2>;

	using cd_audio_input_volume = data_reg<432, 4>;
	using ext_audio_input_volume = data_reg<436, 4>;

	// SPU Voice Flags
	using adsr_kon = data_reg<392, 4>;
	using adsr_koff = data_reg<396, 4>;
	using adsr_endx = data_reg<412, 4>;

	// SPU Noise Generator
	using noise_gen = data_reg<404, 4>;

	// SPU Memory Access
	using data_transfer_addr = data_reg<422, 2>;
	using data_transfer_fifo = data_reg<424, 2>;
	using data_transfer_ctrl = data_reg<428, 2>;

	// SPU Interrupt
	using irq_address = data_reg<420, 2>;

	// SPU Reverb Registers
	using rev_echo_on = data_reg<408, 4>;

	using rev_out_vol_left = data_reg<388, 2>;
	using rev_out_vol_right = data_reg<390, 2>;
	using rev_work_start_addr = data_reg<418, 2>;
	using rev_apf_offset1 = data_reg<448, 2>;
	using rev_apf_offset2 = data_reg<450, 2>;
	using rev_ref_vol1 = data_reg<452, 2>;
	using rev_comb_vol1 = data_reg<454, 2>;
	using rev_comb_vol2 = data_reg<456, 2>;
	using rev_comb_vol3 = data_reg<458, 2>;
	using rev_comb_vol4 = data_reg<460, 2>;
	using rev_ref_vol2 = data_reg<462, 2>;
	using rev_apf_vol1 = data_reg<464, 2>;
	using rev_apf_vol2 = data_reg<466, 2>;
	using rev_ssr_addr1_left = data_reg<468, 2>;
	using rev_ssr_addr1_right = data_reg<470, 2>;
	using rev_comb_addr1_left = data_reg<472, 2>;
	using rev_comb_addr1_right = data_reg<474, 2>;
	using rev_comb_addr2_left = data_reg<476, 2>;
	using rev_comb_addr2_right = data_reg<478, 2>;
	using rev_ssr_addr2_left = data_reg<480, 2>;
	using rev_ssr_addr2_right = data_reg<482, 2>;
	using rev_dsr_addr1_left = data_reg<484, 2>;
	using rev_dsr_addr1_right = data_reg<486, 2>;
	using rev_comb_addr3_left = data_reg<488, 2>;
	using rev_comb_addr3_right = data_reg<490, 2>;
	using rev_comb_addr4_left = data_reg<492, 2>;
	using rev_comb_addr4_right = data_reg<494, 2>;
	using rev_dsr_addr2_left = data_reg<496, 2>;
	using rev_dsr_addr2_right = data_reg<498, 2>;
	using rev_apf_addr1_left = data_reg<500, 2>;
	using rev_apf_addr1_right = data_reg<502, 2>;
	using rev_apf_addr2_left = data_reg<504, 2>;
	using rev_apf_addr2_right = data_reg<506, 2>;
	using rev_in_vol_left = data_reg<508, 2>;
	using rev_in_vol_right = data_reg<510, 2>;
}

namespace {
	namespace spucnt_bits {
		using mask = psycris::bit_mask<class spucnt_bits, uint16_t>;

		constexpr mask enable = mask{0x8000};
		constexpr mask mute = mask{0x4000};
		constexpr mask noise_freq_shift = mask{0x3c00};
		constexpr mask noise_freq_step = mask{0x0300};
		constexpr mask rev_enable = mask{0x0080};
		constexpr mask irq9_enable = mask{0x0040};

		enum transfer_mode {
			stop = 0,
			manual_write = 1,
			dma_write = 2,
			dma_read = 3,
		};
		constexpr mask ram_transfer_mode = mask{0x0030};

		constexpr mask ext_audio_rev = mask{0x0008};
		constexpr mask cd_audio_rev = mask{0x0004};
		constexpr mask ext_audio_enable = mask{0x0002};
		constexpr mask cd_audio_enable = mask{0x0001};

		// spu_mode is an utility mask to handle in one go all the bits that
		// must be copied from spucnt to spustat
		constexpr mask spu_mode = mask{0x003f};
	}
}

namespace psycris::hw {
	using psycris::log;

	spu::spu(gsl::span<uint8_t, size> buffer) : mmap_device{buffer, spucnt{}, spustat{}} {}

	void spu::wcb(spucnt, uint32_t new_value, uint32_t) {
		using namespace spucnt_bits;
		// XXX
		uint16_t w = 0;
		spu_mode(w) = spu_mode(new_value);
		write<spustat>(w);
		log->info("copying {:x} {:x}", new_value, w);
	}
	void spu::wcb(spustat, uint32_t, uint32_t) { log->warn("SPUSTAT should be R/O"); }
}