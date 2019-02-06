#include "disassembly.hpp"
#include "decoder.hpp"
#include <unordered_map>

namespace {
	std::unordered_map<uint8_t, std::string> gp0_ins{
	    // GPU Render Polygon Commands
	    {0x20, "Monochrome three-point polygon, opaque"},
	    {0x22, "Monochrome three-point polygon, semi-transparent"},
	    {0x28, "Monochrome four-point polygon, opaque"},
	    {0x2A, "Monochrome four-point polygon, semi-transparent"},
	    {0x24, "Textured three-point polygon, opaque, texture-blending"},
	    {0x25, "Textured three-point polygon, opaque, raw-texture"},
	    {0x26, "Textured three-point polygon, semi-transparent, texture-blending"},
	    {0x27, "Textured three-point polygon, semi-transparent, raw-texture"},
	    {0x2C, "Textured four-point polygon, opaque, texture-blending"},
	    {0x2D, "Textured four-point polygon, opaque, raw-texture"},
	    {0x2E, "Textured four-point polygon, semi-transparent, texture-blending"},
	    {0x2F, "Textured four-point polygon, semi-transparent, raw-texture"},
	    {0x30, "Shaded three-point polygon, opaque"},
	    {0x32, "Shaded three-point polygon, semi-transparent"},
	    {0x38, "Shaded four-point polygon, opaque"},
	    {0x3A, "Shaded four-point polygon, semi-transparent"},
	    {0x34, "Shaded Textured three-point polygon, opaque, texture-blending"},
	    {0x36, "Shaded Textured three-point polygon, semi-transparent, tex-blend"},
	    {0x3C, "Shaded Textured four-point polygon, opaque, texture-blending"},
	    {0x3E, "Shaded Textured four-point polygon, semi-transparent, tex-blend"},

	    // GPU Render Line Commands
	    {0x40, "Monochrome line, opaque"},
	    {0x42, "Monochrome line, semi-transparent"},
	    {0x48, "Monochrome Poly-line, opaque"},
	    {0x4A, "Monochrome Poly-line, semi-transparent"},
	    {0x50, "Shaded line, opaque"},
	    {0x52, "Shaded line, semi-transparent"},
	    {0x58, "Shaded Poly-line, opaque"},
	    {0x5A, "Shaded Poly-line, semi-transparent"},

	    // GPU Render Rectangle Commands
	    {0x60, "Monochrome Rectangle (variable size) (opaque)"},
	    {0x62, "Monochrome Rectangle (variable size) (semi-transparent)"},
	    {0x68, "Monochrome Rectangle (1x1) (Dot) (opaque)"},
	    {0x6A, "Monochrome Rectangle (1x1) (Dot) (semi-transparent)"},
	    {0x70, "Monochrome Rectangle (8x8) (opaque)"},
	    {0x72, "Monochrome Rectangle (8x8) (semi-transparent)"},
	    {0x78, "Monochrome Rectangle (16x16) (opaque)"},
	    {0x7A, "Monochrome Rectangle (16x16) (semi-transparent)"},
	    {0x64, "Textured Rectangle, variable size, opaque, texture-blending"},
	    {0x65, "Textured Rectangle, variable size, opaque, raw-texture"},
	    {0x66, "Textured Rectangle, variable size, semi-transp, texture-blending"},
	    {0x67, "Textured Rectangle, variable size, semi-transp, raw-texture"},
	    {0x6C, "Textured Rectangle, 1x1 (nonsense), opaque, texture-blending"},
	    {0x6D, "Textured Rectangle, 1x1 (nonsense), opaque, raw-texture"},
	    {0x6E, "Textured Rectangle, 1x1 (nonsense), semi-transp, texture-blending"},
	    {0x6F, "Textured Rectangle, 1x1 (nonsense), semi-transp, raw-texture"},
	    {0x74, "Textured Rectangle, 8x8, opaque, texture-blending"},
	    {0x75, "Textured Rectangle, 8x8, opaque, raw-texture"},
	    {0x76, "Textured Rectangle, 8x8, semi-transparent, texture-blending"},
	    {0x77, "Textured Rectangle, 8x8, semi-transparent, raw-texture"},
	    {0x7C, "Textured Rectangle, 16x16, opaque, texture-blending"},
	    {0x7D, "Textured Rectangle, 16x16, opaque, raw-texture"},
	    {0x7E, "Textured Rectangle, 16x16, semi-transparent, texture-blending"},
	    {0x7F, "Textured Rectangle, 16x16, semi-transparent, raw-texture"},

	    // GPU Rendering Attributes
	    {0xE1, "Draw Mode setting(aka \"Texpage\")"},
	    {0xE2, "Texture Window setting"},
	    {0xE3, "Set Drawing Area top left (X1,Y1)"},
	    {0xE4, "Set Drawing Area bottom right (X2,Y2)"},
	    {0xE5, "Set Drawing Offset (X,Y)"},
	    {0xE6, "Mask Bit Setting"},

	    // GPU Memory Transfer Commands
	    {0x01, "Clear Cache"},
	    {0x02, "Fill Rectangle in VRAM"},
	    {0x80, "Copy Rectangle (VRAM to VRAM)"},
	    {0xA0, "Copy Rectangle (CPU to VRAM)"},
	    {0xC0, "Copy Rectangle (VRAM to CPU)"},

	    // GPU Other Commands
	    {0x1F, "Interrupt Request (IRQ1)"},
	    {0x03, "Unknown?"},
	    {0x00, "NOP?"},

	    // undocumented
	    {0x35, "undocumented"},
	    {0x37, "undocumented"},
	    {0x3D, "undocumented"},
	    {0x3F, "undocumented"},
	    {0x21, "undocumented"},
	    {0x23, "undocumented"},
	    {0x29, "undocumented"},
	    {0x2B, "undocumented"},
	    {0x31, "undocumented"},
	    {0x33, "undocumented"},
	    {0x39, "undocumented"},
	    {0x3B, "undocumented"},
	};

