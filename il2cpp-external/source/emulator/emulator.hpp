#pragma once
#include <Windows.h>
#include <cstdio>
#include <array>
#include "types.hpp"
#include "state.hpp"
#include "decoder.hpp"
#include "handler.hpp"

#include "instructions/movement.hpp"
#include "instructions/floating.hpp"
#include "instructions/arithmetic.hpp"
#include "instructions/flow.hpp"
#include "instructions/avx.hpp"

using external_call_fn = std::function< bool ( std::uint64_t address, cpu_state& cpu, const memory_handler& mem ) >;

inline external_call_fn g_external_call_handler;

class emulator
{
public:
	using call_trace_fn = std::function< void ( std::uint64_t target, std::uint64_t call_site ) >;

	emulator ( memory_read_fn read_callback, memory_write_fn write_callback );
	~emulator ( ) = default;

	auto step ( std::uint64_t address ) -> bool;
	auto run ( std::uint64_t start_address, std::uint64_t end_address, std::size_t max_instructions = 10000 ) -> bool;
	auto reset ( ) -> void;
	auto get_cpu ( ) -> cpu_state&;
	auto get_cpu ( ) const -> const cpu_state&;
	auto get_memory ( ) -> memory_handler&;
	auto get_memory ( ) const -> const memory_handler&;
	auto set_debug ( bool value ) -> void;
	auto set_module_base ( std::uint64_t base ) -> void;
	auto get_module_base ( ) const -> std::uint64_t;
	auto set_call_trace_hook ( call_trace_fn fn ) -> void;

private:
	auto execute ( const instruction& instr ) -> bool;
	auto fetch_instruction ( std::uint64_t address, std::uint8_t* buffer, std::size_t max_size ) -> bool;
	auto is_control_flow_instruction ( instruction_type type ) const -> bool;
	auto push_trace ( std::uint64_t rip, const std::uint8_t* bytes, std::uint8_t length, std::uint16_t type ) -> void;
	auto dump_trace ( ) -> void;

	template< typename... Args >
	auto log ( const char* fmt, Args... args ) -> void
	{
		if ( !this->m_debug )
		{
			return;
		}

		std::printf ( fmt, args... );
	}

	struct trace_entry
	{
		std::uint64_t rip;
		std::uint8_t bytes[ 8 ];
		std::uint8_t length;
		std::uint16_t type;
	};

	static constexpr std::size_t trace_capacity = 32;

	std::uint64_t m_base;
	cpu_state m_cpu;
	memory_handler m_memory;
	decoder m_decoder;
	bool m_should_stop;
	bool m_debug;
	call_trace_fn m_call_trace_hook;
	std::array< trace_entry, trace_capacity > m_trace;
	std::size_t m_trace_head;
	std::size_t m_trace_count;
};

inline emulator::emulator ( memory_read_fn read_callback, memory_write_fn write_callback )
	: m_base ( 0 ), m_memory ( read_callback, write_callback ), m_should_stop ( false ), m_debug ( false ),
	m_trace ( { } ), m_trace_head ( 0 ), m_trace_count ( 0 )
{ }

inline auto emulator::set_debug ( bool value ) -> void
{
	this->m_debug = value;
}

inline auto emulator::get_cpu ( ) -> cpu_state&
{
	return this->m_cpu;
}

inline auto emulator::get_cpu ( ) const -> const cpu_state&
{
	return this->m_cpu;
}

inline auto emulator::get_memory ( ) -> memory_handler&
{
	return this->m_memory;
}

inline auto emulator::get_memory ( ) const -> const memory_handler&
{
	return this->m_memory;
}

inline auto emulator::set_module_base ( std::uint64_t base ) -> void
{
	this->m_base = base;
}

inline auto emulator::get_module_base ( ) const -> std::uint64_t
{
	return this->m_base;
}

inline auto emulator::set_call_trace_hook ( call_trace_fn fn ) -> void
{
	this->m_call_trace_hook = std::move ( fn );
}

inline auto emulator::reset ( ) -> void
{
	this->m_cpu.reset ( );
	this->m_should_stop = false;
	this->m_trace_head = 0;
	this->m_trace_count = 0;
}

