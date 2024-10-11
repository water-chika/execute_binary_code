#pragma once

#define NOMINMAX
#include <Windows.h>

#include <span>
#include <iostream>
#include <cassert>
#include <map>
#include <array>
#include <iomanip>
#include <cstdint>
#include <vector>

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
	binary_code(std::span<uint8_t> code) : m_mem{ create_mem(code) } {}
	template<class... ARGS>
	uint64_t execute(ARGS... args) {
		auto fun = static_cast<uint64_t(*)(ARGS...)>(m_mem.get_addr());
		return fun(args...);
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