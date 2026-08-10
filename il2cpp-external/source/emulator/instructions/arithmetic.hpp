#pragma once
#include <Windows.h>
#include <cstdio>
#include <array>
#include "../state.hpp"
#include "../handler.hpp"
#include "../decoder.hpp"

#if defined(_MSC_VER)
#include <intrin.h>
#pragma intrinsic(_umul128, _mul128)
#endif

namespace instructions
{
	namespace cmpxchg
	{
		inline auto execute ( cpu_state& cpu, memory_handler& memory, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];
			const auto size = dst.size;

			std::uint64_t dest_value = 0;
			if ( dst.type == operand_type::reg )
			{
				dest_value = cpu.read_gpr ( dst.reg );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !memory.read ( address, &dest_value, static_cast< int >( size ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			std::uint64_t src_value = 0;
			if ( src.type == operand_type::reg )
			{
				src_value = cpu.read_gpr ( src.reg );
			}
			else
			{
				return false;
			}

			std::uint64_t acc_value = cpu.read_gpr ( 0 );

			std::uint64_t mask = 0xFFFFFFFFFFFFFFFF;
			if ( size == operand_size::byte )
			{
				mask = 0xFF;
			}
			else if ( size == operand_size::word )
			{
				mask = 0xFFFF;
			}
			else if ( size == operand_size::dword )
			{
				mask = 0xFFFFFFFF;
			}

			dest_value &= mask;
			src_value &= mask;
			acc_value &= mask;

			const bool equal = ( acc_value == dest_value );

			if ( equal )
			{
				cpu.set_flag ( cpu_state::flag_zf, true );

				if ( dst.type == operand_type::reg )
				{
					if ( size == operand_size::dword || size == operand_size::qword )
					{
						cpu.write_gpr ( dst.reg, src_value );
					}
					else
					{
						std::uint64_t reg_val = cpu.read_gpr ( dst.reg );
						reg_val = ( reg_val & ~mask ) | ( src_value & mask );
						cpu.write_gpr ( dst.reg, reg_val );
					}
				}
				else if ( dst.type == operand_type::mem )
				{
					const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
					if ( !memory.write ( address, &src_value, static_cast< int >( size ) ) )
					{
						return false;
					}
				}
			}
			else
			{
				cpu.set_flag ( cpu_state::flag_zf, false );

				if ( size == operand_size::dword || size == operand_size::qword )
				{
					cpu.write_gpr ( 0, dest_value );
				}
				else
				{
					std::uint64_t rax = cpu.read_gpr ( 0 );
					rax = ( rax & ~mask ) | ( dest_value & mask );
					cpu.write_gpr ( 0, rax );
				}
			}

			std::uint64_t result = acc_value - dest_value;
			cpu.update_flags_zsp ( result & mask, size );
			cpu.set_flag ( cpu_state::flag_cf, acc_value < dest_value );

			return true;
		}
	}

	namespace xadd
	{
		inline auto execute ( cpu_state& cpu, memory_handler& memory, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];
			const auto size = dst.size;

			if ( src.type != operand_type::reg )
			{
				return false;
			}

			std::uint64_t mask = 0xFFFFFFFFFFFFFFFF;

			if ( size == operand_size::byte )
			{
				mask = 0xFF;
			}
			else if ( size == operand_size::word )
			{
				mask = 0xFFFF;
			}
			else if ( size == operand_size::dword )
			{
				mask = 0xFFFFFFFF;
			}

			std::uint64_t address = 0;
			std::uint64_t dest_value = 0;

			if ( dst.type == operand_type::reg )
			{
				dest_value = cpu.read_gpr ( dst.reg );
			}
			else if ( dst.type == operand_type::mem )
			{
				address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );

				if ( !memory.read ( address, &dest_value, static_cast< int >( size ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			const auto src_value = cpu.read_gpr ( src.reg ) & mask;

			dest_value &= mask;

			const auto sum = ( dest_value + src_value ) & mask;

			if ( size == operand_size::dword || size == operand_size::qword )
			{
				cpu.write_gpr ( src.reg, dest_value );
			}
			else
			{
				auto reg_val = cpu.read_gpr ( src.reg );
				reg_val = ( reg_val & ~mask ) | dest_value;

				cpu.write_gpr ( src.reg, reg_val );
			}

			if ( dst.type == operand_type::reg )
			{
				if ( size == operand_size::dword || size == operand_size::qword )
				{
					cpu.write_gpr ( dst.reg, sum );
				}
				else
				{
					auto reg_val = cpu.read_gpr ( dst.reg );
					reg_val = ( reg_val & ~mask ) | sum;

					cpu.write_gpr ( dst.reg, reg_val );
				}
			}
			else
			{
				if ( !memory.write ( address, &sum, static_cast< int >( size ) ) )
				{
					return false;
				}
			}

			cpu.update_flags_zsp ( sum, size );
			cpu.set_flag ( cpu_state::flag_cf, sum < dest_value );

			return true;
		}
	}

	namespace cmpxchg16b
	{
		inline auto execute ( cpu_state& cpu, memory_handler& memory, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 1 || instr.operands[ 0 ].type != operand_type::mem )
			{
				return false;
			}

			const auto address = cpu.calculate_memory_address ( instr.operands[ 0 ].mem, instr.address + instr.length );

			std::uint64_t mem_lo = 0, mem_hi = 0;
			if ( !memory.read ( address, &mem_lo, 8 ) ||
				 !memory.read ( address + 8, &mem_hi, 8 ) )
			{
				return false;
			}

			const std::uint64_t rax = cpu.read_gpr ( 0 );
			const std::uint64_t rdx = cpu.read_gpr ( 3 );
			const std::uint64_t rbx = cpu.read_gpr ( 1 );
			const std::uint64_t rcx = cpu.read_gpr ( 2 );

			if ( rax == mem_lo && rdx == mem_hi )
			{
				cpu.set_flag ( cpu_state::flag_zf, true );

				if ( !memory.write ( address, &rbx, 8 ) ||
					 !memory.write ( address + 8, &rcx, 8 ) )
				{
					return false;
				}
			}
			else
			{
				cpu.set_flag ( cpu_state::flag_zf, false );
				cpu.write_gpr ( 0, mem_lo );
				cpu.write_gpr ( 3, mem_hi );
			}

			cpu.rip = instr.address + instr.length;
			return true;
		}
	}

	namespace inc
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 1 )
			{
				return false;
			}

			const auto& operand = instr.operands[ 0 ];
			std::uint64_t value = 0;

			if ( operand.type == operand_type::reg )
			{
				value = cpu.read_gpr ( operand.reg );
			}
			else if ( operand.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( operand.mem, instr.address + instr.length );
				const auto size = static_cast< std::size_t >( operand.size );
				if ( !mem.read ( address, &value, size ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			std::uint64_t result = value + 1;

			if ( operand.size == operand_size::byte )
			{
				result &= 0xFF;
			}
			else if ( operand.size == operand_size::word )
			{
				result &= 0xFFFF;
			}
			else if ( operand.size == operand_size::dword )
			{
				result &= 0xFFFFFFFF;
			}

			cpu.update_flags_zsp ( result, operand.size );

			if ( operand.type == operand_type::reg )
			{
				cpu.write_gpr ( operand.reg, result );
			}
			else if ( operand.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( operand.mem, instr.address + instr.length );
				const auto size = static_cast< std::size_t >( operand.size );
				if ( !mem.write ( address, &result, size ) )
				{
					return false;
				}
			}

			return true;
		}
	}

	namespace dec
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 1 )
			{
				return false;
			}

			const auto& operand = instr.operands[ 0 ];
			std::uint64_t value = 0;

			if ( operand.type == operand_type::reg )
			{
				value = cpu.read_gpr ( operand.reg );
			}
			else if ( operand.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( operand.mem, instr.address + instr.length );
				const auto size = static_cast< std::size_t >( operand.size );
				if ( !mem.read ( address, &value, size ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			std::uint64_t result = value - 1;

			if ( operand.size == operand_size::byte )
			{
				result &= 0xFF;
			}
			else if ( operand.size == operand_size::word )
			{
				result &= 0xFFFF;
			}
			else if ( operand.size == operand_size::dword )
			{
				result &= 0xFFFFFFFF;
			}

			cpu.update_flags_zsp ( result, operand.size );

			if ( operand.type == operand_type::reg )
			{
				cpu.write_gpr ( operand.reg, result );
			}
			else if ( operand.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( operand.mem, instr.address + instr.length );
				const auto size = static_cast< std::size_t >( operand.size );
				if ( !mem.write ( address, &result, size ) )
				{
					return false;
				}
			}

			return true;
		}
	}

	namespace neg
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 1 )
			{
				return false;
			}

			const auto& operand = instr.operands[ 0 ];
			std::uint64_t value = 0;

			if ( operand.type == operand_type::reg )
			{
				value = cpu.read_gpr ( operand.reg );
			}
			else if ( operand.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( operand.mem, instr.address + instr.length );
				const auto size = static_cast< std::size_t >( operand.size );
				if ( !mem.read ( address, &value, size ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			std::uint64_t result = ( ~value ) + 1;

			if ( operand.size == operand_size::byte )
			{
				result &= 0xFF;
			}
			else if ( operand.size == operand_size::word )
			{
				result &= 0xFFFF;
			}
			else if ( operand.size == operand_size::dword )
			{
				result &= 0xFFFFFFFF;
			}

			cpu.set_flag ( cpu_state::flag_cf, value != 0 );
			cpu.update_flags_zsp ( result, operand.size );

			if ( operand.type == operand_type::reg )
			{
				cpu.write_gpr ( operand.reg, result );
			}
			else if ( operand.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( operand.mem, instr.address + instr.length );
				const auto size = static_cast< std::size_t >( operand.size );
				if ( !mem.write ( address, &result, size ) )
				{
					return false;
				}
			}

			return true;
		}
	}

	namespace add
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			std::uint64_t src_value = 0;
			if ( src.type == operand_type::imm )
			{
				src_value = src.imm;
			}
			else if ( src.type == operand_type::reg )
			{
				src_value = cpu.read_gpr ( src.reg );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &src_value, static_cast< std::size_t >( src.size ) ) )
				{
					src_value = 0;
				}
			}
			else
			{
				return false;
			}

