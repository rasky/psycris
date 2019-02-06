#pragma once

namespace psycris::cpu {
	class mips;
}

namespace psycris::gpu {
	class cxd;
}

namespace psycris {
	class scheduler {
	  public:
		scheduler(cpu::mips&, gpu::cxd&);

	  public:
		void run();

	  private:
		cpu::mips* _cpu;
		gpu::cxd* _gpu;
	};
}