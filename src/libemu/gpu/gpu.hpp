#pragma once
#include "../devices/mmap_device.hpp"

namespace psycris::gpu {
	using hw::data_reg;
	using hw::mmap_device;

	class controller : public mmap_device<controller, 8> {
	  public:
		static constexpr char const* device_name = "GPU controller";

		controller(gsl::span<uint8_t, size> buffer);

	  private:
		using gp0 = data_reg<0, 4>;
		using gp1 = data_reg<4, 4>;

		friend mmap_device;

		void wcb(gp0, uint32_t, uint32_t);
		void wcb(gp1, uint32_t, uint32_t);
	};

	class cxd : public mmap_device<cxd, 4> {
	  public:
		static constexpr char const* device_name = "GPU CXD???";

		cxd(gsl::span<uint8_t, size> buffer);

	  public:
		void run(uint64_t until);

		uint64_t ticks() const;

	  private:
		friend mmap_device;

		void wcb(class XXX);
	};
}