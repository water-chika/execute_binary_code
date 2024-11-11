#pragma once

#include <span>
#include <iostream>
#include <cassert>
#include <map>
#include <array>
#include <iomanip>
#include <cstdint>
#include <vector>
#include <cstring>

#ifdef linux
#include "memory_linux.hpp"
#endif
#ifdef WIN32
#include "memory_windows.hpp"
#endif

class binary_code {
public:
	binary_code(std::span<uint8_t> code) : m_mem{ create_mem(code) } {}
	template<class... ARGS>
	uint64_t execute(ARGS... args) {
		auto fun = reinterpret_cast<uint64_t(*)(ARGS...)>(m_mem.get_addr());
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
