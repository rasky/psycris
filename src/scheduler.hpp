#pragma once

namespace psycris::cpu {
	class mips;
}

namespace psycris::gpu {
	class gpu;
}

namespace psycris {
	class scheduler {
	  public:
		scheduler(cpu::mips&, gpu::gpu&);

	  public:
		void run();

	  private:
		cpu::mips* _cpu;
		gpu::gpu* _gpu;
	};
}