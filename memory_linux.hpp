#pragma once

#include <cstdint>
#include <cassert>
#include <cstdio>

#include <sys/mman.h>
#include <unistd.h>

enum class page_protect {
	eReadWrite,
	eExecute,
};

class virtual_memory {
public:
	virtual_memory(size_t size, page_protect protect) : m_tmp{tmpfile()}, m_size{ size } {
        if (m_tmp == nullptr) {
            throw std::runtime_error{"failed to create temp file"};
        }
        int fd = fileno(m_tmp);
        uint8_t* buf = new uint8_t[m_size];
        int count = write(fd, buf, m_size);
        delete[] buf;
        if (count != m_size) {
            throw std::runtime_error{"failed to write fiel"};
        }
        m_addr = mmap(nullptr, m_size, to_linux_const(protect), MAP_PRIVATE, fd, 0);
        if (m_addr == MAP_FAILED) {
            throw std::runtime_error{"failed to map file"};
        }
    }
	~virtual_memory() {
		if (m_addr != nullptr) {
            munmap(m_addr, m_size);
            fclose(m_tmp);
			m_addr = nullptr;
		}
	}
	bool set_protect(page_protect protect) {
        int ret = mprotect(m_addr, m_size, to_linux_const(protect));
        return ret == 0;
	}
	void* get_addr() {
		return m_addr;
	}
private:
	static int to_linux_const(page_protect protect) {
		std::map<page_protect, int> page_protect_enum_map{
			{page_protect::eReadWrite, PROT_READ | PROT_WRITE},
			{page_protect::eExecute, PROT_EXEC}
		};
		return page_protect_enum_map[protect];
	}
    FILE* m_tmp;
	void* m_addr;
	size_t m_size;
};