inline auto emulator::fetch_instruction ( std::uint64_t address, std::uint8_t* buffer, std::size_t max_size ) -> bool
{
	return this->m_memory.read ( address, buffer, max_size );
}

inline auto emulator::push_trace ( std::uint64_t rip, const std::uint8_t* bytes, std::uint8_t length, std::uint16_t type ) -> void
{
	auto& slot = this->m_trace[ this->m_trace_head ];
	slot.rip = rip;
	slot.length = length;
	slot.type = type;

	for ( std::size_t i = 0; i < 8; i++ )
	{
		slot.bytes[ i ] = bytes[ i ];
	}

	this->m_trace_head = ( this->m_trace_head + 1 ) % trace_capacity;

	if ( this->m_trace_count < trace_capacity )
	{
		this->m_trace_count++;
	}
}

inline auto emulator::dump_trace ( ) -> void
{
	if ( this->m_trace_count == 0 )
	{
		return;
	}

	const auto start = ( this->m_trace_head + trace_capacity - this->m_trace_count ) % trace_capacity;

	this->log ( "  --- last %zu instructions ---\n", this->m_trace_count );

	for ( std::size_t i = 0; i < this->m_trace_count; i++ )
	{
		const auto& entry = this->m_trace[ ( start + i ) % trace_capacity ];
		this->log ( "  [-%2zu] rip=0x%llx (ida: 0x%llx) type=%u len=%u  %02x %02x %02x %02x %02x %02x %02x %02x\n",
					this->m_trace_count - i,
					static_cast< unsigned long long > ( entry.rip ),
					static_cast< unsigned long long > ( 0x180000000 + entry.rip - this->m_base ),
					static_cast< unsigned > ( entry.type ),
					static_cast< unsigned > ( entry.length ),
					entry.bytes[ 0 ], entry.bytes[ 1 ], entry.bytes[ 2 ], entry.bytes[ 3 ],
					entry.bytes[ 4 ], entry.bytes[ 5 ], entry.bytes[ 6 ], entry.bytes[ 7 ] );
	}
}

inline auto emulator::is_control_flow_instruction ( instruction_type type ) const -> bool
{
	switch ( type )
	{
		case instruction_type::jmp:
		case instruction_type::call:
		case instruction_type::ret:
		case instruction_type::syscall:
		case instruction_type::je:
		case instruction_type::jne:
		case instruction_type::jg:
		case instruction_type::jge:
		case instruction_type::jl:
		case instruction_type::jle:
		case instruction_type::ja:
		case instruction_type::jae:
		case instruction_type::jb:
		case instruction_type::jbe:
		case instruction_type::jz:
		case instruction_type::jnz:
		case instruction_type::js:
		case instruction_type::jns:
		case instruction_type::jo:
		case instruction_type::jno:
		case instruction_type::jp:
		case instruction_type::jnp:
		case instruction_type::jcxz:
		case instruction_type::jecxz:
		case instruction_type::jrcxz:
			return true;

		default:
			return false;
	}
}

