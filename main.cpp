#include <execute_binary_code.hpp>

#include <set>
#include <map>

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

enum reg : uint8_t {
    RAX,RCX,RDX,RBX,RSP,RBP,RSI,RDI,
    EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI,
};

uint8_t modrm_reg_encode(reg r) {
    std::set<reg> table[8];
    table[0b000] = std::set<reg>{
        RAX,EAX
    };
    table[0b001] = std::set<reg>{
        RCX,ECX
    };
    table[0b010] = std::set<reg>{
        RDX,EDX
    };
    table[0b011] = std::set<reg>{
        RBX,EBX
    };
    table[0b100] = std::set<reg>{
        RSP,ESP
    };
    table[0b101] = std::set<reg>{
        RBP,EBP
    };
    table[0b110] = std::set<reg>{
        RSI,ESI
    };
    table[0b111] = std::set<reg>{
        RDI,EDI
    };
    std::map<reg, uint8_t> reg_map{};
    for (uint8_t code = 0; code < 8; code++) {
        for (auto r : table[code]) {
            reg_map.emplace(r, code);
        }
    }

    return reg_map[r];
}

uint8_t encode_operands(reg src, reg dst) {
    return 0b11'000'000 | (modrm_reg_encode(src) << 3) | modrm_reg_encode(dst);
}

#ifdef WIN32
int32_t add(int32_t lhs, int32_t rhs) {
	// MOV 89 /r
	auto codes = std::vector<uint8_t>{
		0x89, encode_operands(ECX, EAX), //0b11001000,
		0x01, encode_operands(EDX, EAX), //0b11010000,
		0xc3
	};
	return binary_code{ codes }.execute(lhs, rhs);
}
#endif
#ifdef linux
int32_t add(int32_t lhs, int32_t rhs) {
	// MOV 89 /r
	auto codes = std::vector<uint8_t>{
		0x89, encode_operands(EDI, EAX), //0b11'111'000,
        0x01, encode_operands(ESI, EAX), //0b11'110'000,
		0xc3
	};
	return binary_code{ codes }.execute(lhs, rhs);
}
#endif

bool test_little_endian() {
	uint64_t x = 0x0706050403020100;
	uint64_t* p_x = &x;
	auto codes = std::vector<uint8_t>{
				0x31, 0b11000000,
				0x8A, 0b00000001,
				0xc3
	};
	return binary_code{
			codes
		}.execute(p_x) == 0x00;
}

void add_test(uint64_t* lhs, uint64_t* rhs, uint64_t* res, size_t size) {
	auto codes = std::vector<uint8_t>{
		0b0100'1000, 0x8B, 0b00'000'001,
		0b0100'1000, 0x13, 0b00'000'010,
		0b0100'1001, 0x89, 0b00'000'000,
		0b0100'1000, 0xFF, 0b11'000'001,
		0b0100'1000, 0xFF, 0b11'000'001,
		0b0100'1000, 0xFF, 0b11'000'001,
		0b0100'1000, 0xFF, 0b11'000'001,
		0b0100'1000, 0xFF, 0b11'000'001,
		0b0100'1000, 0xFF, 0b11'000'001,
		0b0100'1000, 0xFF, 0b11'000'001,
		0b0100'1000, 0xFF, 0b11'000'001,
		0b0100'1000, 0xFF, 0b11'000'010,
		0b0100'1000, 0xFF, 0b11'000'010,
		0b0100'1000, 0xFF, 0b11'000'010,
		0b0100'1000, 0xFF, 0b11'000'010,
		0b0100'1000, 0xFF, 0b11'000'010,
		0b0100'1000, 0xFF, 0b11'000'010,
		0b0100'1000, 0xFF, 0b11'000'010,
		0b0100'1000, 0xFF, 0b11'000'010,
		0b0100'1001, 0xFF, 0b11'000'000,
		0b0100'1001, 0xFF, 0b11'000'000,
		0b0100'1001, 0xFF, 0b11'000'000,
		0b0100'1001, 0xFF, 0b11'000'000,
		0b0100'1001, 0xFF, 0b11'000'000,
		0b0100'1001, 0xFF, 0b11'000'000,
		0b0100'1001, 0xFF, 0b11'000'000,
		0b0100'1001, 0xFF, 0b11'000'000,
		0b0100'1001, 0xFF, 0b11'001'001,
		0x75, static_cast<uint8_t>(-86),
		0xc3
	};
	binary_code{ codes }.execute(lhs, rhs, res, size);
}

int main() {
	std::cout << std::hex << std::setw(64/4) << std::setfill('0') << add(1,3) << std::endl;
	std::cout << "little endian: " << test_little_endian() << std::endl;
	std::array<uint64_t, 9> x{ 0x10, 0x32, 0x54, 0x76, 0x98, 0xba, 0xdc, 0xfe, 0x32 };
	std::array<uint64_t, 9> y{ 0xffffffffffffffff, 0xff00, 0x1};
	std::array<uint64_t, 10> z{};
	add_test(x.data(), y.data(), z.data(), x.size());
	for (auto d : z) {
		std::cout << d << ' ';
	}
	std::cout << std::endl;
	return 0;
}
