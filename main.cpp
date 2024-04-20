#include <Windows.h>

#include <span>
#include <iostream>
#include <cassert>
#include <map>
#include <array>
#include <iomanip>
#include <cstdint>

enum class page_protect {
	eReadWrite,
	eExecute,
};

class virtual_memory {
public:
	virtual_memory(size_t size, page_protect protect) : m_addr{ alloc(size, protect) }, m_size{ size } {}
	~virtual_memory() {
		if (m_addr != nullptr) {
			free(m_addr);
			m_addr = nullptr;
		}
	}
	bool set_protect(page_protect protect) {
		DWORD old;
		return VirtualProtect(m_addr, m_size, to_win32_const(protect), &old);
	}
	void* get_addr() {
		return m_addr;
	}
private:
	static void* alloc(size_t size, page_protect protect) {
		void* addr = VirtualAlloc(NULL, size, MEM_COMMIT, to_win32_const(protect));
		if (addr == NULL) {
			throw std::runtime_error("VirtualAlloc fail!");
		}
		return addr;
	}
	static void free(void* addr) {
		VirtualFree(addr, 0, MEM_RELEASE);
	}
	static DWORD to_win32_const(page_protect protect) {
		std::map<page_protect, DWORD> page_protect_enum_map{
			{page_protect::eReadWrite, PAGE_READWRITE},
			{page_protect::eExecute, PAGE_EXECUTE}
		};
		return page_protect_enum_map[protect];
	}
	void* m_addr;
	size_t m_size;
};

class binary_code {
public:
	binary_code(std::span<uint8_t> code) : m_mem{create_mem(code)} {}
	uint64_t execute() {
		auto fun = static_cast<uint64_t (*)()>(m_mem.get_addr());
		return fun();
	}
private:
	static virtual_memory create_mem(std::span<uint8_t> code) {
		virtual_memory mem{ code.size_bytes(), page_protect::eReadWrite };
		void* addr = mem.get_addr();
		memcpy(addr, code.data(), code.size_bytes());
		bool ret = mem.set_protect(page_protect::eExecute);
		assert(ret == true);
		return mem;
	}
	virtual_memory m_mem;
};

namespace amd64 {
	enum class operation {
		XOR,
		RET,
	};

	namespace reg {
		class AL {};
	}
	enum class reg32 {
		EAX
	};
	struct imm8 {
		uint8_t value;
	};

	template<uint32_t SIZE>
	class opcode {
	public:
		std::array<uint8_t, SIZE> codes;
	};
	struct none {};

	template<operation OPERATION, class DEST, class SRC>
	class mnemonic {

	};

	template<>
	class mnemonic<operation::XOR, reg::AL, imm8> {
	public:
		imm8 imm;
		opcode<2> get_opcode() {
			return { 0x34, imm.value };
		}
	};
	template<>
	class mnemonic<operation::XOR, reg32, reg32> {
	public:
		reg32 dst;
		reg32 src;
		opcode<2> get_opcode() {
			return { 0x33, 0xc0 };
		}
	};
	template<>
	class mnemonic<operation::RET, none, none> {
	public:
		uint8_t get_opcode() {
			return 0xc3;
		}
	};
}

int main() {
	using namespace amd64;
	auto opcode = mnemonic<operation::XOR, reg32, reg32>{reg32::EAX, reg32::EAX}.get_opcode();
	std::array<uint8_t,3> codes = { opcode.codes[0], opcode.codes[1], mnemonic<operation::RET, none, none>{}.get_opcode()}; // 0xc3 is x86 instruction: RET ---- return from a function call.
	binary_code code{ std::span<uint8_t>{codes.data(), codes.size()}};
	std::cout << std::hex << std::setw(64/4) << std::setfill('0') << code.execute() << std::endl;
	return 0;
}