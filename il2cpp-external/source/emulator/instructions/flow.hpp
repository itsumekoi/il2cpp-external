#pragma once
#include <Windows.h>
#include <cstdio>
#include <array>
#include "../state.hpp"
#include "../handler.hpp"
#include "../decoder.hpp"

namespace instructions
{
	namespace jcc
	{
		inline auto should_jump ( const cpu_state& cpu, instruction_type type ) -> bool
		{
			const auto zf = cpu.get_flag ( cpu_state::flag_zf );
			const auto sf = cpu.get_flag ( cpu_state::flag_sf );
			const auto of = cpu.get_flag ( cpu_state::flag_of );
			const auto cf = cpu.get_flag ( cpu_state::flag_cf );
			const auto pf = cpu.get_flag ( cpu_state::flag_pf );

			switch ( type )
			{

				case instruction_type::je:
				case instruction_type::jz:
				{
					return zf;
				}

				case instruction_type::jne:
				case instruction_type::jnz:
				{
					return !zf;
				}

				case instruction_type::jg:
				{
					return !zf && ( sf == of );
				}

				case instruction_type::jge:
				{
					return sf == of;
				}

				case instruction_type::jl:
				{
					return sf != of;
				}

				case instruction_type::jle:
				{
					return zf || ( sf != of );
				}

				case instruction_type::ja:
				{
					return !cf && !zf;
				}

				case instruction_type::jae:
				{
					return !cf;
				}

				case instruction_type::jb:
				{
					return cf;
				}

				case instruction_type::jbe:
				{
					return cf || zf;
				}

				case instruction_type::js:
				{
					return sf;
				}

				case instruction_type::jns:
				{
					return !sf;
				}

				case instruction_type::jo:
				{
					return of;
				}

				case instruction_type::jno:
				{
					return !of;
				}

				case instruction_type::jp:
				{
					return pf;
				}

				case instruction_type::jnp:
				{
					return !pf;
				}

				default:
				{
					return false;
				}
			}
		}

		inline auto execute ( cpu_state& cpu, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 1 || instr.operands[ 0 ].type != operand_type::imm )
			{
				return false;
			}

			const auto rel_offset = static_cast< std::int64_t > ( instr.operands[ 0 ].imm );

			const auto next_instr = instr.address + instr.length;

			if ( instructions::jcc::should_jump ( cpu, instr.type ) )
			{

				const auto target = next_instr + rel_offset;
				cpu.rip = target;
			}
			else
			{

				cpu.rip = next_instr;
			}

			return true;
		}
	}

	namespace jcxz
	{
		inline auto execute ( cpu_state& cpu, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 1 || instr.operands[ 0 ].type != operand_type::imm )
			{
				return false;
			}

			const auto rcx = cpu.read_gpr ( 1 );
			bool is_zero = false;

			switch ( instr.type )
			{
				case instruction_type::jcxz:
				{
					is_zero = ( rcx & 0xFFFF ) == 0;
					break;
				}

				case instruction_type::jecxz:
				{
					is_zero = ( rcx & 0xFFFFFFFF ) == 0;
					break;
				}

				case instruction_type::jrcxz:
				{
					is_zero = rcx == 0;
					break;
				}

				default:
				{
					return false;
				}
			}

			const auto rel_offset = static_cast< std::int64_t >( instr.operands[ 0 ].imm );
			const auto next_instr = instr.address + instr.length;

			if ( is_zero )
			{
				cpu.rip = next_instr + rel_offset;
			}
			else
			{
				cpu.rip = next_instr;
			}

			return true;
		}
	}

	namespace call
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 1 )
			{
				return false;
			}

			std::uint64_t target = 0;

			if ( instr.operands[ 0 ].type == operand_type::imm )
			{

				const auto rel_offset = static_cast< std::int64_t > ( instr.operands[ 0 ].imm );
				target = ( instr.address + instr.length ) + rel_offset;
			}
			else if ( instr.operands[ 0 ].type == operand_type::reg )
			{

				target = cpu.read_gpr ( instr.operands[ 0 ].reg );
			}
			else if ( instr.operands[ 0 ].type == operand_type::mem )
			{

				const auto address = cpu.calculate_memory_address ( instr.operands[ 0 ].mem,
																	instr.address + instr.length );
				if ( !mem.read ( address, &target, sizeof ( std::uint64_t ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			const auto return_address = instr.address + instr.length;

			auto rsp = cpu.read_gpr ( 4 );
			rsp -= 8;

			if ( !mem.write ( rsp, &return_address, sizeof ( std::uint64_t ) ) )
			{
				return false;
			}

			cpu.write_gpr ( 4, rsp );
			cpu.rip = target;

			return true;
		}
	}

	namespace pushfq
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			cpu.gpr[ 4 ] -= 8;
			const auto rsp = cpu.gpr[ 4 ];
			const auto flags = cpu.rflags;
			return mem.write ( rsp, &flags, 8 );
		}
	}

	namespace jmp
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 1 )
			{
				return false;
			}

			std::uint64_t target = 0;

			if ( instr.operands[ 0 ].type == operand_type::imm )
			{

				const auto rel_offset = static_cast< std::int64_t > ( instr.operands[ 0 ].imm );
				target = ( instr.address + instr.length ) + rel_offset;
			}
			else if ( instr.operands[ 0 ].type == operand_type::reg )
			{

				target = cpu.read_gpr ( instr.operands[ 0 ].reg );
			}
			else if ( instr.operands[ 0 ].type == operand_type::mem )
			{

				const auto address = cpu.calculate_memory_address ( instr.operands[ 0 ].mem,
																	instr.address + instr.length );
				if ( !mem.read ( address, &target, sizeof ( std::uint64_t ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			cpu.rip = target;

			return true;
		}
	}
}