	std::unordered_map<uint8_t, std::string> gp1_ins{
	    {0x00, "Reset GPU"},
	    {0x01, "Reset Command Buffer"},
	    {0x02, "Acknowledge GPU Interrupt (IRQ1)"},
	    {0x03, "Display Enable"},
	    {0x04, "DMA Direction / Data Request"},
	    {0x05, "Start of Display area (in VRAM)"},
	    {0x06, "Horizontal Display range (on Screen)"},
	    {0x07, "Vertical Display range (on Screen)"},
	    {0x08, "Display mode"},
	    {0x10, "Get GPU Info"},
	    {0x09, "New Texture Disable"},
	    {0x20, "Special/Prototype Texture Disable"},
	    {0x0B, "Unknown/Internal?"},
	};
}

namespace psycris::gpu {
	std::string disassembly(gpu_port port, uint32_t ins) {
		decoder dec{ins};
		uint8_t op = dec.opcode();

		std::string s;
		if (port == gpu_port::gp0) {
			auto it = gp0_ins.find(op);
			if (it != gp0_ins.end()) {
				s = it->second;
			}
		} else {
			auto it = gp1_ins.find(op);
			if (it != gp1_ins.end()) {
				s = it->second;
			}
		}

		if (s != "") {
			return s;
		}

		if (port == gpu_port::gp0) {
			if ((op >= 0x04 && op <= 0x1e) || op == 0xe0 || (op >= 0xe7 && op <= 0xef)) {
				s = "Mirror of GP0(00h) - NOP";
			} else if (op >= 0x81 && op <= 0x9f) {
				s = "Mirror of GP0(80h) - Copy Rectangle (VRAM to VRAM)";
			} else if (op >= 0xa1 && op <= 0xbf) {
				s = "Mirror of GP0(A0h) - Copy Rectangle (CPU to VRAM)";
			} else if (op >= 0xc1 && op <= 0xdf) {
				s = "Mirror of GP0(C0h) - Copy Rectangle (VRAM to CPU)";
			} else {
				s = "unknown";
			}
		} else {
			if (op >= 0x11 && op <= 0x1f) {
				s = "Mirrors of GP1(10h), Get GPU Info";
			} else {
				s = "unknown";
			}
		}

		return s;
	}
}