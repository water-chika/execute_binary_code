#pragma once

#define NOMINMAX
#include <Windows.h>

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
