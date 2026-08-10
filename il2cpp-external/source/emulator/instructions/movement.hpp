#pragma once
#include <Windows.h>
#include <cstdio>
#include <array>
#include "../state.hpp"
#include "../handler.hpp"
#include "../decoder.hpp"

namespace instructions
{
	namespace mov
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			const auto size = dst.size;

			std::uint64_t value = 0;

			if ( src.type == operand_type::reg )
			{
				value = cpu.read_gpr ( src.reg );
			}
			else if ( src.type == operand_type::mem )
			{
				std::uint64_t address = 0;

				if ( instr.has_segment_prefix )
				{
					if ( instr.segment_prefix == 0x65 )
					{
						address += cpu.gs_base;
					}
					else if ( instr.segment_prefix == 0x64 )
					{
						address += cpu.fs_base;
					}
				}

				if ( src.mem.base != 0xFF )
				{
					std::uint64_t base_value = 0;

					if ( src.mem.base == 16 )
					{
						base_value = instr.address + instr.length;
					}
					else
					{
						base_value = cpu.read_gpr ( src.mem.base );
					}

					address += base_value;
				}

				if ( src.mem.index != 0xFF )
				{
					const auto index_value = cpu.read_gpr ( src.mem.index );
					address += index_value * src.mem.scale;
				}

				address += src.mem.displacement;

				if ( !mem.read ( address, &value, static_cast< std::size_t >( size ) ) )
				{
					return false;
				}
			}
			else if ( src.type == operand_type::imm )
			{

				value = src.imm;
			}
			else
			{
				return false;
			}

			if ( dst.type == operand_type::reg )
			{
				if ( size == operand_size::dword )
				{
					value &= 0xFFFFFFFF;
				}

				cpu.write_gpr ( dst.reg, value );
			}
			else if ( dst.type == operand_type::mem )
			{
				std::uint64_t address = 0;

				if ( dst.mem.base != 0xFF )
				{
					address += cpu.read_gpr ( dst.mem.base );
				}

				if ( dst.mem.index != 0xFF )
				{
					address += cpu.read_gpr ( dst.mem.index ) * dst.mem.scale;
				}

				address += dst.mem.displacement;

				mem.write ( address, &value, static_cast< std::size_t >( size ) );
			}
			else
			{
				return false;
			}

			return true;
		}
	}

	namespace movsx
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			std::int64_t value = 0;

			if ( src.type == operand_type::reg )
			{

				const auto src_val = cpu.read_gpr ( src.reg );

				if ( src.size == operand_size::byte )
				{

					value = static_cast< std::int8_t >( src_val & 0xFF );
				}
				else if ( src.size == operand_size::word )
				{

					value = static_cast< std::int16_t >( src_val & 0xFFFF );
				}
				else if ( src.size == operand_size::dword )
				{

					value = static_cast< std::int32_t >( src_val & 0xFFFFFFFF );
				}
				else
				{
					return false;
				}
			}
			else if ( src.type == operand_type::mem )
			{

				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );

				if ( src.size == operand_size::byte )
				{

					std::int8_t byte_val = 0;
					if ( !mem.read ( address, &byte_val, 1 ) )
					{
						return false;
					}
					value = byte_val;
				}
				else if ( src.size == operand_size::word )
				{

					std::int16_t word_val = 0;
					if ( !mem.read ( address, &word_val, 2 ) )
					{
						return false;
					}
					value = word_val;
				}
				else if ( src.size == operand_size::dword )
				{

					std::int32_t dword_val = 0;
					if ( !mem.read ( address, &dword_val, 4 ) )
					{
						return false;
					}
					value = dword_val;
				}
				else
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

				cpu.write_gpr ( dst.reg, static_cast< std::uint64_t >( value ) );
			}
			else
			{
				return false;
			}

			return true;
		}
	}

	namespace movzx
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
				if ( !mem.read ( address, &value, static_cast< std::size_t >( src.size ) ) )
				{
					value = 0;
				}
			}
			else
			{
				return false;
			}

			if ( src.size == operand_size::byte )
			{

				value &= 0xFF;
			}
			else if ( src.size == operand_size::word )
			{

				value &= 0xFFFF;
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

	namespace lea
	{
		inline auto execute ( cpu_state& cpu, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::reg || src.type != operand_type::mem )
			{
				return false;
			}

			std::uint64_t address = 0;

			if ( src.mem.base != 0xFF )
			{
				std::uint64_t base_value = 0;

				if ( src.mem.base == 16 )
				{
					base_value = instr.address + instr.length;
				}
				else
				{
					base_value = cpu.read_gpr ( src.mem.base );
				}

				address += base_value;
			}

			if ( src.mem.index != 0xFF )
			{
				address += cpu.read_gpr ( src.mem.index ) * src.mem.scale;
			}

			address += src.mem.displacement;

			cpu.write_gpr ( dst.reg, address );

			return true;
		}
	}

	namespace push
	{
		inline auto execute_r64 ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 1 )
			{
				return false;
			}

			const auto& src = instr.operands[ 0 ];

			if ( src.type != operand_type::reg )
			{
				return false;
			}

			auto rsp = cpu.read_gpr ( 4 );
			rsp -= 8;

			const auto value = cpu.read_gpr ( src.reg );

			if ( !mem.write ( rsp, &value, sizeof ( std::uint64_t ) ) )
			{
				return false;
			}

			cpu.write_gpr ( 4, rsp );

			return true;
		}
	}

	namespace pop
	{
		inline auto execute_r64 ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
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

			auto rsp = cpu.read_gpr ( 4 );

			std::uint64_t value = 0;
			if ( !mem.read ( rsp, &value, sizeof ( std::uint64_t ) ) )
			{
				return false;
			}

			cpu.write_gpr ( dst.reg, value );

			rsp += 8;
			cpu.write_gpr ( 4, rsp );

			return true;
		}
	}

	namespace xchg
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& op1 = instr.operands[ 0 ];
			const auto& op2 = instr.operands[ 1 ];

			std::uint64_t val1 = 0;
			std::uint64_t val2 = 0;

			if ( op1.type == operand_type::reg )
			{
				val1 = cpu.read_gpr ( op1.reg );
			}
			else if ( op1.type == operand_type::mem )
			{
				const auto addr1 = cpu.calculate_memory_address ( op1.mem, instr.address + instr.length );
				if ( !mem.read ( addr1, &val1, static_cast< std::size_t >( op1.size ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			if ( op2.type == operand_type::reg )
			{
				val2 = cpu.read_gpr ( op2.reg );
			}
			else if ( op2.type == operand_type::mem )
			{
				const auto addr2 = cpu.calculate_memory_address ( op2.mem, instr.address + instr.length );
				if ( !mem.read ( addr2, &val2, static_cast< std::size_t >( op2.size ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			if ( op1.type == operand_type::reg )
			{
				cpu.write_gpr ( op1.reg, val2 );
			}
			else if ( op1.type == operand_type::mem )
			{
				const auto addr1 = cpu.calculate_memory_address ( op1.mem, instr.address + instr.length );
				if ( !mem.write ( addr1, &val2, static_cast< std::size_t >( op1.size ) ) )
				{
					return false;
				}
			}

			if ( op2.type == operand_type::reg )
			{
				cpu.write_gpr ( op2.reg, val1 );
			}
			else if ( op2.type == operand_type::mem )
			{
				const auto addr2 = cpu.calculate_memory_address ( op2.mem, instr.address + instr.length );
				if ( !mem.write ( addr2, &val1, static_cast< std::size_t >( op2.size ) ) )
				{
					return false;
				}
			}

			return true;
		}
	}
}