#pragma once
#include <gsl/span>
#include <istream>

namespace psycris {
	/**
	 * \brief loads a bios
	 *
	 * The `istream` content is read as-is into the memory region; no parsing is
	 * performed on the input data.
	 */
	void load_bios(std::istream&, gsl::span<uint8_t>);

	/**
	 * \brief loads a PSX-EXE
	 *
	 * The `istream` content is parsed and placed into the memory region.
	 */
	// TODO - re-enable when needed
	// void load_exe(std::istream&, gsl::span<uint8_t>);
}