inline auto emulator::execute ( const instruction& instr ) -> bool
{
	switch ( instr.type )
	{
		case instruction_type::mov:
		{
			return instructions::mov::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::movzx:
		{
			return instructions::movzx::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::movsx:
		case instruction_type::movsxd:
		{
			return instructions::movsx::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::lea:
		{
			return instructions::lea::execute ( this->m_cpu, instr );
		}

		case instruction_type::push:
		{
			return instructions::push::execute_r64 ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::pushfq:
		{
			return instructions::pushfq::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::pop:
		{
			return instructions::pop::execute_r64 ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::xchg:
		{
			return instructions::xchg::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::add:
		{
			return instructions::add::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::adc:
		{
			return instructions::adc::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::sub:
		{
			return instructions::sub::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::sbb:
		{
			return instructions::sbb::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::mul:
		{
			return instructions::mul::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::imul:
		{
			return instructions::imul::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::div:
		{
			return instructions::div::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::idiv:
		{
			return instructions::idiv::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::inc:
		{
			return instructions::inc::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::dec:
		{
			return instructions::dec::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::neg:
		{
			return instructions::neg::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::cmp:
		{
			return instructions::cmp::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::test:
		{
			return instructions::test::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::and_instr:
		{
			return instructions::and_instr::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::or_instr:
		{
			return instructions::or_instr::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::xor_instr:
		{
			return instructions::xor_instr::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::not_instr:
		{
			return instructions::not_instr::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::shl:
		{
			return instructions::shl::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::shr:
		{
			return instructions::shr::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::sar:
		{
			return instructions::sar::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::rol:
		{
			return instructions::rol::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::ror:
		{
			return instructions::ror::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::rcl:
		{
			return instructions::rcl::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::rcr:
		{
			return instructions::rcr::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::bt:
		{
			return instructions::bt::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::bts:
		{
			return instructions::bts::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::btr:
		{
			return instructions::btr::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::btc:
		{
			return instructions::btc::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::bswap:
		{
			return instructions::bswap::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::bsf:
		{
			return instructions::bsf::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::bsr:
		{
			return instructions::bsr::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::call:
		{
			if ( !instructions::call::execute ( this->m_cpu, this->m_memory, instr ) )
			{
				return false;
			}

			if ( this->m_call_trace_hook )
			{
				this->m_call_trace_hook ( this->m_cpu.rip, instr.address );
			}

			return true;
		}

		case instruction_type::stmxcsr:
		{
			const auto address = this->m_cpu.calculate_memory_address ( instr.operands[ 0 ].mem, instr.address + instr.length );
			const std::uint32_t mxcsr = 0x00001F80;
			if ( !this->m_memory.write ( address, &mxcsr, 4 ) )
			{
				return false;
			}
			this->m_cpu.rip = instr.address + instr.length;
			return true;
		}

		case instruction_type::fstcw:
		{
			const auto address = this->m_cpu.calculate_memory_address ( instr.operands[ 0 ].mem, instr.address + instr.length );
			const std::uint16_t cw = 0x037F;
			if ( !this->m_memory.write ( address, &cw, 2 ) )
			{
				return false;
			}
			this->m_cpu.rip = instr.address + instr.length;
			return true;
		}

		case instruction_type::ret:
		{
			auto rsp = this->m_cpu.read_gpr ( 4 );
			std::uint64_t return_address = 0;


			if ( !this->m_memory.read ( rsp, &return_address, sizeof ( std::uint64_t ) ) )
			{
				return false;
			}

			rsp += 8;
			this->m_cpu.write_gpr ( 4, rsp );
			this->m_cpu.rip = return_address;
			return true;
		}

		case instruction_type::jmp:
		{
			return instructions::jmp::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::je:
		case instruction_type::jne:
		case instruction_type::jg:
		case instruction_type::jge:
		case instruction_type::jl:
		case instruction_type::jle:
		case instruction_type::ja:
		case instruction_type::jae:
		case instruction_type::jb:
		case instruction_type::jbe:
		case instruction_type::jz:
		case instruction_type::jnz:
		case instruction_type::js:
		case instruction_type::jns:
		case instruction_type::jo:
		case instruction_type::jno:
		case instruction_type::jp:
		case instruction_type::jnp:
		{
			return instructions::jcc::execute ( this->m_cpu, instr );
		}

		case instruction_type::jcxz:
		case instruction_type::jecxz:
		case instruction_type::jrcxz:
		{
			return instructions::jcxz::execute ( this->m_cpu, instr );
		}

		case instruction_type::cmove:
		case instruction_type::cmovne:
		case instruction_type::cmovg:
		case instruction_type::cmovge:
		case instruction_type::cmovl:
		case instruction_type::cmovle:
		case instruction_type::cmova:
		case instruction_type::cmovae:
		case instruction_type::cmovb:
		case instruction_type::cmovbe:
		case instruction_type::cmovs:
		case instruction_type::cmovns:
		case instruction_type::cmovo:
		case instruction_type::cmovno:
		{
			return instructions::cmov::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::sete:
		case instruction_type::setne:
		case instruction_type::setg:
		case instruction_type::setge:
		case instruction_type::setl:
		case instruction_type::setle:
		case instruction_type::seta:
		case instruction_type::setae:
		case instruction_type::setb:
		case instruction_type::setbe:
		case instruction_type::sets:
		case instruction_type::setns:
		case instruction_type::seto:
		case instruction_type::setno:
		{
			return instructions::setcc::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::movss:
		{
			return instructions::sse::movss ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::addss:
		{
			return instructions::sse::addss ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::subss:
		{
			return instructions::sse::subss ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::mulss:
		{
			return instructions::sse::mulss ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::divss:
		{
			return instructions::sse::divss ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::sqrtss:
		{
			return instructions::sse::sqrtss ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::movsd:
		{
			return instructions::sse::movsd ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::addsd:
		{
			return instructions::sse::addsd ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::subsd:
		{
			return instructions::sse::subsd ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::mulsd:
		{
			return instructions::sse::mulsd ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::divsd:
		{
			return instructions::sse::divsd ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::sqrtsd:
		{
			return instructions::sse::sqrtsd ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::cvtsi2ss:
		{
			return instructions::sse::cvtsi2ss ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::cvtsi2sd:
		{
			return instructions::sse::cvtsi2sd ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::cvtpd2ps:
		{
			return instructions::sse::cvtpd2ps ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::cvtps2pd:
		{
			return instructions::sse::cvtps2pd ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::cvtss2sd:
		{
			return instructions::sse::cvtss2sd ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::cvtsd2ss:
		{
			return instructions::sse::cvtsd2ss ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::vcvtss2sd:
		{
			return instructions::sse::vcvtss2sd ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::vcvtsd2ss:
		{
			return instructions::sse::vcvtsd2ss ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::cvttss2si:
		{
			return instructions::sse::cvttss2si ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::cvttsd2si:
		{
			return instructions::sse::cvttsd2si ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::movaps:
		{
			return instructions::sse::movaps ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::movups:
		{
			return instructions::sse::movups ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::movapd:
		{
			return instructions::sse::movapd ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::movupd:
		{
			return instructions::sse::movupd ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::movdqa:
		{
			return instructions::sse::movdqa ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::movdqu:
		{
			return instructions::sse::movdqu ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::movq:
		{
			return instructions::sse::movq ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::movlps:
		{
			return instructions::sse::movlps ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::movhps:
		{
			return instructions::sse::movhps ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::shufps:
		{
			return instructions::sse::shufps ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::shufpd:
		{
			return instructions::sse::shufpd ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::cmpps:
		{
			return instructions::sse::cmpps ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::cmppd:
		{
			return instructions::sse::cmppd ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::cmpss:
		{
			return instructions::sse::cmpss ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::cmpsd:
		{
			return instructions::sse::cmpsd ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::movmskps:
		{
			return instructions::sse::movmskps ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::movmskpd:
		{
			return instructions::sse::movmskpd ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::pmovmskb:
		{
			return instructions::sse::pmovmskb ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::unpcklps:
		{
			return instructions::sse::unpcklps ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::unpckhps:
		{
			return instructions::sse::unpckhps ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::xorps:
		{
			return instructions::sse::xorps ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::xorpd:
		{
			return instructions::sse::xorpd ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::andps:
		{
			return instructions::sse::andps ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::andpd:
		{
			return instructions::sse::andpd ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::addps:
		{
			return instructions::sse::addps ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::subps:
		{
			return instructions::sse::subps ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::mulps:
		{
			return instructions::sse::mulps ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::divps:
		{
			return instructions::sse::divps ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::rcpps:
		{
			return instructions::sse::rcpps ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::rcpss:
		{
			return instructions::sse::rcpss ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::pshufd:
		{
			return instructions::sse::pshufd ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::psrldq:
		{
			return instructions::sse::psrldq ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::pslldq:
		{
			return instructions::sse::pslldq ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::psrlq:
		{
			return instructions::sse::psrlq ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::psllq:
		{
			return instructions::sse::psllq ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::psrld:
		{
			return instructions::sse::psrld ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::pslld:
		{
			return instructions::sse::pslld ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::psrlw:
		{
			return instructions::sse::psrlw ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::psllw:
		{
			return instructions::sse::psllw ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::psrad:
		{
			return instructions::sse::psrad ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::psraw:
		{
			return instructions::sse::psraw ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::cvtdq2ps:
		{
			return instructions::sse::cvtdq2ps ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::cvtps2dq:
		{
			return instructions::sse::cvtps2dq ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::cvttps2dq:
		{
			return instructions::sse::cvttps2dq ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::minss:
		{
			return instructions::sse::minss ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::minsd:
		{
			return instructions::sse::minsd ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::minps:
		{
			return instructions::sse::minps ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::minpd:
		{
			return instructions::sse::minpd ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::maxss:
		{
			return instructions::sse::maxss ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::maxsd:
		{
			return instructions::sse::maxsd ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::maxps:
		{
			return instructions::sse::maxps ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::maxpd:
		{
			return instructions::sse::maxpd ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::andnps:
		{
			return instructions::sse::andnps ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::andnpd:
		{
			return instructions::sse::andnpd ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::movd:
		{
			return instructions::sse::movd ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::pxor:
		{
			return instructions::sse::pxor ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::pand:
		{
			return instructions::sse::pand ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::por:
		{
			return instructions::sse::por ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::punpcklbw:
		{
			return instructions::sse::punpcklbw ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::punpcklwd:
		{
			return instructions::sse::punpcklwd ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::punpckldq:
		{
			return instructions::sse::punpckldq ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::punpcklqdq:
		{
			return instructions::sse::punpcklqdq ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::punpckhbw:
		{
			return instructions::sse::punpckhbw ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::punpckhwd:
		{
			return instructions::sse::punpckhwd ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::punpckhdq:
		{
			return instructions::sse::punpckhdq ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::punpckhqdq:
		{
			return instructions::sse::punpckhqdq ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::packsswb:
		{
			return instructions::sse::packsswb ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::packuswb:
		{
			return instructions::sse::packuswb ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::packssdw:
		{
			return instructions::sse::packssdw ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::pcmpgtb:
		{
			return instructions::sse::pcmpgtb ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::pcmpgtw:
		{
			return instructions::sse::pcmpgtw ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::pcmpgtd:
		{
			return instructions::sse::pcmpgtd ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::ucomiss:
		{
			return instructions::sse::ucomiss ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::comiss:
		{
			return instructions::sse::comiss ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::ucomisd:
		{
			return instructions::sse::ucomisd ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::comisd:
		{
			return instructions::sse::comisd ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::orps:
		{
			return instructions::sse::orps ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::orpd:
		{
			return instructions::sse::orpd ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::nop:
		{
			this->m_cpu.rip = instr.address + instr.length;
			return true;
		}

		case instruction_type::rdtsc:
		{
			return instructions::rdtsc::execute ( this->m_cpu, instr );
		}

		case instruction_type::rdtscp:
		{
			return instructions::rdtscp::execute ( this->m_cpu, instr );
		}

		case instruction_type::cpuid:
		{
			return instructions::cpuid::execute ( this->m_cpu, instr );
		}

		case instruction_type::cdq:
		{
			return instructions::cdq::execute ( this->m_cpu, instr );
		}

		case instruction_type::cqo:
		{
			return instructions::cqo::execute ( this->m_cpu, instr );
		}

		case instruction_type::cbw:
		{
			return instructions::cbw::execute ( this->m_cpu, instr );
		}

		case instruction_type::cwde:
		{
			return instructions::cwde::execute ( this->m_cpu, instr );
		}

		case instruction_type::cdqe:
		{
			return instructions::cdqe::execute ( this->m_cpu, instr );
		}

		case instruction_type::cmpxchg:
		{
			return instructions::cmpxchg::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::cmpxchg16b:
		{
			return instructions::cmpxchg16b::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::xadd:
		{
			return instructions::xadd::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::vzeroupper:
		{
			return instructions::vzeroupper::execute ( this->m_cpu, instr );
		}

		case instruction_type::vinsertf128:
		{
			return instructions::vinsertf128::execute ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::vpxor:
		{
			return instructions::avx256::vpxor ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::vpshufb:
		{
			return instructions::avx256::vpshufb ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::vpcmpeqb:
		{
			return instructions::avx256::vpcmpeqb ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::vpcmpeqw:
		{
			return instructions::avx256::vpcmpeqw ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::vpcmpeqd:
		{
			return instructions::avx256::vpcmpeqd ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::vpmovmskb:
		{
			return instructions::avx256::vpmovmskb ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::vmovdqu256:
		case instruction_type::vmovdqa256:
		case instruction_type::vmovups256:
		case instruction_type::vmovupd256:
		case instruction_type::vmovaps256:
		case instruction_type::vmovapd256:
		{
			return instructions::avx256::execute_move ( this->m_cpu, this->m_memory, instr );
		}

		case instruction_type::syscall:
		{
			this->m_cpu.gpr[ 0 ] = 0;
			this->m_cpu.rip = instr.address + instr.length;
			return true;
		}

		default:
		{
			std::printf ( "[emu] unhandled type=%d\n", static_cast< int >( instr.type ) );
			return false;
		}
	}
}

inline auto emulator::step ( std::uint64_t address ) -> bool
{
	this->m_memory.clear_fault ( );

	if ( g_external_call_handler && g_external_call_handler ( address, this->m_cpu, this->m_memory ) )
	{
		return true;
	}

	std::uint8_t instruction_bytes[ 15 ];

	if ( !this->fetch_instruction ( address, instruction_bytes, sizeof ( instruction_bytes ) ) )
	{
		this->log ( "[emu] FAIL fetch rip=0x%llx\n", static_cast< unsigned long long > ( address ) );
		return false;
	}

	instruction instr;

	if ( !this->m_decoder.decode ( instruction_bytes, sizeof ( instruction_bytes ), instr ) )
	{
		this->log ( "[emu] FAIL decode rip=0x%llx (ida: 0x%llx) bytes=%02x %02x %02x %02x %02x %02x %02x %02x\n",
					  static_cast< unsigned long long > ( address ),
					  static_cast< unsigned long long > ( 0x180000000 + address - this->m_base ),
					  instruction_bytes[ 0 ], instruction_bytes[ 1 ], instruction_bytes[ 2 ], instruction_bytes[ 3 ],
					  instruction_bytes[ 4 ], instruction_bytes[ 5 ], instruction_bytes[ 6 ], instruction_bytes[ 7 ] );
		return false;
	}

	instr.address = address;

	if ( instr.length == 0 || instr.length > 15 )
	{
		this->log ( "[emu] FAIL invalid length rip=0x%llx (ida: 0x%llx) len=%u bytes=%02x %02x %02x %02x %02x\n",
					static_cast< unsigned long long > ( address ),
					static_cast< unsigned long long > ( 0x180000000 + address - this->m_base ),
					static_cast< unsigned > ( instr.length ),
					instruction_bytes[ 0 ], instruction_bytes[ 1 ],
					instruction_bytes[ 2 ], instruction_bytes[ 3 ],
					instruction_bytes[ 4 ] );
		return false;
	}

	this->log ( "[emu] rip=0x%llx (ida: 0x%llx) type=%d len=%u\n",
				static_cast< unsigned long long > ( address ),
				static_cast< unsigned long long > ( 0x180000000 + address - this->m_base ),
				static_cast< int > ( instr.type ),
				static_cast< unsigned > ( instr.length ) );

	if ( !this->execute ( instr ) )
	{
		this->log ( "[emu] FAIL execute rip=0x%llx (ida: 0x%llx) type=%d bytes=%02x %02x %02x %02x %02x %02x %02x %02x fault_addr=0x%llx (%s)\n",
					  static_cast< unsigned long long > ( address ),
					  static_cast< unsigned long long > ( 0x180000000 + address - this->m_base ),
					  static_cast< int > ( instr.type ),
					  instruction_bytes[ 0 ], instruction_bytes[ 1 ], instruction_bytes[ 2 ], instruction_bytes[ 3 ],
					  instruction_bytes[ 4 ], instruction_bytes[ 5 ], instruction_bytes[ 6 ], instruction_bytes[ 7 ],
					  static_cast< unsigned long long > ( this->m_memory.get_last_fault_address ( ) ),
					  this->m_memory.get_last_fault_was_write ( ) ? "write" : "read" );

		this->log ( "  rax=%016llx rcx=%016llx rdx=%016llx rbx=%016llx\n",
					  static_cast< unsigned long long > ( this->m_cpu.read_gpr ( 0 ) ),
					  static_cast< unsigned long long > ( this->m_cpu.read_gpr ( 1 ) ),
					  static_cast< unsigned long long > ( this->m_cpu.read_gpr ( 2 ) ),
					  static_cast< unsigned long long > ( this->m_cpu.read_gpr ( 3 ) ) );
		this->log ( "  rsp=%016llx rbp=%016llx rsi=%016llx rdi=%016llx\n",
					  static_cast< unsigned long long > ( this->m_cpu.read_gpr ( 4 ) ),
					  static_cast< unsigned long long > ( this->m_cpu.read_gpr ( 5 ) ),
					  static_cast< unsigned long long > ( this->m_cpu.read_gpr ( 6 ) ),
					  static_cast< unsigned long long > ( this->m_cpu.read_gpr ( 7 ) ) );
		this->log ( "   r8=%016llx  r9=%016llx r10=%016llx r11=%016llx\n",
					  static_cast< unsigned long long > ( this->m_cpu.read_gpr ( 8 ) ),
					  static_cast< unsigned long long > ( this->m_cpu.read_gpr ( 9 ) ),
					  static_cast< unsigned long long > ( this->m_cpu.read_gpr ( 10 ) ),
					  static_cast< unsigned long long > ( this->m_cpu.read_gpr ( 11 ) ) );
		this->log ( "  r12=%016llx r13=%016llx r14=%016llx r15=%016llx\n",
					  static_cast< unsigned long long > ( this->m_cpu.read_gpr ( 12 ) ),
					  static_cast< unsigned long long > ( this->m_cpu.read_gpr ( 13 ) ),
					  static_cast< unsigned long long > ( this->m_cpu.read_gpr ( 14 ) ),
					  static_cast< unsigned long long > ( this->m_cpu.read_gpr ( 15 ) ) );
		this->dump_trace ( );
		return false;
	}

	this->push_trace ( address, instruction_bytes, instr.length, static_cast< std::uint16_t > ( instr.type ) );

	if ( !this->is_control_flow_instruction ( instr.type ) )
	{
		this->m_cpu.instr = instr;
		this->m_cpu.rip = address + instr.length;
	}

	return true;
}

inline auto emulator::run ( std::uint64_t start_address, std::uint64_t end_address, std::size_t max_instructions ) -> bool
{
	this->m_cpu.rip = start_address;
	this->m_should_stop = false;

	std::size_t instruction_count = 0;

	while ( !this->m_should_stop && instruction_count < max_instructions )
	{
		if ( this->m_cpu.rip == end_address )
		{
			this->log ( "[emu] run OK at end_address after %zu instructions\n", instruction_count );
			return true;
		}

		if ( this->m_cpu.rip == 0 )
		{
			this->log ( "[emu] run FAIL null rip at instruction_count=%zu\n", instruction_count );
			return false;
		}

		if ( !this->step ( this->m_cpu.rip ) )
		{
			this->log ( "[emu] run FAIL at instruction_count=%zu rip=0x%llx\n",
						instruction_count, static_cast< unsigned long long > ( this->m_cpu.rip ) );
			return false;
		}

		instruction_count++;
	}

	if ( this->m_cpu.rip != end_address )
	{
		this->log ( "[emu] run FAIL budget exhausted after %zu instructions, rip=0x%llx (ida: 0x%llx) stopped=%d\n",
					instruction_count,
					static_cast< unsigned long long > ( this->m_cpu.rip ),
					static_cast< unsigned long long > ( 0x180000000 + this->m_cpu.rip - this->m_base ),
					this->m_should_stop ? 1 : 0 );
	}

	return this->m_cpu.rip == end_address;
}