			std::uint64_t dst_value = 0;
			if ( dst.type == operand_type::reg )
			{
				dst_value = cpu.read_gpr ( dst.reg );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.read ( address, &dst_value, static_cast< std::size_t >( dst.size ) ) )
				{
					dst_value = 0;
				}
			}
			else
			{
				return false;
			}

			std::uint64_t result = dst_value + src_value;

			if ( dst.size == operand_size::byte )
			{
				result &= 0xFF;
			}
			else if ( dst.size == operand_size::word )
			{
				result &= 0xFFFF;
			}
			else if ( dst.size == operand_size::dword )
			{
				result &= 0xFFFFFFFF;
			}

			cpu.update_flags_zsp ( result, dst.size );

			if ( dst.type == operand_type::reg )
			{
				cpu.write_gpr ( dst.reg, result );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				mem.write ( address, &result, static_cast< std::size_t >( dst.size ) );
			}

			return true;
		}
	}

	namespace adc
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			std::uint64_t src_value = 0;
			if ( src.type == operand_type::imm )
			{
				src_value = src.imm;
			}
			else if ( src.type == operand_type::reg )
			{
				src_value = cpu.read_gpr ( src.reg );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &src_value, static_cast< std::size_t >( src.size ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			std::uint64_t dst_value = 0;
			if ( dst.type == operand_type::reg )
			{
				dst_value = cpu.read_gpr ( dst.reg );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.read ( address, &dst_value, static_cast< std::size_t >( dst.size ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			const auto carry = cpu.get_flag ( cpu_state::flag_cf ) ? 1 : 0;
			std::uint64_t result = dst_value + src_value + carry;

			if ( dst.size == operand_size::byte )
			{
				result &= 0xFF;
			}
			else if ( dst.size == operand_size::word )
			{
				result &= 0xFFFF;
			}
			else if ( dst.size == operand_size::dword )
			{
				result &= 0xFFFFFFFF;
			}

			cpu.update_flags_zsp ( result, dst.size );

			if ( dst.type == operand_type::reg )
			{
				cpu.write_gpr ( dst.reg, result );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.write ( address, &result, static_cast< std::size_t >( dst.size ) ) )
				{
					return false;
				}
			}

			return true;
		}
	}

	namespace sub
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];
			const auto size = static_cast< int > ( dst.size );

			std::uint64_t src_value = 0;
			if ( src.type == operand_type::imm )
			{
				src_value = src.imm;
			}
			else if ( src.type == operand_type::reg )
			{
				src_value = cpu.read_gpr ( src.reg );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &src_value, static_cast< std::size_t >( src.size ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			std::uint64_t dst_value = 0;
			if ( dst.type == operand_type::reg )
			{
				dst_value = cpu.read_gpr ( dst.reg );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.read ( address, &dst_value, static_cast< std::size_t >( dst.size ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			const auto mask = ( dst.size == operand_size::byte ) ? 0xFFULL :
				( dst.size == operand_size::word ) ? 0xFFFFULL :
				( dst.size == operand_size::dword ) ? 0xFFFFFFFFULL : 0xFFFFFFFFFFFFFFFFULL;

			dst_value &= mask;
			src_value &= mask;

			const std::uint64_t result = ( dst_value - src_value ) & mask;

			cpu.update_flags_zsp ( result, dst.size );

			cpu.set_flag ( cpu_state::flag_cf, dst_value < src_value );

			const bool dst_sign = ( dst_value >> ( size * 8 - 1 ) ) & 1;
			const bool src_sign = ( src_value >> ( size * 8 - 1 ) ) & 1;
			const bool res_sign = ( result >> ( size * 8 - 1 ) ) & 1;
			cpu.set_flag ( cpu_state::flag_of, dst_sign != src_sign && dst_sign != res_sign );

			if ( dst.type == operand_type::reg )
			{
				cpu.write_gpr ( dst.reg, result );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.write ( address, &result, static_cast< std::size_t >( dst.size ) ) )
				{
					return false;
				}
			}

			return true;
		}
	}

	namespace sbb
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			std::uint64_t src_value = 0;
			if ( src.type == operand_type::imm )
			{
				src_value = src.imm;
			}
			else if ( src.type == operand_type::reg )
			{
				src_value = cpu.read_gpr ( src.reg );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &src_value, static_cast< std::size_t >( src.size ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			std::uint64_t dst_value = 0;
			if ( dst.type == operand_type::reg )
			{
				dst_value = cpu.read_gpr ( dst.reg );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.read ( address, &dst_value, static_cast< std::size_t >( dst.size ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			const auto carry = cpu.get_flag ( cpu_state::flag_cf ) ? 1 : 0;
			std::uint64_t result = dst_value - src_value - carry;

			if ( dst.size == operand_size::byte )
			{
				result &= 0xFF;
			}
			else if ( dst.size == operand_size::word )
			{
				result &= 0xFFFF;
			}
			else if ( dst.size == operand_size::dword )
			{
				result &= 0xFFFFFFFF;
			}

			cpu.update_flags_zsp ( result, dst.size );

			if ( dst.type == operand_type::reg )
			{
				cpu.write_gpr ( dst.reg, result );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.write ( address, &result, static_cast< std::size_t >( dst.size ) ) )
				{
					return false;
				}
			}

			return true;
		}
	}

	namespace mul
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 1 )
			{
				return false;
			}

			const auto& src = instr.operands[ 0 ];
			const auto size = static_cast< int > ( src.size );

			std::uint64_t rax_value = cpu.read_gpr ( 0 );
			std::uint64_t src_value = 0;

			if ( src.type == operand_type::reg )
			{
				src_value = cpu.read_gpr ( src.reg );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &src_value, size ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			if ( size == 4 )
			{
				rax_value &= 0xFFFFFFFF;
				src_value &= 0xFFFFFFFF;
			}

			if ( size == 8 )
			{
#if defined(_MSC_VER)
				std::uint64_t high = 0;
				std::uint64_t low = _umul128 ( rax_value, src_value, &high );
#else
				__uint128_t result = static_cast< __uint128_t >( rax_value ) * static_cast< __uint128_t >( src_value );
				std::uint64_t low = static_cast< std::uint64_t >( result );
				std::uint64_t high = static_cast< std::uint64_t >( result >> 64 );
#endif

				cpu.write_gpr ( 0, low );
				cpu.write_gpr ( 2, high );

				cpu.set_flag ( cpu_state::flag_cf, high != 0 );
				cpu.set_flag ( cpu_state::flag_of, high != 0 );
			}
			else
			{
				std::uint64_t result = rax_value * src_value;
				std::uint32_t low = static_cast< std::uint32_t >( result & 0xFFFFFFFF );
				std::uint32_t high = static_cast< std::uint32_t >( result >> 32 );

				cpu.write_gpr ( 0, low );
				cpu.write_gpr ( 2, high );

				cpu.set_flag ( cpu_state::flag_cf, high != 0 );
				cpu.set_flag ( cpu_state::flag_of, high != 0 );
			}

			return true;
		}
	}

	namespace imul
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 1 )
			{
				return false;
			}

			if ( instr.operand_count == 2 )
			{
				const auto& dst = instr.operands[ 0 ];
				const auto& src = instr.operands[ 1 ];

				std::int64_t dst_val = static_cast< std::int64_t > ( cpu.read_gpr ( dst.reg ) );
				std::int64_t src_val = 0;

				if ( src.type == operand_type::reg )
				{
					src_val = static_cast< std::int64_t >( cpu.read_gpr ( src.reg ) );
				}
				else if ( src.type == operand_type::mem )
				{
					const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
					std::uint64_t temp = 0;
					if ( !mem.read ( address, &temp, static_cast< std::size_t >( src.size ) ) )
					{
						return false;
					}
					src_val = static_cast< std::int64_t >( temp );
				}
				else
				{
					return false;
				}

				if ( dst.size == operand_size::dword )
				{
					dst_val = static_cast< std::int32_t >( dst_val );
					src_val = static_cast< std::int32_t >( src_val );

					const std::int64_t full = dst_val * src_val;
					const std::int32_t result = static_cast< std::int32_t >( full );

					cpu.write_gpr ( dst.reg, static_cast< std::uint64_t >( static_cast< std::uint32_t >( result ) ) );

					const bool overflow = ( full != static_cast< std::int64_t >( result ) );
					cpu.set_flag ( cpu_state::flag_cf, overflow );
					cpu.set_flag ( cpu_state::flag_of, overflow );
				}
				else
				{
#if defined(_MSC_VER)
					std::int64_t high = 0;
					const std::int64_t low = _mul128 ( dst_val, src_val, &high );
#else
					const __int128_t full = static_cast< __int128_t >( dst_val ) * static_cast< __int128_t >( src_val );
					const std::int64_t low = static_cast< std::int64_t >( full );
					const std::int64_t high = static_cast< std::int64_t >( full >> 64 );
#endif
					cpu.write_gpr ( dst.reg, static_cast< std::uint64_t >( low ) );

					const bool overflow = ( high != ( low >> 63 ? -1LL : 0LL ) );
					cpu.set_flag ( cpu_state::flag_cf, overflow );
					cpu.set_flag ( cpu_state::flag_of, overflow );
				}

				return true;
			}

			if ( instr.operand_count == 3 )
			{
				const auto& dst = instr.operands[ 0 ];
				const auto& src = instr.operands[ 1 ];
				const auto& imm = instr.operands[ 2 ];

				std::int64_t src_val = 0;
				if ( src.type == operand_type::reg )
				{
					src_val = static_cast< std::int64_t >( cpu.read_gpr ( src.reg ) );
				}
				else if ( src.type == operand_type::mem )
				{
					const auto addr = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
					std::uint64_t temp = 0;
					if ( !mem.read ( addr, &temp, static_cast< std::size_t >( src.size ) ) )
					{
						return false;
					}
					src_val = static_cast< std::int64_t >( temp );
				}
				else
				{
					return false;
				}

				if ( src.size == operand_size::dword )
				{
					src_val = static_cast< std::int32_t >( src_val );
				}

				std::int64_t imm_val = static_cast< std::int64_t >( imm.imm );
				std::int64_t result = src_val * imm_val;

				if ( dst.size == operand_size::dword )
				{
					result = static_cast< std::int32_t >( result );
				}

				cpu.write_gpr ( dst.reg, static_cast< std::uint64_t >( result ) );
				return true;
			}

			const auto& src = instr.operands[ 0 ];
			const auto size = static_cast< int >( src.size );

			std::int64_t rax_value = static_cast< std::int64_t >( cpu.read_gpr ( 0 ) );
			std::int64_t src_value = 0;

			if ( src.type == operand_type::reg )
			{
				src_value = static_cast< std::int64_t >( cpu.read_gpr ( src.reg ) );
			}
			else if ( src.type == operand_type::mem )
			{
				std::uint64_t temp = 0;
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &temp, size ) )
				{
					return false;
				}
				src_value = static_cast< std::int64_t >( temp );
			}
			else
			{
				return false;
			}

			if ( size == 4 )
			{
				rax_value = static_cast< std::int32_t >( rax_value );
				src_value = static_cast< std::int32_t >( src_value );
			}

			if ( size == 8 )
			{
#if defined(_MSC_VER)
				std::int64_t high = 0;
				std::int64_t low = _mul128 ( rax_value, src_value, &high );
#else
				__int128_t result = static_cast< __int128_t >( rax_value ) * static_cast< __int128_t >( src_value );
				std::int64_t low = static_cast< std::int64_t >( result );
				std::int64_t high = static_cast< std::int64_t >( result >> 64 );
#endif
				cpu.write_gpr ( 0, static_cast< std::uint64_t >( low ) );
				cpu.write_gpr ( 2, static_cast< std::uint64_t >( high ) );

				bool overflow = ( high != ( low >> 63 ? -1LL : 0LL ) );
				cpu.set_flag ( cpu_state::flag_cf, overflow );
				cpu.set_flag ( cpu_state::flag_of, overflow );
			}
			else
			{
				std::int64_t result = rax_value * src_value;
				std::int32_t low = static_cast< std::int32_t >( result );
				std::int32_t high = static_cast< std::int32_t >( result >> 32 );

				cpu.write_gpr ( 0, static_cast< std::uint64_t >( static_cast< std::uint32_t >( low ) ) );
				cpu.write_gpr ( 2, static_cast< std::uint64_t >( static_cast< std::uint32_t >( high ) ) );

				bool overflow = ( high != ( low >> 31 ? -1 : 0 ) );
				cpu.set_flag ( cpu_state::flag_cf, overflow );
				cpu.set_flag ( cpu_state::flag_of, overflow );
			}

			return true;
		}
	}

	namespace div
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 1 )
			{
				return false;
			}

			const auto& src = instr.operands[ 0 ];
			const auto size = static_cast< int > ( src.size );

			std::uint64_t divisor = 0;
			if ( src.type == operand_type::reg )
			{
				divisor = cpu.read_gpr ( src.reg );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &divisor, size ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			if ( divisor == 0 )
			{
				return false;
			}

			if ( size == 8 )
			{
				std::uint64_t rax = cpu.read_gpr ( 0 );
				std::uint64_t rdx = cpu.read_gpr ( 2 );

				if ( rdx >= divisor )
				{
					return false;
				}

				std::uint64_t quotient = rax / divisor;
				std::uint64_t remainder = rax % divisor;

				cpu.write_gpr ( 0, quotient );
				cpu.write_gpr ( 2, remainder );
			}
			else
			{
				std::uint32_t eax = static_cast< std::uint32_t >( cpu.read_gpr ( 0 ) & 0xFFFFFFFF );
				std::uint32_t edx = static_cast< std::uint32_t >( cpu.read_gpr ( 2 ) & 0xFFFFFFFF );
				std::uint32_t divisor32 = static_cast< std::uint32_t >( divisor & 0xFFFFFFFF );

				std::uint64_t dividend = ( static_cast< std::uint64_t >( edx ) << 32 ) | eax;

				std::uint32_t quotient = static_cast< std::uint32_t >( dividend / divisor32 );
				std::uint32_t remainder = static_cast< std::uint32_t >( dividend % divisor32 );

				cpu.write_gpr ( 0, quotient );
				cpu.write_gpr ( 2, remainder );
			}

			return true;
		}
	}

	namespace idiv
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 1 )
			{
				return false;
			}

			const auto& src = instr.operands[ 0 ];
			const auto size = static_cast< int > ( src.size );

			std::int64_t divisor = 0;
			if ( src.type == operand_type::reg )
			{
				divisor = static_cast< std::int64_t >( cpu.read_gpr ( src.reg ) );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				std::uint64_t temp = 0;
				if ( !mem.read ( address, &temp, size ) )
				{
					return false;
				}
				divisor = static_cast< std::int64_t >( temp );
			}
			else
			{
				return false;
			}

			if ( size == 4 )
			{
				divisor = static_cast< std::int32_t >( divisor );
			}

			if ( divisor == 0 )
			{
				return false;
			}

			if ( size == 8 )
			{
				std::int64_t rax = static_cast< std::int64_t >( cpu.read_gpr ( 0 ) );
				std::int64_t rdx = static_cast< std::int64_t >( cpu.read_gpr ( 2 ) );

				std::int64_t quotient = rax / divisor;
				std::int64_t remainder = rax % divisor;

				cpu.write_gpr ( 0, static_cast< std::uint64_t >( quotient ) );
				cpu.write_gpr ( 2, static_cast< std::uint64_t >( remainder ) );
			}
			else
			{
				std::int32_t eax = static_cast< std::int32_t >( cpu.read_gpr ( 0 ) & 0xFFFFFFFF );
				std::int32_t edx = static_cast< std::int32_t >( cpu.read_gpr ( 2 ) & 0xFFFFFFFF );
				std::int32_t divisor32 = static_cast< std::int32_t >( divisor );

				std::int64_t dividend = ( static_cast< std::int64_t >( edx ) << 32 ) | static_cast< std::uint32_t >( eax );

				std::int32_t quotient = static_cast< std::int32_t >( dividend / divisor32 );
				std::int32_t remainder = static_cast< std::int32_t >( dividend % divisor32 );

				cpu.write_gpr ( 0, static_cast< std::uint32_t >( quotient ) );
				cpu.write_gpr ( 2, static_cast< std::uint32_t >( remainder ) );
			}

			return true;
		}
	}

	namespace and_instr
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			std::uint64_t src_value = 0;
			if ( src.type == operand_type::imm )
			{
				src_value = src.imm;
			}
			else if ( src.type == operand_type::reg )
			{
				src_value = cpu.read_gpr ( src.reg );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &src_value, static_cast< std::size_t >( src.size ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			std::uint64_t dst_value = 0;
			if ( dst.type == operand_type::reg )
			{
				dst_value = cpu.read_gpr ( dst.reg );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.read ( address, &dst_value, static_cast< std::size_t >( dst.size ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			std::uint64_t result = dst_value & src_value;

			if ( dst.size == operand_size::byte )
			{
				result &= 0xFF;
			}
			else if ( dst.size == operand_size::word )
			{
				result &= 0xFFFF;
			}
			else if ( dst.size == operand_size::dword )
			{
				result &= 0xFFFFFFFF;
			}

			cpu.update_flags_logic ( result, dst.size );

			if ( dst.type == operand_type::reg )
			{
				cpu.write_gpr ( dst.reg, result );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.write ( address, &result, static_cast< std::size_t >( dst.size ) ) )
				{
					return false;
				}
			}

			return true;
		}
	}

	namespace or_instr
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];
			const auto size = static_cast< int > ( dst.size );

			std::uint64_t dst_value = 0;
			std::uint64_t src_value = 0;

			if ( dst.type == operand_type::reg )
			{
				dst_value = cpu.read_gpr ( dst.reg );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.read ( address, &dst_value, size ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			if ( src.type == operand_type::reg )
			{
				src_value = cpu.read_gpr ( src.reg );
			}
			else if ( src.type == operand_type::imm )
			{
				src_value = src.imm;
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &src_value, size ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			if ( size == 4 )
			{
				dst_value &= 0xFFFFFFFF;
				src_value &= 0xFFFFFFFF;
			}

			std::uint64_t result = dst_value | src_value;

			if ( size == 4 )
			{
				result &= 0xFFFFFFFF;
			}

			cpu.update_flags_logic ( result, dst.size );

			if ( dst.type == operand_type::reg )
			{
				cpu.write_gpr ( dst.reg, result );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.write ( address, &result, size ) )
				{
					return false;
				}
			}

			return true;
		}
	}

	namespace xor_instr
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];
			const auto size = static_cast< int > ( dst.size );

			std::uint64_t dst_value = 0;
			std::uint64_t src_value = 0;

			if ( dst.type == operand_type::reg )
			{
				dst_value = cpu.read_gpr ( dst.reg );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.read ( address, &dst_value, size ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			if ( src.type == operand_type::reg )
			{
				src_value = cpu.read_gpr ( src.reg );
			}
			else if ( src.type == operand_type::imm )
			{
				src_value = src.imm;
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &src_value, size ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			if ( size == 4 )
			{
				dst_value &= 0xFFFFFFFF;
				src_value &= 0xFFFFFFFF;
			}

			std::uint64_t result = dst_value ^ src_value;

			if ( size == 4 )
			{
				result &= 0xFFFFFFFF;
			}

			cpu.update_flags_logic ( result, dst.size );

			if ( dst.type == operand_type::reg )
			{
				cpu.write_gpr ( dst.reg, result );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.write ( address, &result, size ) )
				{
					return false;
				}
			}

			return true;
		}
	}

	namespace not_instr
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 1 )
			{
				return false;
			}

			const auto& operand = instr.operands[ 0 ];
			std::uint64_t value = 0;

			if ( operand.type == operand_type::reg )
			{
				value = cpu.read_gpr ( operand.reg );
			}
			else if ( operand.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( operand.mem, instr.address + instr.length );
				const auto size = static_cast< std::size_t >( operand.size );
				if ( !mem.read ( address, &value, size ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			std::uint64_t result = ~value;

			if ( operand.size == operand_size::byte )
			{
				result &= 0xFF;
			}
			else if ( operand.size == operand_size::word )
			{
				result &= 0xFFFF;
			}
			else if ( operand.size == operand_size::dword )
			{
				result &= 0xFFFFFFFF;
			}

			if ( operand.type == operand_type::reg )
			{
				cpu.write_gpr ( operand.reg, result );
			}
			else if ( operand.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( operand.mem, instr.address + instr.length );
				const auto size = static_cast< std::size_t >( operand.size );
				if ( !mem.write ( address, &result, size ) )
				{
					return false;
				}
			}

			return true;
		}
	}

	namespace test
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];
			const auto size = static_cast< int > ( dst.size );

			std::uint64_t dst_value = 0;
			std::uint64_t src_value = 0;


			if ( src.type == operand_type::imm )
			{
				src_value = src.imm;
			}
			else if ( src.type == operand_type::reg )
			{
				src_value = cpu.read_gpr ( src.reg );
			}
			else
			{
				return false;
			}

			if ( src_value == 0 )
			{
				cpu.update_flags_logic ( 0, dst.size );
				cpu.rip = instr.address + instr.length;
				return true;
			}

			if ( dst.type == operand_type::reg )
			{
				dst_value = cpu.read_gpr ( dst.reg );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.read ( address, &dst_value, size ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			const auto result = dst_value & src_value;
			cpu.update_flags_logic ( result, dst.size );
			cpu.rip = instr.address + instr.length;
			return true;
		}
	}

	namespace cmp
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];
			const auto size = static_cast< int > ( dst.size );

			std::uint64_t dst_value = 0;
			std::uint64_t src_value = 0;

			if ( dst.type == operand_type::reg )
			{
				dst_value = cpu.read_gpr ( dst.reg );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.read ( address, &dst_value, size ) )
				{
					dst_value = 0;
				}
			}
			else
			{
				return false;
			}

			if ( src.type == operand_type::reg )
			{
				src_value = cpu.read_gpr ( src.reg );
			}
			else if ( src.type == operand_type::imm )
			{
				src_value = src.imm;
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &src_value, size ) )
				{
					src_value = 0;
				}
			}
			else
			{
				return false;
			}

			const auto mask = ( dst.size == operand_size::byte ) ? 0xFFULL :
				( dst.size == operand_size::word ) ? 0xFFFFULL :
				( dst.size == operand_size::dword ) ? 0xFFFFFFFFULL : 0xFFFFFFFFFFFFFFFFULL;

			dst_value &= mask;
			src_value &= mask;

			const std::uint64_t result = ( dst_value - src_value ) & mask;

			cpu.update_flags_zsp ( result, dst.size );

			cpu.set_flag ( cpu_state::flag_cf, dst_value < src_value );

			const bool dst_sign = ( dst_value >> ( size * 8 - 1 ) ) & 1;
			const bool src_sign = ( src_value >> ( size * 8 - 1 ) ) & 1;
			const bool res_sign = ( result >> ( size * 8 - 1 ) ) & 1;
			cpu.set_flag ( cpu_state::flag_of, dst_sign != src_sign && dst_sign != res_sign );

			return true;
		}
	}

	namespace shl
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];
			const auto size = static_cast< int > ( dst.size );

			std::uint64_t value = 0;
			std::uint64_t count = 0;

			if ( dst.type == operand_type::reg )
			{
				value = cpu.read_gpr ( dst.reg );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.read ( address, &value, size ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			if ( src.type == operand_type::imm )
			{
				count = src.imm & 0x3F;
			}
			else
			{
				return false;
			}

			if ( size == 4 )
			{
				value &= 0xFFFFFFFF;
			}

			std::uint64_t result = value << count;

			if ( size == 4 )
			{
				result &= 0xFFFFFFFF;
			}

			cpu.update_flags_zsp ( result, dst.size );

			if ( dst.type == operand_type::reg )
			{
				cpu.write_gpr ( dst.reg, result );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.write ( address, &result, size ) )
				{
					return false;
				}
			}

			return true;
		}
	}

	namespace shr
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];
			const auto size = static_cast< int > ( dst.size );

			std::uint64_t value = 0;
			std::uint64_t count = 0;

			if ( dst.type == operand_type::reg )
			{
				value = cpu.read_gpr ( dst.reg );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.read ( address, &value, size ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			if ( src.type == operand_type::imm )
			{
				count = src.imm & 0x3F;
			}
			else
			{
				return false;
			}

			if ( size == 4 )
			{
				value &= 0xFFFFFFFF;
			}

			std::uint64_t result = value >> count;

			cpu.update_flags_zsp ( result, dst.size );

			if ( dst.type == operand_type::reg )
			{
				cpu.write_gpr ( dst.reg, result );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.write ( address, &result, size ) )
				{
					return false;
				}
			}

			return true;
		}
	}

	namespace sar
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];
			const auto size = static_cast< int > ( dst.size );

			std::int64_t value = 0;
			std::uint64_t count = 0;

			if ( dst.type == operand_type::reg )
			{
				value = static_cast< std::int64_t >( cpu.read_gpr ( dst.reg ) );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				std::uint64_t temp = 0;
				if ( !mem.read ( address, &temp, size ) )
				{
					return false;
				}
				value = static_cast< std::int64_t >( temp );
			}
			else
			{
				return false;
			}

			if ( src.type == operand_type::imm )
			{
				count = src.imm & 0x3F;
			}
			else
			{
				return false;
			}

			if ( size == 4 )
			{
				value = static_cast< std::int32_t >( value );
			}

			std::int64_t result = value >> count;

			std::uint64_t unsigned_result = static_cast< std::uint64_t >( result );
			if ( size == 4 )
			{
				unsigned_result &= 0xFFFFFFFF;
			}

			cpu.update_flags_zsp ( unsigned_result, dst.size );

			if ( dst.type == operand_type::reg )
			{
				cpu.write_gpr ( dst.reg, unsigned_result );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.write ( address, &unsigned_result, size ) )
				{
					return false;
				}
			}

			return true;
		}
	}

	namespace rol
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];
			const auto size = static_cast< int > ( dst.size );

			std::uint64_t value = 0;
			std::uint64_t count = 0;

			if ( dst.type == operand_type::reg )
			{
				value = cpu.read_gpr ( dst.reg );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.read ( address, &value, size ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			if ( src.type == operand_type::imm )
			{
				count = src.imm & 0x3F;
			}
			else
			{
				return false;
			}

			if ( count == 0 )
			{
				return true;
			}

			const int bits = size * 8;
			count = count % bits;

			std::uint64_t result = ( value << count ) | ( value >> ( bits - count ) );

			if ( size == 4 )
			{
				result &= 0xFFFFFFFF;
			}

			if ( dst.type == operand_type::reg )
			{
				cpu.write_gpr ( dst.reg, result );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.write ( address, &result, size ) )
				{
					return false;
				}
			}

			return true;
		}
	}

	namespace ror
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];
			const auto size = static_cast< int > ( dst.size );

			std::uint64_t value = 0;
			std::uint64_t count = 0;

			if ( dst.type == operand_type::reg )
			{
				value = cpu.read_gpr ( dst.reg );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.read ( address, &value, size ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			if ( src.type == operand_type::imm )
			{
				count = src.imm & 0x3F;
			}
			else
			{
				return false;
			}

			if ( count == 0 )
			{
				return true;
			}

			const int bits = size * 8;
			count = count % bits;

			std::uint64_t result = ( value >> count ) | ( value << ( bits - count ) );

			if ( size == 4 )
			{
				result &= 0xFFFFFFFF;
			}

			if ( dst.type == operand_type::reg )
			{
				cpu.write_gpr ( dst.reg, result );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.write ( address, &result, size ) )
				{
					return false;
				}
			}

			return true;
		}
	}

	namespace rcl
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			std::uint64_t value = 0;
			if ( dst.type == operand_type::reg )
			{
				value = cpu.read_gpr ( dst.reg );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.read ( address, &value, static_cast< int >( dst.size ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			std::uint64_t count = 0;
			if ( src.type == operand_type::imm )
			{
				count = src.imm & 0x3F;
			}
			else
			{
				return false;
			}

			if ( dst.type == operand_type::reg )
			{
				cpu.write_gpr ( dst.reg, value );
			}

			return true;
		}
	}

	namespace rcr
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			std::uint64_t value = 0;
			if ( dst.type == operand_type::reg )
			{
				value = cpu.read_gpr ( dst.reg );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.read ( address, &value, static_cast< int >( dst.size ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			std::uint64_t count = 0;
			if ( src.type == operand_type::imm )
			{
				count = src.imm & 0x3F;
			}
			else
			{
				return false;
			}

			if ( dst.type == operand_type::reg )
			{
				cpu.write_gpr ( dst.reg, value );
			}

			return true;
		}
	}

	namespace bt
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			std::uint64_t value = 0;
			if ( dst.type == operand_type::reg )
			{
				value = cpu.read_gpr ( dst.reg );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.read ( address, &value, static_cast< int >( dst.size ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			std::uint64_t bit_index = 0;
			if ( src.type == operand_type::imm )
			{
				bit_index = src.imm;
			}
			else if ( src.type == operand_type::reg )
			{
				bit_index = cpu.read_gpr ( src.reg );
			}
			else
			{
				return false;
			}

			const int bits = static_cast< int >( dst.size ) * 8;
			bit_index = bit_index % bits;

			const bool bit_value = ( value >> bit_index ) & 1;
			cpu.set_flag ( cpu_state::flag_cf, bit_value );

			return true;
		}
	}

	namespace bts
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			std::uint64_t value = 0;
			if ( dst.type == operand_type::reg )
			{
				value = cpu.read_gpr ( dst.reg );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.read ( address, &value, static_cast< int >( dst.size ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			std::uint64_t bit_index = 0;
			if ( src.type == operand_type::imm )
			{
				bit_index = src.imm;
			}
			else if ( src.type == operand_type::reg )
			{
				bit_index = cpu.read_gpr ( src.reg );
			}
			else
			{
				return false;
			}

			const int bits = static_cast< int >( dst.size ) * 8;
			bit_index = bit_index % bits;

			const bool bit_value = ( value >> bit_index ) & 1;
			cpu.set_flag ( cpu_state::flag_cf, bit_value );

			value |= ( 1ULL << bit_index );

			if ( dst.type == operand_type::reg )
			{
				cpu.write_gpr ( dst.reg, value );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.write ( address, &value, static_cast< int >( dst.size ) ) )
				{
					return false;
				}
			}

			return true;
		}
	}

	namespace btr
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			std::uint64_t value = 0;
			if ( dst.type == operand_type::reg )
			{
				value = cpu.read_gpr ( dst.reg );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.read ( address, &value, static_cast< int >( dst.size ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			std::uint64_t bit_index = 0;
			if ( src.type == operand_type::imm )
			{
				bit_index = src.imm;
			}
			else if ( src.type == operand_type::reg )
			{
				bit_index = cpu.read_gpr ( src.reg );
			}
			else
			{
				return false;
			}

			const int bits = static_cast< int >( dst.size ) * 8;
			bit_index = bit_index % bits;

			const bool bit_value = ( value >> bit_index ) & 1;
			cpu.set_flag ( cpu_state::flag_cf, bit_value );

			value &= ~( 1ULL << bit_index );

			if ( dst.type == operand_type::reg )
			{
				cpu.write_gpr ( dst.reg, value );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.write ( address, &value, static_cast< int >( dst.size ) ) )
				{
					return false;
				}
			}

			return true;
		}
	}

	namespace btc
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			std::uint64_t value = 0;
			if ( dst.type == operand_type::reg )
			{
				value = cpu.read_gpr ( dst.reg );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.read ( address, &value, static_cast< int >( dst.size ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			std::uint64_t bit_index = 0;
			if ( src.type == operand_type::imm )
			{
				bit_index = src.imm;
			}
			else if ( src.type == operand_type::reg )
			{
				bit_index = cpu.read_gpr ( src.reg );
			}
			else
			{
				return false;
			}

			const int bits = static_cast< int >( dst.size ) * 8;
			bit_index = bit_index % bits;

			const bool bit_value = ( value >> bit_index ) & 1;
			cpu.set_flag ( cpu_state::flag_cf, bit_value );

			value ^= ( 1ULL << bit_index );

			if ( dst.type == operand_type::reg )
			{
				cpu.write_gpr ( dst.reg, value );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.write ( address, &value, static_cast< int >( dst.size ) ) )
				{
					return false;
				}
			}

			return true;
		}
	}

	namespace bsf
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			std::uint64_t value = 0;
			if ( src.type == operand_type::reg )
			{
				value = cpu.read_gpr ( src.reg );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &value, static_cast< int >( src.size ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			if ( value == 0 )
			{
				cpu.set_flag ( cpu_state::flag_zf, true );
				return true;
			}

			std::uint64_t index = 0;
			for ( int i = 0; i < 64; i++ )
			{
				if ( value & ( 1ULL << i ) )
				{
					index = i;
					break;
				}
			}

			cpu.set_flag ( cpu_state::flag_zf, false );

			if ( dst.type == operand_type::reg )
			{
				cpu.write_gpr ( dst.reg, index );
			}

			return true;
		}
	}

	namespace bsr
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			std::uint64_t value = 0;
			if ( src.type == operand_type::reg )
			{
				value = cpu.read_gpr ( src.reg );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &value, static_cast< int >( src.size ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			if ( value == 0 )
			{
				cpu.set_flag ( cpu_state::flag_zf, true );
				return true;
			}

			std::uint64_t index = 0;
			for ( int i = 63; i >= 0; i-- )
			{
				if ( value & ( 1ULL << i ) )
				{
					index = i;
					break;
				}
			}

			cpu.set_flag ( cpu_state::flag_zf, false );

			if ( dst.type == operand_type::reg )
			{
				cpu.write_gpr ( dst.reg, index );
			}

			return true;
		}
	}

	namespace bswap
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 1 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];

			if ( dst.type != operand_type::reg )
			{
				return false;
			}

			const auto value = cpu.read_gpr ( dst.reg );

			if ( dst.size == operand_size::qword )
			{
				const auto swapped =
					( ( value & 0x00000000000000FFull ) << 56 ) |
					( ( value & 0x000000000000FF00ull ) << 40 ) |
					( ( value & 0x0000000000FF0000ull ) << 24 ) |
					( ( value & 0x00000000FF000000ull ) << 8 ) |
					( ( value & 0x000000FF00000000ull ) >> 8 ) |
					( ( value & 0x0000FF0000000000ull ) >> 24 ) |
					( ( value & 0x00FF000000000000ull ) >> 40 ) |
					( ( value & 0xFF00000000000000ull ) >> 56 );

				cpu.write_gpr ( dst.reg, swapped );

				return true;
			}

			const auto lower = static_cast< std::uint32_t > ( value );

			const auto swapped =
				( ( lower & 0x000000FFu ) << 24 ) |
				( ( lower & 0x0000FF00u ) << 8 ) |
				( ( lower & 0x00FF0000u ) >> 8 ) |
				( ( lower & 0xFF000000u ) >> 24 );

			cpu.write_gpr ( dst.reg, static_cast< std::uint64_t > ( swapped ) );

			return true;
		}
	}

	namespace cmov
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			const auto zf = cpu.get_flag ( cpu_state::flag_zf );
			const auto sf = cpu.get_flag ( cpu_state::flag_sf );
			const auto of = cpu.get_flag ( cpu_state::flag_of );
			const auto cf = cpu.get_flag ( cpu_state::flag_cf );

			bool condition = false;
			switch ( instr.type )
			{
				case instruction_type::cmove:
					condition = zf;
					break;
				case instruction_type::cmovne:
					condition = !zf;
					break;
				case instruction_type::cmovg:
					condition = !zf && ( sf == of );
					break;
				case instruction_type::cmovge:
					condition = sf == of;
					break;
				case instruction_type::cmovl:
					condition = sf != of;
					break;
				case instruction_type::cmovle:
					condition = zf || ( sf != of );
					break;
				case instruction_type::cmova:
					condition = !cf && !zf;
					break;
				case instruction_type::cmovae:
					condition = !cf;
					break;
				case instruction_type::cmovb:
					condition = cf;
					break;
				case instruction_type::cmovbe:
					condition = cf || zf;
					break;
				case instruction_type::cmovs:
					condition = sf;
					break;
				case instruction_type::cmovns:
					condition = !sf;
					break;
				case instruction_type::cmovo:
					condition = of;
					break;
				case instruction_type::cmovno:
					condition = !of;
					break;
				default:
					return false;
			}

			if ( !condition )
			{
				return true;
			}

			std::uint64_t value = 0;
			if ( src.type == operand_type::reg )
			{
				value = cpu.read_gpr ( src.reg );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &value, static_cast< int >( src.size ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			if ( dst.type == operand_type::reg )
			{
				cpu.write_gpr ( dst.reg, value );
			}
			else
			{
				return false;
			}

			return true;
		}
	}

	namespace setcc
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 1 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];

			const auto zf = cpu.get_flag ( cpu_state::flag_zf );
			const auto sf = cpu.get_flag ( cpu_state::flag_sf );
			const auto of = cpu.get_flag ( cpu_state::flag_of );
			const auto cf = cpu.get_flag ( cpu_state::flag_cf );

			bool condition = false;
			switch ( instr.type )
			{
				case instruction_type::sete:
					condition = zf;
					break;
				case instruction_type::setne:
					condition = !zf;
					break;
				case instruction_type::setg:
					condition = !zf && ( sf == of );
					break;
				case instruction_type::setge:
					condition = sf == of;
					break;
				case instruction_type::setl:
					condition = sf != of;
					break;
				case instruction_type::setle:
					condition = zf || ( sf != of );
					break;
				case instruction_type::seta:
					condition = !cf && !zf;
					break;
				case instruction_type::setae:
					condition = !cf;
					break;
				case instruction_type::setb:
					condition = cf;
					break;
				case instruction_type::setbe:
					condition = cf || zf;
					break;
				case instruction_type::sets:
					condition = sf;
					break;
				case instruction_type::setns:
					condition = !sf;
					break;
				case instruction_type::seto:
					condition = of;
					break;
				case instruction_type::setno:
					condition = !of;
					break;
				default:
					return false;
			}

			const std::uint8_t value = condition ? 1 : 0;

			if ( dst.type == operand_type::reg )
			{
				cpu.write_gpr ( dst.reg, value );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.write ( address, &value, 1 ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			return true;
		}
	}

	namespace rdtsc
	{
		inline auto execute ( cpu_state& cpu, const instruction& instr ) -> bool
		{
			const std::uint64_t timestamp = 0x1234567890ABCDEF;
			cpu.write_gpr ( 0, timestamp & 0xFFFFFFFF );
			cpu.write_gpr ( 2, timestamp >> 32 );
			return true;
		}
	}

	namespace rdtscp
	{
		inline auto execute ( cpu_state& cpu, const instruction& instr ) -> bool
		{
			const std::uint64_t timestamp = 0x1234567890ABCDEF;
			cpu.write_gpr ( 0, timestamp & 0xFFFFFFFF );
			cpu.write_gpr ( 2, timestamp >> 32 );
			cpu.write_gpr ( 1, 0 );
			return true;
		}
	}

	namespace cpuid
	{
		inline auto execute ( cpu_state& cpu, const instruction& instr ) -> bool
		{
			const auto leaf = cpu.read_gpr ( 0 );

			switch ( leaf )
			{
				case 0:
					cpu.write_gpr ( 0, 0x16 );
					cpu.write_gpr ( 1, 0x756E6547 );
					cpu.write_gpr ( 2, 0x6C65746E );
					cpu.write_gpr ( 3, 0x49656E69 );
					break;
				case 1:
					cpu.write_gpr ( 0, 0x000806E9 );
					cpu.write_gpr ( 1, 0x00000000 );
					cpu.write_gpr ( 2, 0x7FFAFBBF );
					cpu.write_gpr ( 3, 0xBFEBFBFF );
					break;
				default:
					cpu.write_gpr ( 0, 0 );
					cpu.write_gpr ( 1, 0 );
					cpu.write_gpr ( 2, 0 );
					cpu.write_gpr ( 3, 0 );
					break;
			}

			return true;
		}
	}

	namespace cdq
	{
		inline auto execute ( cpu_state& cpu, const instruction& instr ) -> bool
		{
			const auto eax = static_cast< std::int32_t >( cpu.read_gpr ( 0 ) & 0xFFFFFFFF );
			const auto edx = ( eax < 0 ) ? 0xFFFFFFFF : 0;
			cpu.write_gpr ( 2, edx );
			return true;
		}
	}

	namespace cqo
	{
		inline auto execute ( cpu_state& cpu, const instruction& instr ) -> bool
		{
			const auto rax = static_cast< std::int64_t > ( cpu.read_gpr ( 0 ) );
			const auto rdx = ( rax < 0 ) ? 0xFFFFFFFFFFFFFFFF : 0;
			cpu.write_gpr ( 2, rdx );
			return true;
		}
	}

	namespace cbw
	{
		inline auto execute ( cpu_state& cpu, const instruction& instr ) -> bool
		{
			const auto al = static_cast< std::int8_t > ( cpu.read_gpr ( 0 ) & 0xFF );
			const auto ax = static_cast< std::int16_t >( al );
			cpu.write_gpr ( 0, static_cast< std::uint16_t >( ax ) );
			return true;
		}
	}

	namespace cwde
	{
		inline auto execute ( cpu_state& cpu, const instruction& instr ) -> bool
		{
			const auto ax = static_cast< std::int16_t >( cpu.read_gpr ( 0 ) & 0xFFFF );
			const auto eax = static_cast< std::int32_t >( ax );
			cpu.write_gpr ( 0, static_cast< std::uint32_t >( eax ) );
			return true;
		}
	}

	namespace cdqe
	{
		inline auto execute ( cpu_state& cpu, const instruction& instr ) -> bool
		{
			const auto eax = static_cast< std::int32_t >( cpu.read_gpr ( 0 ) & 0xFFFFFFFF );
			const auto rax = static_cast< std::int64_t >( eax );
			cpu.write_gpr ( 0, static_cast< std::uint64_t >( rax ) );
			return true;
		}
	}
}
