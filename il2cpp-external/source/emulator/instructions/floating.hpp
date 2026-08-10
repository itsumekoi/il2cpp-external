#pragma once
#include <Windows.h>
#include <cstdio>
#include <array>
#include "../state.hpp"
#include "../handler.hpp"
#include "../decoder.hpp"

namespace instructions
{
	namespace sse
	{
		inline auto ucomiss ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			float a = 0.00f, b = 0.00f;
			std::memcpy ( &a, cpu.read_xmm ( dst.reg ), sizeof ( float ) );

			if ( src.type == operand_type::xmm )
			{
				std::memcpy ( &b, cpu.read_xmm ( src.reg ), sizeof ( float ) );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &b, sizeof ( float ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			if ( a != a || b != b )
			{
				cpu.set_flag ( cpu_state::flag_zf, true );
				cpu.set_flag ( cpu_state::flag_pf, true );
				cpu.set_flag ( cpu_state::flag_cf, true );
			}
			else if ( a < b )
			{
				cpu.set_flag ( cpu_state::flag_zf, false );
				cpu.set_flag ( cpu_state::flag_pf, false );
				cpu.set_flag ( cpu_state::flag_cf, true );
			}
			else if ( a > b )
			{
				cpu.set_flag ( cpu_state::flag_zf, false );
				cpu.set_flag ( cpu_state::flag_pf, false );
				cpu.set_flag ( cpu_state::flag_cf, false );
			}
			else
			{
				cpu.set_flag ( cpu_state::flag_zf, true );
				cpu.set_flag ( cpu_state::flag_pf, false );
				cpu.set_flag ( cpu_state::flag_cf, false );
			}

			cpu.set_flag ( cpu_state::flag_of, false );
			cpu.set_flag ( cpu_state::flag_sf, false );
			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto comiss ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			return instructions::sse::ucomiss ( cpu, mem, instr );
		}

		inline auto ucomisd ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			double a = 0.0, b = 0.0;
			std::memcpy ( &a, cpu.read_xmm ( dst.reg ), sizeof ( double ) );

			if ( src.type == operand_type::xmm )
			{
				std::memcpy ( &b, cpu.read_xmm ( src.reg ), sizeof ( double ) );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &b, sizeof ( double ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			if ( a != a || b != b )
			{
				cpu.set_flag ( cpu_state::flag_zf, true );
				cpu.set_flag ( cpu_state::flag_pf, true );
				cpu.set_flag ( cpu_state::flag_cf, true );
			}
			else if ( a < b )
			{
				cpu.set_flag ( cpu_state::flag_zf, false );
				cpu.set_flag ( cpu_state::flag_pf, false );
				cpu.set_flag ( cpu_state::flag_cf, true );
			}
			else if ( a > b )
			{
				cpu.set_flag ( cpu_state::flag_zf, false );
				cpu.set_flag ( cpu_state::flag_pf, false );
				cpu.set_flag ( cpu_state::flag_cf, false );
			}
			else
			{
				cpu.set_flag ( cpu_state::flag_zf, true );
				cpu.set_flag ( cpu_state::flag_pf, false );
				cpu.set_flag ( cpu_state::flag_cf, false );
			}

			cpu.set_flag ( cpu_state::flag_of, false );
			cpu.set_flag ( cpu_state::flag_sf, false );
			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto comisd ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			return instructions::sse::ucomisd ( cpu, mem, instr );
		}

		inline auto pxor ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 16 ] = { };

			if ( src.type == operand_type::xmm )
			{
				std::memcpy ( src_data, cpu.read_xmm ( src.reg ), 16 );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, src_data, 16 ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			std::uint8_t dst_data[ 16 ];
			std::memcpy ( dst_data, cpu.read_xmm ( dst.reg ), 16 );

			std::uint8_t result[ 16 ];
			for ( int i = 0; i < 16; i++ )
			{
				result[ i ] = dst_data[ i ] ^ src_data[ i ];
			}

			cpu.write_xmm ( dst.reg, result );
			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto pand ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 16 ] = { };

			if ( src.type == operand_type::xmm )
			{
				std::memcpy ( src_data, cpu.read_xmm ( src.reg ), 16 );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, src_data, 16 ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			std::uint8_t dst_data[ 16 ];
			std::memcpy ( dst_data, cpu.read_xmm ( dst.reg ), 16 );

			std::uint8_t result[ 16 ];
			for ( int i = 0; i < 16; i++ )
			{
				result[ i ] = dst_data[ i ] & src_data[ i ];
			}

			cpu.write_xmm ( dst.reg, result );
			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto por ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 16 ] = { };

			if ( src.type == operand_type::xmm )
			{
				std::memcpy ( src_data, cpu.read_xmm ( src.reg ), 16 );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, src_data, 16 ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			std::uint8_t dst_data[ 16 ];
			std::memcpy ( dst_data, cpu.read_xmm ( dst.reg ), 16 );

			std::uint8_t result[ 16 ];
			for ( int i = 0; i < 16; i++ )
			{
				result[ i ] = dst_data[ i ] | src_data[ i ];
			}

			cpu.write_xmm ( dst.reg, result );
			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto movd ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type == operand_type::xmm )
			{
				std::uint8_t xmm_data[ 16 ] = { };

				if ( src.type == operand_type::reg )
				{
					const auto val = cpu.read_gpr ( src.reg );
					const auto size = static_cast< std::size_t >( src.size );
					std::memcpy ( xmm_data, &val, size );
				}
				else if ( src.type == operand_type::mem )
				{
					const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
					const auto size = static_cast< std::size_t >( src.size );
					if ( !mem.read ( address, xmm_data, size ) )
					{
						return false;
					}
				}
				else
				{
					return false;
				}

				cpu.write_xmm ( dst.xmm, xmm_data );
			}
			else if ( dst.type == operand_type::reg )
			{
				std::uint64_t val = 0;

				if ( src.type == operand_type::xmm )
				{
					const auto size = static_cast< std::size_t >( dst.size );
					std::memcpy ( &val, cpu.read_xmm ( src.xmm ), size );
				}
				else if ( src.type == operand_type::mem )
				{
					const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
					const auto size = static_cast< std::size_t >( dst.size );
					if ( !mem.read ( address, &val, size ) )
					{
						return false;
					}
				}
				else
				{
					return false;
				}

				cpu.write_gpr ( dst.reg, val );
			}
			else if ( dst.type == operand_type::mem )
			{
				std::uint64_t val = 0;

				if ( src.type == operand_type::xmm )
				{
					const auto size = static_cast< std::size_t >( dst.size );
					std::memcpy ( &val, cpu.read_xmm ( src.xmm ), size );
					const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
					if ( !mem.write ( address, &val, size ) )
					{
						return false;
					}
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

			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto andnps ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 16 ] = { };

			if ( src.type == operand_type::xmm )
			{
				std::memcpy ( src_data, cpu.read_xmm ( src.reg ), 16 );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, src_data, 16 ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			std::uint8_t dst_data[ 16 ];
			std::memcpy ( dst_data, cpu.read_xmm ( dst.reg ), 16 );

			std::uint8_t result[ 16 ];
			for ( int i = 0; i < 16; i++ )
			{
				result[ i ] = ( ~dst_data[ i ] ) & src_data[ i ];
			}

			cpu.write_xmm ( dst.reg, result );
			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto andnpd ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			return instructions::sse::andnps ( cpu, mem, instr );
		}

		inline auto addps ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 16 ] = { };

			if ( src.type == operand_type::xmm )
			{
				std::memcpy ( src_data, cpu.read_xmm ( src.reg ), 16 );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, src_data, 16 ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			float src_floats[ 4 ], dst_floats[ 4 ], result[ 4 ];
			std::memcpy ( src_floats, src_data, 16 );
			std::memcpy ( dst_floats, cpu.read_xmm ( dst.reg ), 16 );

			for ( int i = 0; i < 4; i++ )
			{
				result[ i ] = dst_floats[ i ] + src_floats[ i ];
			}

			std::uint8_t result_data[ 16 ];
			std::memcpy ( result_data, result, 16 );
			cpu.write_xmm ( dst.reg, result_data );

			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto subps ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 16 ] = { };

			if ( src.type == operand_type::xmm )
			{
				std::memcpy ( src_data, cpu.read_xmm ( src.reg ), 16 );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, src_data, 16 ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			float src_floats[ 4 ], dst_floats[ 4 ], result[ 4 ];
			std::memcpy ( src_floats, src_data, 16 );
			std::memcpy ( dst_floats, cpu.read_xmm ( dst.reg ), 16 );

			for ( int i = 0; i < 4; i++ )
			{
				result[ i ] = dst_floats[ i ] - src_floats[ i ];
			}

			std::uint8_t result_data[ 16 ];
			std::memcpy ( result_data, result, 16 );
			cpu.write_xmm ( dst.reg, result_data );

			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto divps ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 16 ] = { };

			if ( src.type == operand_type::xmm )
			{
				std::memcpy ( src_data, cpu.read_xmm ( src.reg ), 16 );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, src_data, 16 ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			float src_floats[ 4 ], dst_floats[ 4 ], result[ 4 ];
			std::memcpy ( src_floats, src_data, 16 );
			std::memcpy ( dst_floats, cpu.read_xmm ( dst.reg ), 16 );

			for ( int i = 0; i < 4; i++ )
			{
				if ( src_floats[ i ] == 0.00f )
				{
					const auto bits = *reinterpret_cast< std::uint32_t* > ( &dst_floats[ i ] );
					const std::uint32_t inf_bits = ( bits & 0x80000000 ) | 0x7F800000;
					std::memcpy ( &result[ i ], &inf_bits, 4 );
				}
				else
				{
					result[ i ] = dst_floats[ i ] / src_floats[ i ];
				}
			}

			std::uint8_t result_data[ 16 ];
			std::memcpy ( result_data, result, 16 );
			cpu.write_xmm ( dst.reg, result_data );

			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto mulps ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 16 ] = { };

			if ( src.type == operand_type::xmm )
			{
				std::memcpy ( src_data, cpu.read_xmm ( src.reg ), 16 );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, src_data, 16 ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			float src_floats[ 4 ], dst_floats[ 4 ], result[ 4 ];
			std::memcpy ( src_floats, src_data, 16 );
			std::memcpy ( dst_floats, cpu.read_xmm ( dst.reg ), 16 );

			for ( int i = 0; i < 4; i++ )
			{
				result[ i ] = dst_floats[ i ] * src_floats[ i ];
			}

			std::uint8_t result_data[ 16 ];
			std::memcpy ( result_data, result, 16 );
			cpu.write_xmm ( dst.reg, result_data );

			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto pshufd ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 3 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];
			const auto order = static_cast< std::uint8_t > ( instr.operands[ 2 ].imm );

			std::uint8_t src_data[ 16 ] = { };

			if ( src.type == operand_type::xmm )
			{
				std::memcpy ( src_data, cpu.read_xmm ( src.reg ), 16 );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, src_data, 16 ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			std::uint32_t lanes[ 4 ];
			std::memcpy ( lanes, src_data, 16 );

			std::uint32_t result[ 4 ];
			result[ 0 ] = lanes[ ( order >> 0 ) & 0x03 ];
			result[ 1 ] = lanes[ ( order >> 2 ) & 0x03 ];
			result[ 2 ] = lanes[ ( order >> 4 ) & 0x03 ];
			result[ 3 ] = lanes[ ( order >> 6 ) & 0x03 ];

			std::uint8_t result_data[ 16 ];
			std::memcpy ( result_data, result, 16 );
			cpu.write_xmm ( dst.reg, result_data );

			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto psrldq ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto  shift = static_cast< std::uint8_t > ( instr.operands[ 1 ].imm );

			std::uint8_t data[ 16 ] = { };
			std::memcpy ( data, cpu.read_xmm ( dst.reg ), 16 );

			std::uint8_t result[ 16 ] = { };
			if ( shift < 16 )
			{
				std::memcpy ( result, data + shift, 16 - shift );
			}

			cpu.write_xmm ( dst.reg, result );
			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto pslldq ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto  shift = static_cast< std::uint8_t > ( instr.operands[ 1 ].imm );

			std::uint8_t data[ 16 ] = { };
			std::memcpy ( data, cpu.read_xmm ( dst.reg ), 16 );

			std::uint8_t result[ 16 ] = { };
			if ( shift < 16 )
			{
				std::memcpy ( result + shift, data, 16 - shift );
			}

			cpu.write_xmm ( dst.reg, result );
			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto psrlq ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto  shift = static_cast< std::uint8_t > ( instr.operands[ 1 ].imm );

			std::uint8_t data[ 16 ] = { };
			std::memcpy ( data, cpu.read_xmm ( dst.reg ), 16 );

			std::uint64_t lo = 0, hi = 0;
			std::memcpy ( &lo, data, 8 );
			std::memcpy ( &hi, data + 8, 8 );

			lo = ( shift >= 64 ) ? 0 : lo >> shift;
			hi = ( shift >= 64 ) ? 0 : hi >> shift;

			std::memcpy ( data, &lo, 8 );
			std::memcpy ( data + 8, &hi, 8 );

			cpu.write_xmm ( dst.reg, data );
			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto psllq ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto  shift = static_cast< std::uint8_t > ( instr.operands[ 1 ].imm );

			std::uint8_t data[ 16 ] = { };
			std::memcpy ( data, cpu.read_xmm ( dst.reg ), 16 );

			std::uint64_t lo = 0, hi = 0;
			std::memcpy ( &lo, data, 8 );
			std::memcpy ( &hi, data + 8, 8 );

			lo = ( shift >= 64 ) ? 0 : lo << shift;
			hi = ( shift >= 64 ) ? 0 : hi << shift;

			std::memcpy ( data, &lo, 8 );
			std::memcpy ( data + 8, &hi, 8 );

			cpu.write_xmm ( dst.reg, data );
			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto psrld ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto  shift = static_cast< std::uint8_t > ( instr.operands[ 1 ].imm );

			std::uint8_t data[ 16 ] = { };
			std::memcpy ( data, cpu.read_xmm ( dst.reg ), 16 );

			for ( int i = 0; i < 4; i++ )
			{
				std::uint32_t lane = 0;
				std::memcpy ( &lane, data + i * 4, 4 );
				lane = ( shift >= 32 ) ? 0 : lane >> shift;
				std::memcpy ( data + i * 4, &lane, 4 );
			}

			cpu.write_xmm ( dst.reg, data );
			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto pslld ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto  shift = static_cast< std::uint8_t > ( instr.operands[ 1 ].imm );

			std::uint8_t data[ 16 ] = { };
			std::memcpy ( data, cpu.read_xmm ( dst.reg ), 16 );

			for ( int i = 0; i < 4; i++ )
			{
				std::uint32_t lane = 0;
				std::memcpy ( &lane, data + i * 4, 4 );
				lane = ( shift >= 32 ) ? 0 : lane << shift;
				std::memcpy ( data + i * 4, &lane, 4 );
			}

			cpu.write_xmm ( dst.reg, data );
			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto psrlw ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto  shift = static_cast< std::uint8_t > ( instr.operands[ 1 ].imm );

			std::uint8_t data[ 16 ] = { };
			std::memcpy ( data, cpu.read_xmm ( dst.reg ), 16 );

			for ( int i = 0; i < 8; i++ )
			{
				std::uint16_t lane = 0;
				std::memcpy ( &lane, data + i * 2, 2 );
				lane = ( shift >= 16 ) ? 0 : lane >> shift;
				std::memcpy ( data + i * 2, &lane, 2 );
			}

			cpu.write_xmm ( dst.reg, data );
			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto psllw ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto  shift = static_cast< std::uint8_t > ( instr.operands[ 1 ].imm );

			std::uint8_t data[ 16 ] = { };
			std::memcpy ( data, cpu.read_xmm ( dst.reg ), 16 );

			for ( int i = 0; i < 8; i++ )
			{
				std::uint16_t lane = 0;
				std::memcpy ( &lane, data + i * 2, 2 );
				lane = ( shift >= 16 ) ? 0 : lane << shift;
				std::memcpy ( data + i * 2, &lane, 2 );
			}

			cpu.write_xmm ( dst.reg, data );
			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto psrad ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto  shift = static_cast< std::uint8_t > ( instr.operands[ 1 ].imm );

			std::uint8_t data[ 16 ] = { };
			std::memcpy ( data, cpu.read_xmm ( dst.reg ), 16 );

			for ( int i = 0; i < 4; i++ )
			{
				std::int32_t lane = 0;
				std::memcpy ( &lane, data + i * 4, 4 );
				lane = ( shift >= 32 ) ? ( lane >> 31 ) : lane >> shift;
				std::memcpy ( data + i * 4, &lane, 4 );
			}

			cpu.write_xmm ( dst.reg, data );
			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto psraw ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto  shift = static_cast< std::uint8_t > ( instr.operands[ 1 ].imm );

			std::uint8_t data[ 16 ] = { };
			std::memcpy ( data, cpu.read_xmm ( dst.reg ), 16 );

			for ( int i = 0; i < 8; i++ )
			{
				std::int16_t lane = 0;
				std::memcpy ( &lane, data + i * 2, 2 );
				lane = ( shift >= 16 ) ? ( lane >> 15 ) : lane >> shift;
				std::memcpy ( data + i * 2, &lane, 2 );
			}

			cpu.write_xmm ( dst.reg, data );
			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto minss ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			float src_value = 0.00f;
			if ( src.type == operand_type::xmm )
			{
				const auto src_xmm = cpu.read_xmm ( src.reg );
				std::memcpy ( &src_value, src_xmm, sizeof ( float ) );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &src_value, sizeof ( float ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			float dst_value = 0.00f;
			const auto dst_xmm = cpu.read_xmm ( dst.reg );
			std::memcpy ( &dst_value, dst_xmm, sizeof ( float ) );

			const float result = std::fminf ( dst_value, src_value );

			std::uint8_t xmm_value[ 16 ];
			std::memcpy ( xmm_value, dst_xmm, 16 );
			std::memcpy ( xmm_value, &result, sizeof ( float ) );
			cpu.write_xmm ( dst.reg, xmm_value );

			return true;
		}

		inline auto minsd ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			double src_value = 0.0;
			if ( src.type == operand_type::xmm )
			{
				const auto src_xmm = cpu.read_xmm ( src.reg );
				std::memcpy ( &src_value, src_xmm, sizeof ( double ) );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &src_value, sizeof ( double ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			double dst_value = 0.0;
			const auto dst_xmm = cpu.read_xmm ( dst.reg );
			std::memcpy ( &dst_value, dst_xmm, sizeof ( double ) );

			const double result = std::fmin ( dst_value, src_value );

			std::uint8_t xmm_value[ 16 ];
			std::memcpy ( xmm_value, dst_xmm, 16 );
			std::memcpy ( xmm_value, &result, sizeof ( double ) );
			cpu.write_xmm ( dst.reg, xmm_value );

			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto minps ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 16 ] = { };

			if ( src.type == operand_type::xmm )
			{
				std::memcpy ( src_data, cpu.read_xmm ( src.reg ), 16 );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, src_data, 16 ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			float src_floats[ 4 ], dst_floats[ 4 ], result[ 4 ];
			std::memcpy ( src_floats, src_data, 16 );
			std::memcpy ( dst_floats, cpu.read_xmm ( dst.reg ), 16 );

			for ( int i = 0; i < 4; i++ )
			{
				result[ i ] = std::fminf ( dst_floats[ i ], src_floats[ i ] );
			}

			std::uint8_t result_data[ 16 ];
			std::memcpy ( result_data, result, 16 );
			cpu.write_xmm ( dst.reg, result_data );

			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto minpd ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 16 ] = { };

			if ( src.type == operand_type::xmm )
			{
				std::memcpy ( src_data, cpu.read_xmm ( src.reg ), 16 );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, src_data, 16 ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			double src_doubles[ 2 ], dst_doubles[ 2 ], result[ 2 ];
			std::memcpy ( src_doubles, src_data, 16 );
			std::memcpy ( dst_doubles, cpu.read_xmm ( dst.reg ), 16 );

			for ( int i = 0; i < 2; i++ )
			{
				result[ i ] = std::fmin ( dst_doubles[ i ], src_doubles[ i ] );
			}

			std::uint8_t result_data[ 16 ];
			std::memcpy ( result_data, result, 16 );
			cpu.write_xmm ( dst.reg, result_data );

			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto maxss ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			float src_value = 0.00f;
			if ( src.type == operand_type::xmm )
			{
				const auto src_xmm = cpu.read_xmm ( src.reg );
				std::memcpy ( &src_value, src_xmm, sizeof ( float ) );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &src_value, sizeof ( float ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			float dst_value = 0.00f;
			const auto dst_xmm = cpu.read_xmm ( dst.reg );
			std::memcpy ( &dst_value, dst_xmm, sizeof ( float ) );

			const float result = std::fmaxf ( dst_value, src_value );

			std::uint8_t xmm_value[ 16 ];
			std::memcpy ( xmm_value, dst_xmm, 16 );
			std::memcpy ( xmm_value, &result, sizeof ( float ) );
			cpu.write_xmm ( dst.reg, xmm_value );

			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto maxsd ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			double src_value = 0.0;
			if ( src.type == operand_type::xmm )
			{
				const auto src_xmm = cpu.read_xmm ( src.reg );
				std::memcpy ( &src_value, src_xmm, sizeof ( double ) );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &src_value, sizeof ( double ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			double dst_value = 0.0;
			const auto dst_xmm = cpu.read_xmm ( dst.reg );
			std::memcpy ( &dst_value, dst_xmm, sizeof ( double ) );

			const double result = std::fmax ( dst_value, src_value );

			std::uint8_t xmm_value[ 16 ];
			std::memcpy ( xmm_value, dst_xmm, 16 );
			std::memcpy ( xmm_value, &result, sizeof ( double ) );
			cpu.write_xmm ( dst.reg, xmm_value );

			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto maxps ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 16 ] = { };

			if ( src.type == operand_type::xmm )
			{
				std::memcpy ( src_data, cpu.read_xmm ( src.reg ), 16 );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, src_data, 16 ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			float src_floats[ 4 ], dst_floats[ 4 ], result[ 4 ];
			std::memcpy ( src_floats, src_data, 16 );
			std::memcpy ( dst_floats, cpu.read_xmm ( dst.reg ), 16 );

			for ( int i = 0; i < 4; i++ )
			{
				result[ i ] = std::fmaxf ( dst_floats[ i ], src_floats[ i ] );
			}

			std::uint8_t result_data[ 16 ];
			std::memcpy ( result_data, result, 16 );
			cpu.write_xmm ( dst.reg, result_data );

			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto maxpd ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 16 ] = { };

			if ( src.type == operand_type::xmm )
			{
				std::memcpy ( src_data, cpu.read_xmm ( src.reg ), 16 );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, src_data, 16 ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			double src_doubles[ 2 ], dst_doubles[ 2 ], result[ 2 ];
			std::memcpy ( src_doubles, src_data, 16 );
			std::memcpy ( dst_doubles, cpu.read_xmm ( dst.reg ), 16 );

			for ( int i = 0; i < 2; i++ )
			{
				result[ i ] = std::fmax ( dst_doubles[ i ], src_doubles[ i ] );
			}

			std::uint8_t result_data[ 16 ];
			std::memcpy ( result_data, result, 16 );
			cpu.write_xmm ( dst.reg, result_data );

			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto cvtps2dq ( cpu_state& cpu, memory_handler& mem, const instruction& instr ) -> bool
		{
			std::uint8_t src[ 16 ] = { };

			if ( instr.operands[ 1 ].type == operand_type::xmm )
			{
				const auto data = cpu.read_xmm ( instr.operands[ 1 ].xmm );
				if ( !data )
				{
					return false;
				}

				std::memcpy ( src, data, 16 );
			}
			else if ( instr.operands[ 1 ].type == operand_type::mem )
			{
				const auto addr = cpu.calculate_memory_address ( instr.operands[ 1 ].mem, instr.address + instr.length );
				if ( !mem.read ( addr, src, 16 ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			float floats[ 4 ];
			std::int32_t ints[ 4 ];
			std::memcpy ( floats, src, 16 );

			for ( int i = 0; i < 4; i++ )
			{
				const auto f = floats[ i ];
				if ( f != f || f > 2147483647.00f || f < -2147483648.00f )
				{
					ints[ i ] = static_cast< std::int32_t > ( 0x80000000 );
				}
				else
				{
					ints[ i ] = static_cast< std::int32_t > ( std::roundf ( f ) );
				}
			}

			std::uint8_t result[ 16 ] = { };
			std::memcpy ( result, ints, 16 );
			cpu.write_xmm ( instr.operands[ 0 ].xmm, result );

			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto cvttps2dq ( cpu_state& cpu, memory_handler& mem, const instruction& instr ) -> bool
		{
			std::uint8_t src[ 16 ] = { };

			if ( instr.operands[ 1 ].type == operand_type::xmm )
			{
				const auto data = cpu.read_xmm ( instr.operands[ 1 ].xmm );
				if ( !data )
				{
					return false;
				}

				std::memcpy ( src, data, 16 );
			}
			else if ( instr.operands[ 1 ].type == operand_type::mem )
			{
				const auto addr = cpu.calculate_memory_address ( instr.operands[ 1 ].mem, instr.address + instr.length );
				if ( !mem.read ( addr, src, 16 ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			float floats[ 4 ];
			std::int32_t ints[ 4 ];
			std::memcpy ( floats, src, 16 );

			for ( int i = 0; i < 4; i++ )
			{
				const auto f = floats[ i ];
				if ( f != f || f > 2147483647.00f || f < -2147483648.00f )
				{
					ints[ i ] = static_cast< std::int32_t > ( 0x80000000 );
				}
				else
				{
					ints[ i ] = static_cast< std::int32_t > ( f );
				}
			}

			std::uint8_t result[ 16 ] = { };
			std::memcpy ( result, ints, 16 );
			cpu.write_xmm ( instr.operands[ 0 ].xmm, result );

			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto cvtdq2ps ( cpu_state& cpu, memory_handler& mem, const instruction& instr ) -> bool
		{
			std::uint8_t src[ 16 ] = { };

			if ( instr.operands[ 1 ].type == operand_type::xmm )
			{
				const auto data = cpu.read_xmm ( instr.operands[ 1 ].xmm );
				if ( !data )
				{
					return false;
				}

				std::memcpy ( src, data, 16 );
			}
			else if ( instr.operands[ 1 ].type == operand_type::mem )
			{
				const auto addr = cpu.calculate_memory_address ( instr.operands[ 1 ].mem, instr.address + instr.length );
				if ( !mem.read ( addr, src, 16 ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			std::int32_t ints[ 4 ];
			float floats[ 4 ];
			std::memcpy ( ints, src, 16 );

			for ( int i = 0; i < 4; i++ )
			{
				floats[ i ] = static_cast< float > ( ints[ i ] );
			}

			std::uint8_t result[ 16 ] = { };
			std::memcpy ( result, floats, 16 );
			cpu.write_xmm ( instr.operands[ 0 ].xmm, result );

			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto addss ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			float src_value = 0.00f;
			if ( src.type == operand_type::xmm )
			{
				const auto src_xmm = cpu.read_xmm ( src.reg );
				std::memcpy ( &src_value, src_xmm, sizeof ( float ) );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &src_value, sizeof ( float ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			float dst_value = 0.00f;
			const auto dst_xmm = cpu.read_xmm ( dst.reg );
			std::memcpy ( &dst_value, dst_xmm, sizeof ( float ) );

			float result = dst_value + src_value;

			std::uint8_t xmm_value[ 16 ];
			std::memcpy ( xmm_value, dst_xmm, 16 );
			std::memcpy ( xmm_value, &result, sizeof ( float ) );
			cpu.write_xmm ( dst.reg, xmm_value );

			return true;
		}

		inline auto subss ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			float src_value = 0.00f;
			if ( src.type == operand_type::xmm )
			{
				const auto src_xmm = cpu.read_xmm ( src.reg );
				std::memcpy ( &src_value, src_xmm, sizeof ( float ) );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &src_value, sizeof ( float ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			float dst_value = 0.00f;
			const auto dst_xmm = cpu.read_xmm ( dst.reg );
			std::memcpy ( &dst_value, dst_xmm, sizeof ( float ) );

			float result = dst_value - src_value;

			std::uint8_t xmm_value[ 16 ];
			std::memcpy ( xmm_value, dst_xmm, 16 );
			std::memcpy ( xmm_value, &result, sizeof ( float ) );
			cpu.write_xmm ( dst.reg, xmm_value );

			return true;
		}

		inline auto mulss ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			float src_value = 0.00f;
			if ( src.type == operand_type::xmm )
			{
				const auto src_xmm = cpu.read_xmm ( src.reg );
				std::memcpy ( &src_value, src_xmm, sizeof ( float ) );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &src_value, sizeof ( float ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			float dst_value = 0.00f;
			const auto dst_xmm = cpu.read_xmm ( dst.reg );
			std::memcpy ( &dst_value, dst_xmm, sizeof ( float ) );

			float result = dst_value * src_value;

			std::uint8_t xmm_value[ 16 ];
			std::memcpy ( xmm_value, dst_xmm, 16 );
			std::memcpy ( xmm_value, &result, sizeof ( float ) );
			cpu.write_xmm ( dst.reg, xmm_value );

			return true;
		}

		inline auto divss ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			float src_value = 0.00f;
			if ( src.type == operand_type::xmm )
			{
				const auto src_xmm = cpu.read_xmm ( src.reg );
				std::memcpy ( &src_value, src_xmm, sizeof ( float ) );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &src_value, sizeof ( float ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			if ( src_value == 0.00f )
			{
				const float inf = std::numeric_limits< float >::infinity ( );
				std::uint8_t xmm_value[ 16 ];
				const auto dst_xmm = cpu.read_xmm ( dst.reg );
				std::memcpy ( xmm_value, dst_xmm, 16 );
				std::memcpy ( xmm_value, &inf, sizeof ( float ) );
				cpu.write_xmm ( dst.reg, xmm_value );
				return true;
			}

			float dst_value = 0.00f;
			const auto dst_xmm = cpu.read_xmm ( dst.reg );
			std::memcpy ( &dst_value, dst_xmm, sizeof ( float ) );

			float result = dst_value / src_value;

			std::uint8_t xmm_value[ 16 ];
			std::memcpy ( xmm_value, dst_xmm, 16 );
			std::memcpy ( xmm_value, &result, sizeof ( float ) );
			cpu.write_xmm ( dst.reg, xmm_value );

			return true;
		}

		inline auto sqrtss ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			float src_value = 0.00f;
			if ( src.type == operand_type::xmm )
			{
				const auto src_xmm = cpu.read_xmm ( src.reg );
				std::memcpy ( &src_value, src_xmm, sizeof ( float ) );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &src_value, sizeof ( float ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			float result = std::sqrt ( src_value );

			std::uint8_t xmm_value[ 16 ];
			const auto dst_xmm = cpu.read_xmm ( dst.reg );
			std::memcpy ( xmm_value, dst_xmm, 16 );
			std::memcpy ( xmm_value, &result, sizeof ( float ) );
			cpu.write_xmm ( dst.reg, xmm_value );

			return true;
		}

		inline auto rcpps ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 16 ] = { };

			if ( src.type == operand_type::xmm )
			{
				std::memcpy ( src_data, cpu.read_xmm ( src.reg ), 16 );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, src_data, 16 ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			float src_floats[ 4 ], result[ 4 ];
			std::memcpy ( src_floats, src_data, 16 );

			for ( int i = 0; i < 4; i++ )
			{
				if ( src_floats[ i ] == 0.00f )
				{
					const auto bits = *reinterpret_cast< std::uint32_t* > ( &src_floats[ i ] );
					const std::uint32_t inf_bits = ( bits & 0x80000000 ) | 0x7F800000;
					std::memcpy ( &result[ i ], &inf_bits, 4 );
				}
				else
				{
					result[ i ] = 1.00f / src_floats[ i ];
				}
			}

			std::uint8_t result_data[ 16 ];
			std::memcpy ( result_data, result, 16 );
			cpu.write_xmm ( dst.reg, result_data );

			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto rcpss ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			float src_value = 0.00f;
			if ( src.type == operand_type::xmm )
			{
				const auto src_xmm = cpu.read_xmm ( src.reg );
				std::memcpy ( &src_value, src_xmm, sizeof ( float ) );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &src_value, sizeof ( float ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			float result = 0.00f;
			if ( src_value == 0.00f )
			{
				const auto bits = *reinterpret_cast< std::uint32_t* > ( &src_value );
				const std::uint32_t inf_bits = ( bits & 0x80000000 ) | 0x7F800000;
				std::memcpy ( &result, &inf_bits, sizeof ( float ) );
			}
			else
			{
				result = 1.00f / src_value;
			}

			std::uint8_t xmm_value[ 16 ];
			const auto dst_xmm = cpu.read_xmm ( dst.reg );
			std::memcpy ( xmm_value, dst_xmm, 16 );
			std::memcpy ( xmm_value, &result, sizeof ( float ) );
			cpu.write_xmm ( dst.reg, xmm_value );

			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto addsd ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			double src_value = 0.0;
			if ( src.type == operand_type::xmm )
			{
				const auto src_xmm = cpu.read_xmm ( src.reg );
				std::memcpy ( &src_value, src_xmm, sizeof ( double ) );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &src_value, sizeof ( double ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			double dst_value = 0.0;
			const auto dst_xmm = cpu.read_xmm ( dst.reg );
			std::memcpy ( &dst_value, dst_xmm, sizeof ( double ) );

			double result = dst_value + src_value;

			std::uint8_t xmm_value[ 16 ];
			std::memcpy ( xmm_value, dst_xmm, 16 );
			std::memcpy ( xmm_value, &result, sizeof ( double ) );
			cpu.write_xmm ( dst.reg, xmm_value );

			return true;
		}

		inline auto subsd ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			double src_value = 0.0;
			if ( src.type == operand_type::xmm )
			{
				const auto src_xmm = cpu.read_xmm ( src.reg );
				std::memcpy ( &src_value, src_xmm, sizeof ( double ) );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &src_value, sizeof ( double ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			double dst_value = 0.0;
			const auto dst_xmm = cpu.read_xmm ( dst.reg );
			std::memcpy ( &dst_value, dst_xmm, sizeof ( double ) );

			double result = dst_value - src_value;

			std::uint8_t xmm_value[ 16 ];
			std::memcpy ( xmm_value, dst_xmm, 16 );
			std::memcpy ( xmm_value, &result, sizeof ( double ) );
			cpu.write_xmm ( dst.reg, xmm_value );

			return true;
		}

		inline auto mulsd ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			double src_value = 0.0;
			if ( src.type == operand_type::xmm )
			{
				const auto src_xmm = cpu.read_xmm ( src.reg );
				std::memcpy ( &src_value, src_xmm, sizeof ( double ) );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &src_value, sizeof ( double ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			double dst_value = 0.0;
			const auto dst_xmm = cpu.read_xmm ( dst.reg );
			std::memcpy ( &dst_value, dst_xmm, sizeof ( double ) );

			double result = dst_value * src_value;

			std::uint8_t xmm_value[ 16 ];
			std::memcpy ( xmm_value, dst_xmm, 16 );
			std::memcpy ( xmm_value, &result, sizeof ( double ) );
			cpu.write_xmm ( dst.reg, xmm_value );

			return true;
		}

		inline auto divsd ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			double src_value = 0.0;
			if ( src.type == operand_type::xmm )
			{
				const auto src_xmm = cpu.read_xmm ( src.reg );
				std::memcpy ( &src_value, src_xmm, sizeof ( double ) );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &src_value, sizeof ( double ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			if ( src_value == 0.0 )
			{
				return false;
			}

			double dst_value = 0.0;
			const auto dst_xmm = cpu.read_xmm ( dst.reg );
			std::memcpy ( &dst_value, dst_xmm, sizeof ( double ) );

			double result = dst_value / src_value;

			std::uint8_t xmm_value[ 16 ];
			std::memcpy ( xmm_value, dst_xmm, 16 );
			std::memcpy ( xmm_value, &result, sizeof ( double ) );
			cpu.write_xmm ( dst.reg, xmm_value );

			return true;
		}

		inline auto sqrtsd ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			double src_value = 0.0;
			if ( src.type == operand_type::xmm )
			{
				const auto src_xmm = cpu.read_xmm ( src.reg );
				std::memcpy ( &src_value, src_xmm, sizeof ( double ) );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &src_value, sizeof ( double ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			double result = std::sqrt ( src_value );

			std::uint8_t xmm_value[ 16 ];
			const auto dst_xmm = cpu.read_xmm ( dst.reg );
			std::memcpy ( xmm_value, dst_xmm, 16 );
			std::memcpy ( xmm_value, &result, sizeof ( double ) );
			cpu.write_xmm ( dst.reg, xmm_value );

			return true;
		}

		inline auto movss ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			float value = 0.00f;

			if ( src.type == operand_type::xmm )
			{
				const auto xmm_data = cpu.read_xmm ( src.xmm );
				std::memcpy ( &value, xmm_data, sizeof ( float ) );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &value, sizeof ( float ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			if ( dst.type == operand_type::xmm )
			{
				std::uint8_t xmm_data[ 16 ] = { 0 };
				std::memcpy ( xmm_data, &value, sizeof ( float ) );
				cpu.write_xmm ( dst.xmm, xmm_data );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.write ( address, &value, sizeof ( float ) ) )
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

		inline auto movsd ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			double value = 0.0;

			if ( src.type == operand_type::xmm )
			{
				const auto xmm_data = cpu.read_xmm ( src.xmm );
				std::memcpy ( &value, xmm_data, sizeof ( double ) );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &value, sizeof ( double ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			if ( dst.type == operand_type::xmm )
			{
				std::uint8_t xmm_data[ 16 ] = { 0 };
				std::memcpy ( xmm_data, &value, sizeof ( double ) );
				cpu.write_xmm ( dst.xmm, xmm_data );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.write ( address, &value, sizeof ( double ) ) )
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

		inline auto movq ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			std::uint64_t value = 0;

			if ( src.type == operand_type::xmm )
			{
				const auto* xmm_data = cpu.read_xmm ( src.xmm );
				std::memcpy ( &value, xmm_data, 8 );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &value, 8 ) )
				{
					return false;
				}
			}
			else if ( src.type == operand_type::reg )
			{
				value = cpu.read_gpr ( src.reg );
			}
			else
			{
				return false;
			}

			if ( dst.type == operand_type::xmm )
			{
				std::uint8_t xmm_data[ 16 ] = { 0 };
				std::memcpy ( xmm_data, &value, 8 );
				cpu.write_xmm ( dst.xmm, xmm_data );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.write ( address, &value, 8 ) )
				{
					return false;
				}
			}
			else if ( dst.type == operand_type::reg )
			{
				cpu.write_gpr ( dst.reg, value );
			}
			else
			{
				return false;
			}

			return true;
		}

		inline auto movups ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			std::uint8_t data[ 16 ] = { };

			if ( src.type == operand_type::xmm )
			{
				const auto* xmm_data = cpu.read_xmm ( src.xmm );
				std::memcpy ( data, xmm_data, 16 );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				mem.read ( address, data, 16 );
			}
			else
			{
				return false;
			}

			if ( dst.type == operand_type::xmm )
			{
				cpu.write_xmm ( dst.xmm, data );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				mem.write ( address, data, 16 );
			}
			else
			{
				return false;
			}

			return true;
		}

		inline auto movaps ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			return instructions::sse::movups ( cpu, mem, instr );
		}

		inline auto movupd ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			return instructions::sse::movups ( cpu, mem, instr );
		}

		inline auto movapd ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			return instructions::sse::movups ( cpu, mem, instr );
		}

		inline auto movdqa ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			std::uint8_t data[ 16 ];

			if ( src.type == operand_type::xmm )
			{
				const auto* xmm_data = cpu.read_xmm ( src.xmm );
				std::memcpy ( data, xmm_data, 16 );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, data, 16 ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			if ( dst.type == operand_type::xmm )
			{
				cpu.write_xmm ( dst.xmm, data );
			}
			else if ( dst.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.write ( address, data, 16 ) )
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

		inline auto movdqu ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			return instructions::sse::movdqa ( cpu, mem, instr );
		}

		inline auto movlps ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( src.type == operand_type::xmm )
			{
				std::uint64_t value = 0;
				const auto* xmm_data = cpu.read_xmm ( src.xmm );
				std::memcpy ( &value, xmm_data + 8, 8 );

				std::uint8_t dst_data[ 16 ];
				std::memcpy ( dst_data, cpu.read_xmm ( dst.xmm ), 16 );
				std::memcpy ( dst_data, &value, 8 );
				cpu.write_xmm ( dst.xmm, dst_data );
			}
			else if ( src.type == operand_type::mem )
			{
				std::uint64_t value = 0;
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &value, 8 ) )
				{
					return false;
				}

				std::uint8_t dst_data[ 16 ];
				std::memcpy ( dst_data, cpu.read_xmm ( dst.xmm ), 16 );
				std::memcpy ( dst_data, &value, 8 );
				cpu.write_xmm ( dst.xmm, dst_data );
			}
			else if ( dst.type == operand_type::mem )
			{
				std::uint64_t value = 0;
				std::memcpy ( &value, cpu.read_xmm ( src.xmm ), 8 );
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.write ( address, &value, 8 ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto movhps ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( src.type == operand_type::xmm )
			{
				std::uint64_t value = 0;
				const auto* xmm_data = cpu.read_xmm ( src.xmm );
				std::memcpy ( &value, xmm_data, 8 );

				std::uint8_t dst_data[ 16 ];
				std::memcpy ( dst_data, cpu.read_xmm ( dst.xmm ), 16 );
				std::memcpy ( dst_data + 8, &value, 8 );
				cpu.write_xmm ( dst.xmm, dst_data );
			}
			else if ( src.type == operand_type::mem )
			{
				std::uint64_t value = 0;
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &value, 8 ) )
				{
					return false;
				}

				std::uint8_t dst_data[ 16 ];
				std::memcpy ( dst_data, cpu.read_xmm ( dst.xmm ), 16 );
				std::memcpy ( dst_data + 8, &value, 8 );
				cpu.write_xmm ( dst.xmm, dst_data );
			}
			else if ( dst.type == operand_type::mem )
			{
				std::uint64_t value = 0;
				std::memcpy ( &value, cpu.read_xmm ( src.xmm ) + 8, 8 );
				const auto address = cpu.calculate_memory_address ( dst.mem, instr.address + instr.length );
				if ( !mem.write ( address, &value, 8 ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto cvtsi2ss ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			std::int64_t int_value = 0;
			if ( src.type == operand_type::reg )
			{
				int_value = static_cast< std::int64_t >( cpu.read_gpr ( src.reg ) );

				if ( src.size == operand_size::dword )
				{
					int_value = static_cast< std::int32_t >( int_value );
				}
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );

				if ( src.size == operand_size::qword )
				{
					if ( !mem.read ( address, &int_value, 8 ) )
					{
						return false;
					}
				}
				else
				{
					std::int32_t val32 = 0;
					if ( !mem.read ( address, &val32, 4 ) )
					{
						return false;
					}
					int_value = val32;
				}
			}
			else
			{
				return false;
			}

			float float_val = static_cast< float >( int_value );

			std::uint8_t xmm_value[ 16 ];
			std::memcpy ( xmm_value, cpu.read_xmm ( dst.reg ), 16 );
			std::memcpy ( xmm_value, &float_val, sizeof ( float ) );
			cpu.write_xmm ( dst.reg, xmm_value );

			return true;
		}

		inline auto cvtsi2sd ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			std::int64_t int_value = 0;
			if ( src.type == operand_type::reg )
			{
				int_value = static_cast< std::int64_t >( cpu.read_gpr ( src.reg ) );

				if ( src.size == operand_size::dword )
				{
					int_value = static_cast< std::int32_t >( int_value );
				}
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );

				if ( src.size == operand_size::qword )
				{
					if ( !mem.read ( address, &int_value, 8 ) )
					{
						return false;
					}
				}
				else
				{
					std::int32_t val32 = 0;
					if ( !mem.read ( address, &val32, 4 ) )
					{
						return false;
					}
					int_value = val32;
				}
			}
			else
			{
				return false;
			}

			double double_val = static_cast< double >( int_value );

			std::uint8_t xmm_value[ 16 ];
			std::memcpy ( xmm_value, cpu.read_xmm ( dst.reg ), 16 );
			std::memcpy ( xmm_value, &double_val, sizeof ( double ) );
			cpu.write_xmm ( dst.reg, xmm_value );

			return true;
		}

		inline auto cvtss2sd ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			float float_val = 0.00f;
			if ( src.type == operand_type::xmm )
			{
				const auto src_xmm = cpu.read_xmm ( src.xmm );
				std::memcpy ( &float_val, src_xmm, sizeof ( float ) );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &float_val, sizeof ( float ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			double double_val = static_cast< double >( float_val );

			std::uint8_t dst_xmm[ 16 ];
			std::memcpy ( dst_xmm, cpu.read_xmm ( dst.xmm ), 16 );
			std::memcpy ( dst_xmm, &double_val, sizeof ( double ) );
			cpu.write_xmm ( dst.xmm, dst_xmm );

			return true;
		}

		inline auto cvtsd2ss ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			double double_val = 0.0;
			if ( src.type == operand_type::xmm )
			{
				const auto src_xmm = cpu.read_xmm ( src.xmm );
				std::memcpy ( &double_val, src_xmm, sizeof ( double ) );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &double_val, sizeof ( double ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			float float_val = static_cast< float >( double_val );

			std::uint8_t dst_xmm[ 16 ];
			std::memcpy ( dst_xmm, cpu.read_xmm ( dst.xmm ), 16 );
			std::memcpy ( dst_xmm, &float_val, sizeof ( float ) );
			cpu.write_xmm ( dst.xmm, dst_xmm );

			return true;
		}

		inline auto cvtpd2ps ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 || instr.operands[ 0 ].type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 16 ] = { };

			if ( instr.operands[ 1 ].type == operand_type::xmm )
			{
				const auto* xmm = cpu.read_xmm ( instr.operands[ 1 ].xmm );
				if ( !xmm )
				{
					return false;
				}
				std::memcpy ( src_data, xmm, 16 );
			}
			else if ( instr.operands[ 1 ].type == operand_type::mem )
			{
				const auto addr = cpu.calculate_memory_address ( instr.operands[ 1 ].mem, instr.address + instr.length );
				if ( !mem.read ( addr, src_data, 16 ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			double src_doubles[ 2 ];
			std::memcpy ( src_doubles, src_data, 16 );

			float result_floats[ 2 ];
			result_floats[ 0 ] = static_cast< float > ( src_doubles[ 0 ] );
			result_floats[ 1 ] = static_cast< float > ( src_doubles[ 1 ] );

			std::uint8_t result[ 16 ] = { };
			std::memcpy ( result, result_floats, 8 );

			cpu.write_xmm ( instr.operands[ 0 ].xmm, result );
			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto cvtps2pd ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 || instr.operands[ 0 ].type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 8 ] = { };

			if ( instr.operands[ 1 ].type == operand_type::xmm )
			{
				const auto* xmm = cpu.read_xmm ( instr.operands[ 1 ].xmm );
				if ( !xmm )
				{
					return false;
				}
				std::memcpy ( src_data, xmm, 8 );
			}
			else if ( instr.operands[ 1 ].type == operand_type::mem )
			{
				const auto addr = cpu.calculate_memory_address ( instr.operands[ 1 ].mem, instr.address + instr.length );
				if ( !mem.read ( addr, src_data, 8 ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			float src_floats[ 2 ];
			std::memcpy ( src_floats, src_data, 8 );

			double result_doubles[ 2 ];
			result_doubles[ 0 ] = static_cast< double > ( src_floats[ 0 ] );
			result_doubles[ 1 ] = static_cast< double > ( src_floats[ 1 ] );

			std::uint8_t result[ 16 ] = { };
			std::memcpy ( result, result_doubles, 16 );

			cpu.write_xmm ( instr.operands[ 0 ].xmm, result );
			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto vcvtss2sd ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 3 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src1 = instr.operands[ 1 ];
			const auto& src2 = instr.operands[ 2 ];

			if ( dst.type != operand_type::xmm || src1.type != operand_type::xmm )
			{
				return false;
			}

			float float_val = 0.00f;
			if ( src2.type == operand_type::xmm )
			{
				const auto src_xmm = cpu.read_xmm ( src2.xmm );
				std::memcpy ( &float_val, src_xmm, sizeof ( float ) );
			}
			else if ( src2.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src2.mem, instr.address + instr.length );
				if ( !mem.read ( address, &float_val, sizeof ( float ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			const double double_val = static_cast< double >( float_val );

			std::uint8_t dst_xmm[ 16 ];
			std::memcpy ( dst_xmm, cpu.read_xmm ( src1.xmm ), 16 );
			std::memcpy ( dst_xmm, &double_val, sizeof ( double ) );
			cpu.write_xmm ( dst.xmm, dst_xmm );

			return true;
		}

		inline auto vcvtsd2ss ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 3 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src1 = instr.operands[ 1 ];
			const auto& src2 = instr.operands[ 2 ];

			if ( dst.type != operand_type::xmm || src1.type != operand_type::xmm )
			{
				return false;
			}

			double double_val = 0.0;
			if ( src2.type == operand_type::xmm )
			{
				const auto src_xmm = cpu.read_xmm ( src2.xmm );
				std::memcpy ( &double_val, src_xmm, sizeof ( double ) );
			}
			else if ( src2.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src2.mem, instr.address + instr.length );
				if ( !mem.read ( address, &double_val, sizeof ( double ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			const float float_val = static_cast< float >( double_val );

			std::uint8_t dst_xmm[ 16 ];
			std::memcpy ( dst_xmm, cpu.read_xmm ( src1.xmm ), 16 );
			std::memcpy ( dst_xmm, &float_val, sizeof ( float ) );
			cpu.write_xmm ( dst.xmm, dst_xmm );

			return true;
		}

		inline auto cvttss2si ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::reg )
			{
				return false;
			}

			float float_val = 0.00f;
			if ( src.type == operand_type::xmm )
			{
				const auto src_xmm = cpu.read_xmm ( src.reg );
				std::memcpy ( &float_val, src_xmm, sizeof ( float ) );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &float_val, sizeof ( float ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			std::int64_t int_val = static_cast< std::int64_t >( float_val );
			cpu.write_gpr ( dst.reg, static_cast< std::uint64_t >( int_val ) );

			return true;
		}

		inline auto cvttsd2si ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::reg )
			{
				return false;
			}

			double double_val = 0.0;
			if ( src.type == operand_type::xmm )
			{
				const auto src_xmm = cpu.read_xmm ( src.reg );
				std::memcpy ( &double_val, src_xmm, sizeof ( double ) );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &double_val, sizeof ( double ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			std::int64_t int_val = static_cast< std::int64_t >( double_val );
			cpu.write_gpr ( dst.reg, static_cast< std::uint64_t >( int_val ) );

			return true;
		}

		inline auto xorps ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			std::uint8_t src_value[ 16 ] = { 0 };
			if ( src.type == operand_type::xmm )
			{
				std::memcpy ( src_value, cpu.read_xmm ( src.reg ), 16 );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, src_value, 16 ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			std::uint8_t dst_value[ 16 ] = { 0 };
			if ( dst.type == operand_type::xmm )
			{
				std::memcpy ( dst_value, cpu.read_xmm ( dst.reg ), 16 );
			}
			else
			{
				return false;
			}

			std::uint8_t result[ 16 ];
			for ( int i = 0; i < 16; i++ )
			{
				result[ i ] = dst_value[ i ] ^ src_value[ i ];
			}

			cpu.write_xmm ( dst.reg, result );

			return true;
		}

		inline auto xorpd ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			return instructions::sse::xorps ( cpu, mem, instr );
		}

		inline auto andps ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			std::uint8_t src_value[ 16 ] = { 0 };
			if ( src.type == operand_type::xmm )
			{
				std::memcpy ( src_value, cpu.read_xmm ( src.reg ), 16 );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, src_value, 16 ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			std::uint8_t dst_value[ 16 ] = { 0 };
			if ( dst.type == operand_type::xmm )
			{
				std::memcpy ( dst_value, cpu.read_xmm ( dst.reg ), 16 );
			}
			else
			{
				return false;
			}

			std::uint8_t result[ 16 ];
			for ( int i = 0; i < 16; i++ )
			{
				result[ i ] = dst_value[ i ] & src_value[ i ];
			}

			cpu.write_xmm ( dst.reg, result );

			return true;
		}

		inline auto andpd ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			return instructions::sse::andps ( cpu, mem, instr );
		}

		inline auto orps ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			std::uint8_t src_value[ 16 ] = { 0 };
			if ( src.type == operand_type::xmm )
			{
				std::memcpy ( src_value, cpu.read_xmm ( src.reg ), 16 );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, src_value, 16 ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			std::uint8_t dst_value[ 16 ] = { 0 };
			if ( dst.type == operand_type::xmm )
			{
				std::memcpy ( dst_value, cpu.read_xmm ( dst.reg ), 16 );
			}
			else
			{
				return false;
			}

			std::uint8_t result[ 16 ];
			for ( int i = 0; i < 16; i++ )
			{
				result[ i ] = dst_value[ i ] | src_value[ i ];
			}

			cpu.write_xmm ( dst.reg, result );

			return true;
		}

		inline auto orpd ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			return instructions::sse::orps ( cpu, mem, instr );
		}

		inline auto shufps ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 3 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];
			const auto order = static_cast< std::uint8_t > ( instr.operands[ 2 ].imm );

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 16 ];
			if ( src.type == operand_type::xmm )
			{
				std::memcpy ( src_data, cpu.read_xmm ( src.xmm ), 16 );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, src_data, 16 ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			const auto* dst_data = cpu.read_xmm ( dst.xmm );

			float dst_floats[ 4 ], src_floats[ 4 ], result[ 4 ];
			std::memcpy ( dst_floats, dst_data, 16 );
			std::memcpy ( src_floats, src_data, 16 );

			result[ 0 ] = dst_floats[ ( order >> 0 ) & 0x03 ];
			result[ 1 ] = dst_floats[ ( order >> 2 ) & 0x03 ];
			result[ 2 ] = src_floats[ ( order >> 4 ) & 0x03 ];
			result[ 3 ] = src_floats[ ( order >> 6 ) & 0x03 ];

			std::uint8_t result_data[ 16 ];
			std::memcpy ( result_data, result, 16 );
			cpu.write_xmm ( dst.xmm, result_data );

			return true;
		}

		inline auto shufpd ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 3 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];
			const auto order = static_cast< std::uint8_t > ( instr.operands[ 2 ].imm );

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 16 ];
			if ( src.type == operand_type::xmm )
			{
				std::memcpy ( src_data, cpu.read_xmm ( src.xmm ), 16 );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, src_data, 16 ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			const auto* dst_data = cpu.read_xmm ( dst.xmm );

			double dst_doubles[ 2 ], src_doubles[ 2 ], result[ 2 ];
			std::memcpy ( dst_doubles, dst_data, 16 );
			std::memcpy ( src_doubles, src_data, 16 );

			result[ 0 ] = dst_doubles[ ( order >> 0 ) & 0x01 ];
			result[ 1 ] = src_doubles[ ( order >> 1 ) & 0x01 ];

			std::uint8_t result_data[ 16 ];
			std::memcpy ( result_data, result, 16 );
			cpu.write_xmm ( dst.xmm, result_data );

			return true;
		}

		inline auto unpcklps ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 16 ];
			if ( src.type == operand_type::xmm )
			{
				std::memcpy ( src_data, cpu.read_xmm ( src.xmm ), 16 );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, src_data, 16 ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			const auto* dst_data = cpu.read_xmm ( dst.xmm );

			float dst_floats[ 4 ], src_floats[ 4 ], result[ 4 ];
			std::memcpy ( dst_floats, dst_data, 16 );
			std::memcpy ( src_floats, src_data, 16 );

			result[ 0 ] = dst_floats[ 0 ];
			result[ 1 ] = src_floats[ 0 ];
			result[ 2 ] = dst_floats[ 1 ];
			result[ 3 ] = src_floats[ 1 ];

			std::uint8_t result_data[ 16 ];
			std::memcpy ( result_data, result, 16 );
			cpu.write_xmm ( dst.xmm, result_data );

			return true;
		}

		inline auto unpckhps ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 16 ];
			if ( src.type == operand_type::xmm )
			{
				std::memcpy ( src_data, cpu.read_xmm ( src.xmm ), 16 );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, src_data, 16 ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			const auto* dst_data = cpu.read_xmm ( dst.xmm );

			float dst_floats[ 4 ], src_floats[ 4 ], result[ 4 ];
			std::memcpy ( dst_floats, dst_data, 16 );
			std::memcpy ( src_floats, src_data, 16 );

			result[ 0 ] = dst_floats[ 2 ];
			result[ 1 ] = src_floats[ 2 ];
			result[ 2 ] = dst_floats[ 3 ];
			result[ 3 ] = src_floats[ 3 ];

			std::uint8_t result_data[ 16 ];
			std::memcpy ( result_data, result, 16 );
			cpu.write_xmm ( dst.xmm, result_data );

			return true;
		}

		inline auto cmpps ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 3 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];
			const auto predicate = static_cast< std::uint8_t > ( instr.operands[ 2 ].imm ) & 0x07;

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 16 ] = { };

			if ( src.type == operand_type::xmm )
			{
				std::memcpy ( src_data, cpu.read_xmm ( src.xmm ), 16 );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, src_data, 16 ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			float dst_floats[ 4 ], src_floats[ 4 ];
			std::memcpy ( dst_floats, cpu.read_xmm ( dst.xmm ), 16 );
			std::memcpy ( src_floats, src_data, 16 );

			std::uint32_t result[ 4 ];
			for ( int i = 0; i < 4; i++ )
			{
				const float a = dst_floats[ i ];
				const float b = src_floats[ i ];
				bool cmp = false;
				switch ( predicate )
				{
					case 0: cmp = ( a == b );          break;
					case 1: cmp = ( a < b );          break;
					case 2: cmp = ( a <= b );          break;
					case 3: cmp = ( a != a || b != b ); break;
					case 4: cmp = ( a != b );          break;
					case 5: cmp = !( a < b );         break;
					case 6: cmp = !( a <= b );         break;
					case 7: cmp = !( a != a || b != b ); break;
				}
				result[ i ] = cmp ? 0xFFFFFFFF : 0x00000000;
			}

			std::uint8_t result_data[ 16 ];
			std::memcpy ( result_data, result, 16 );
			cpu.write_xmm ( dst.xmm, result_data );

			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto cmppd ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 3 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];
			const auto predicate = static_cast< std::uint8_t > ( instr.operands[ 2 ].imm ) & 0x07;

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 16 ] = { };

			if ( src.type == operand_type::xmm )
			{
				std::memcpy ( src_data, cpu.read_xmm ( src.xmm ), 16 );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, src_data, 16 ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			double dst_doubles[ 2 ], src_doubles[ 2 ];
			std::memcpy ( dst_doubles, cpu.read_xmm ( dst.xmm ), 16 );
			std::memcpy ( src_doubles, src_data, 16 );

			std::uint64_t result[ 2 ];
			for ( int i = 0; i < 2; i++ )
			{
				const double a = dst_doubles[ i ];
				const double b = src_doubles[ i ];
				bool cmp = false;
				switch ( predicate )
				{
					case 0: cmp = ( a == b );          break;
					case 1: cmp = ( a < b );          break;
					case 2: cmp = ( a <= b );          break;
					case 3: cmp = ( a != a || b != b ); break;
					case 4: cmp = ( a != b );          break;
					case 5: cmp = !( a < b );         break;
					case 6: cmp = !( a <= b );         break;
					case 7: cmp = !( a != a || b != b ); break;
				}
				result[ i ] = cmp ? 0xFFFFFFFFFFFFFFFFULL : 0x0000000000000000ULL;
			}

			std::uint8_t result_data[ 16 ];
			std::memcpy ( result_data, result, 16 );
			cpu.write_xmm ( dst.xmm, result_data );

			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto cmpss ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 3 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];
			const auto predicate = static_cast< std::uint8_t > ( instr.operands[ 2 ].imm ) & 0x07;

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			float src_value = 0.00f;
			if ( src.type == operand_type::xmm )
			{
				const auto src_xmm = cpu.read_xmm ( src.xmm );
				std::memcpy ( &src_value, src_xmm, sizeof ( float ) );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &src_value, sizeof ( float ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			float dst_value = 0.00f;
			const auto dst_xmm = cpu.read_xmm ( dst.xmm );
			std::memcpy ( &dst_value, dst_xmm, sizeof ( float ) );

			const float a = dst_value;
			const float b = src_value;
			bool cmp = false;
			switch ( predicate )
			{
				case 0: cmp = ( a == b );          break;
				case 1: cmp = ( a < b );          break;
				case 2: cmp = ( a <= b );          break;
				case 3: cmp = ( a != a || b != b ); break;
				case 4: cmp = ( a != b );          break;
				case 5: cmp = !( a < b );         break;
				case 6: cmp = !( a <= b );         break;
				case 7: cmp = !( a != a || b != b ); break;
			}

			const std::uint32_t mask = cmp ? 0xFFFFFFFF : 0x00000000;

			std::uint8_t xmm_value[ 16 ];
			std::memcpy ( xmm_value, dst_xmm, 16 );
			std::memcpy ( xmm_value, &mask, sizeof ( std::uint32_t ) );
			cpu.write_xmm ( dst.xmm, xmm_value );

			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto cmpsd ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 3 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];
			const auto predicate = static_cast< std::uint8_t > ( instr.operands[ 2 ].imm ) & 0x07;

			if ( dst.type != operand_type::xmm )
			{
				return false;
			}

			double src_value = 0.0;
			if ( src.type == operand_type::xmm )
			{
				const auto src_xmm = cpu.read_xmm ( src.xmm );
				std::memcpy ( &src_value, src_xmm, sizeof ( double ) );
			}
			else if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				if ( !mem.read ( address, &src_value, sizeof ( double ) ) )
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			double dst_value = 0.0;
			const auto dst_xmm = cpu.read_xmm ( dst.xmm );
			std::memcpy ( &dst_value, dst_xmm, sizeof ( double ) );

			const double a = dst_value;
			const double b = src_value;
			bool cmp = false;
			switch ( predicate )
			{
				case 0: cmp = ( a == b );          break;
				case 1: cmp = ( a < b );          break;
				case 2: cmp = ( a <= b );          break;
				case 3: cmp = ( a != a || b != b ); break;
				case 4: cmp = ( a != b );          break;
				case 5: cmp = !( a < b );         break;
				case 6: cmp = !( a <= b );         break;
				case 7: cmp = !( a != a || b != b ); break;
			}

			const std::uint64_t mask = cmp ? 0xFFFFFFFFFFFFFFFFULL : 0x0000000000000000ULL;

			std::uint8_t xmm_value[ 16 ];
			std::memcpy ( xmm_value, dst_xmm, 16 );
			std::memcpy ( xmm_value, &mask, sizeof ( std::uint64_t ) );
			cpu.write_xmm ( dst.xmm, xmm_value );

			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto movmskps ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::reg || src.type != operand_type::xmm )
			{
				return false;
			}

			float src_floats[ 4 ];
			std::memcpy ( src_floats, cpu.read_xmm ( src.xmm ), 16 );

			std::uint32_t mask = 0;
			for ( int i = 0; i < 4; i++ )
			{
				std::uint32_t bits;
				std::memcpy ( &bits, &src_floats[ i ], sizeof ( std::uint32_t ) );
				if ( bits & 0x80000000u )
				{
					mask |= ( 1u << i );
				}
			}

			cpu.write_gpr ( dst.reg, static_cast< std::uint64_t > ( mask ) );

			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto movmskpd ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::reg || src.type != operand_type::xmm )
			{
				return false;
			}

			std::uint64_t src_doubles[ 2 ];
			std::memcpy ( src_doubles, cpu.read_xmm ( src.xmm ), 16 );

			std::uint32_t mask = 0;
			for ( int i = 0; i < 2; i++ )
			{
				if ( src_doubles[ i ] & 0x8000000000000000ULL )
				{
					mask |= ( 1u << i );
				}
			}

			cpu.write_gpr ( dst.reg, static_cast< std::uint64_t > ( mask ) );

			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto pmovmskb ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src = instr.operands[ 1 ];

			if ( dst.type != operand_type::reg || src.type != operand_type::xmm )
			{
				return false;
			}

			const auto* src_bytes = cpu.read_xmm ( src.xmm );

			std::uint32_t mask = 0;
			for ( int i = 0; i < 16; i++ )
			{
				if ( src_bytes[ i ] & 0x80u )
				{
					mask |= ( 1u << i );
				}
			}

			cpu.write_gpr ( dst.reg, static_cast< std::uint64_t > ( mask ) );

			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto read_xmm_or_mem ( cpu_state& cpu, const memory_handler& mem, const instruction& instr, const operand& src, std::uint8_t ( &out )[ 16 ] ) -> bool
		{
			if ( src.type == operand_type::xmm )
			{
				std::memcpy ( out, cpu.read_xmm ( src.xmm ), 16 );
				return true;
			}

			if ( src.type == operand_type::mem )
			{
				const auto address = cpu.calculate_memory_address ( src.mem, instr.address + instr.length );
				return mem.read ( address, out, 16 );
			}

			return false;
		}

		inline auto punpcklbw ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 || instr.operands[ 0 ].type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 16 ] = { };
			if ( !instructions::sse::read_xmm_or_mem ( cpu, mem, instr, instr.operands[ 1 ], src_data ) )
			{
				return false;
			}

			std::uint8_t dst_data[ 16 ];
			std::memcpy ( dst_data, cpu.read_xmm ( instr.operands[ 0 ].xmm ), 16 );

			std::uint8_t result[ 16 ];
			for ( int i = 0; i < 8; i++ )
			{
				result[ i * 2 ] = dst_data[ i ];
				result[ i * 2 + 1 ] = src_data[ i ];
			}

			cpu.write_xmm ( instr.operands[ 0 ].xmm, result );
			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto punpcklwd ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 || instr.operands[ 0 ].type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 16 ] = { };
			if ( !instructions::sse::read_xmm_or_mem ( cpu, mem, instr, instr.operands[ 1 ], src_data ) )
			{
				return false;
			}

			std::uint8_t dst_data[ 16 ];
			std::memcpy ( dst_data, cpu.read_xmm ( instr.operands[ 0 ].xmm ), 16 );

			std::uint16_t dst_words[ 8 ], src_words[ 8 ], result_words[ 8 ];
			std::memcpy ( dst_words, dst_data, 16 );
			std::memcpy ( src_words, src_data, 16 );

			for ( int i = 0; i < 4; i++ )
			{
				result_words[ i * 2 ] = dst_words[ i ];
				result_words[ i * 2 + 1 ] = src_words[ i ];
			}

			std::uint8_t result[ 16 ];
			std::memcpy ( result, result_words, 16 );

			cpu.write_xmm ( instr.operands[ 0 ].xmm, result );
			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto punpckldq ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 || instr.operands[ 0 ].type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 16 ] = { };
			if ( !instructions::sse::read_xmm_or_mem ( cpu, mem, instr, instr.operands[ 1 ], src_data ) )
			{
				return false;
			}

			std::uint8_t dst_data[ 16 ];
			std::memcpy ( dst_data, cpu.read_xmm ( instr.operands[ 0 ].xmm ), 16 );

			std::uint32_t dst_dwords[ 4 ], src_dwords[ 4 ], result_dwords[ 4 ];
			std::memcpy ( dst_dwords, dst_data, 16 );
			std::memcpy ( src_dwords, src_data, 16 );

			for ( int i = 0; i < 2; i++ )
			{
				result_dwords[ i * 2 ] = dst_dwords[ i ];
				result_dwords[ i * 2 + 1 ] = src_dwords[ i ];
			}

			std::uint8_t result[ 16 ];
			std::memcpy ( result, result_dwords, 16 );

			cpu.write_xmm ( instr.operands[ 0 ].xmm, result );
			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto punpcklqdq ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 || instr.operands[ 0 ].type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 16 ] = { };
			if ( !instructions::sse::read_xmm_or_mem ( cpu, mem, instr, instr.operands[ 1 ], src_data ) )
			{
				return false;
			}

			std::uint8_t dst_data[ 16 ];
			std::memcpy ( dst_data, cpu.read_xmm ( instr.operands[ 0 ].xmm ), 16 );

			std::uint8_t result[ 16 ];
			std::memcpy ( result, dst_data, 8 );
			std::memcpy ( result + 8, src_data, 8 );

			cpu.write_xmm ( instr.operands[ 0 ].xmm, result );
			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto punpckhbw ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 || instr.operands[ 0 ].type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 16 ] = { };
			if ( !instructions::sse::read_xmm_or_mem ( cpu, mem, instr, instr.operands[ 1 ], src_data ) )
			{
				return false;
			}

			std::uint8_t dst_data[ 16 ];
			std::memcpy ( dst_data, cpu.read_xmm ( instr.operands[ 0 ].xmm ), 16 );

			std::uint8_t result[ 16 ];
			for ( int i = 0; i < 8; i++ )
			{
				result[ i * 2 ] = dst_data[ i + 8 ];
				result[ i * 2 + 1 ] = src_data[ i + 8 ];
			}

			cpu.write_xmm ( instr.operands[ 0 ].xmm, result );
			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto punpckhwd ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 || instr.operands[ 0 ].type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 16 ] = { };
			if ( !instructions::sse::read_xmm_or_mem ( cpu, mem, instr, instr.operands[ 1 ], src_data ) )
			{
				return false;
			}

			std::uint8_t dst_data[ 16 ];
			std::memcpy ( dst_data, cpu.read_xmm ( instr.operands[ 0 ].xmm ), 16 );

			std::uint16_t dst_words[ 8 ], src_words[ 8 ], result_words[ 8 ];
			std::memcpy ( dst_words, dst_data, 16 );
			std::memcpy ( src_words, src_data, 16 );

			for ( int i = 0; i < 4; i++ )
			{
				result_words[ i * 2 ] = dst_words[ i + 4 ];
				result_words[ i * 2 + 1 ] = src_words[ i + 4 ];
			}

			std::uint8_t result[ 16 ];
			std::memcpy ( result, result_words, 16 );

			cpu.write_xmm ( instr.operands[ 0 ].xmm, result );
			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto punpckhdq ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 || instr.operands[ 0 ].type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 16 ] = { };
			if ( !instructions::sse::read_xmm_or_mem ( cpu, mem, instr, instr.operands[ 1 ], src_data ) )
			{
				return false;
			}

			std::uint8_t dst_data[ 16 ];
			std::memcpy ( dst_data, cpu.read_xmm ( instr.operands[ 0 ].xmm ), 16 );

			std::uint32_t dst_dwords[ 4 ], src_dwords[ 4 ], result_dwords[ 4 ];
			std::memcpy ( dst_dwords, dst_data, 16 );
			std::memcpy ( src_dwords, src_data, 16 );

			for ( int i = 0; i < 2; i++ )
			{
				result_dwords[ i * 2 ] = dst_dwords[ i + 2 ];
				result_dwords[ i * 2 + 1 ] = src_dwords[ i + 2 ];
			}

			std::uint8_t result[ 16 ];
			std::memcpy ( result, result_dwords, 16 );

			cpu.write_xmm ( instr.operands[ 0 ].xmm, result );
			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto punpckhqdq ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 || instr.operands[ 0 ].type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 16 ] = { };
			if ( !instructions::sse::read_xmm_or_mem ( cpu, mem, instr, instr.operands[ 1 ], src_data ) )
			{
				return false;
			}

			std::uint8_t dst_data[ 16 ];
			std::memcpy ( dst_data, cpu.read_xmm ( instr.operands[ 0 ].xmm ), 16 );

			std::uint8_t result[ 16 ];
			std::memcpy ( result, dst_data + 8, 8 );
			std::memcpy ( result + 8, src_data + 8, 8 );

			cpu.write_xmm ( instr.operands[ 0 ].xmm, result );
			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto packsswb ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 || instr.operands[ 0 ].type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 16 ] = { };
			if ( !instructions::sse::read_xmm_or_mem ( cpu, mem, instr, instr.operands[ 1 ], src_data ) )
			{
				return false;
			}

			std::uint8_t dst_data[ 16 ];
			std::memcpy ( dst_data, cpu.read_xmm ( instr.operands[ 0 ].xmm ), 16 );

			std::int16_t dst_words[ 8 ], src_words[ 8 ];
			std::memcpy ( dst_words, dst_data, 16 );
			std::memcpy ( src_words, src_data, 16 );

			std::uint8_t result[ 16 ];
			for ( int i = 0; i < 8; i++ )
			{
				const auto v = static_cast< std::int32_t > ( dst_words[ i ] );
				result[ i ] = static_cast< std::uint8_t > ( v < -128 ? -128 : ( v > 127 ? 127 : v ) );
			}
			for ( int i = 0; i < 8; i++ )
			{
				const auto v = static_cast< std::int32_t > ( src_words[ i ] );
				result[ i + 8 ] = static_cast< std::uint8_t > ( v < -128 ? -128 : ( v > 127 ? 127 : v ) );
			}

			cpu.write_xmm ( instr.operands[ 0 ].xmm, result );
			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto packuswb ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 || instr.operands[ 0 ].type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 16 ] = { };
			if ( !instructions::sse::read_xmm_or_mem ( cpu, mem, instr, instr.operands[ 1 ], src_data ) )
			{
				return false;
			}

			std::uint8_t dst_data[ 16 ];
			std::memcpy ( dst_data, cpu.read_xmm ( instr.operands[ 0 ].xmm ), 16 );

			std::int16_t dst_words[ 8 ], src_words[ 8 ];
			std::memcpy ( dst_words, dst_data, 16 );
			std::memcpy ( src_words, src_data, 16 );

			std::uint8_t result[ 16 ];
			for ( int i = 0; i < 8; i++ )
			{
				const auto v = static_cast< std::int32_t > ( dst_words[ i ] );
				result[ i ] = static_cast< std::uint8_t > ( v < 0 ? 0 : ( v > 255 ? 255 : v ) );
			}
			for ( int i = 0; i < 8; i++ )
			{
				const auto v = static_cast< std::int32_t > ( src_words[ i ] );
				result[ i + 8 ] = static_cast< std::uint8_t > ( v < 0 ? 0 : ( v > 255 ? 255 : v ) );
			}

			cpu.write_xmm ( instr.operands[ 0 ].xmm, result );
			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto packssdw ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 || instr.operands[ 0 ].type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 16 ] = { };
			if ( !instructions::sse::read_xmm_or_mem ( cpu, mem, instr, instr.operands[ 1 ], src_data ) )
			{
				return false;
			}

			std::uint8_t dst_data[ 16 ];
			std::memcpy ( dst_data, cpu.read_xmm ( instr.operands[ 0 ].xmm ), 16 );

			std::int32_t dst_dwords[ 4 ], src_dwords[ 4 ];
			std::memcpy ( dst_dwords, dst_data, 16 );
			std::memcpy ( src_dwords, src_data, 16 );

			std::int16_t result_words[ 8 ];
			for ( int i = 0; i < 4; i++ )
			{
				const auto v = dst_dwords[ i ];
				result_words[ i ] = static_cast< std::int16_t > ( v < -32768 ? -32768 : ( v > 32767 ? 32767 : v ) );
			}
			for ( int i = 0; i < 4; i++ )
			{
				const auto v = src_dwords[ i ];
				result_words[ i + 4 ] = static_cast< std::int16_t > ( v < -32768 ? -32768 : ( v > 32767 ? 32767 : v ) );
			}

			std::uint8_t result[ 16 ];
			std::memcpy ( result, result_words, 16 );

			cpu.write_xmm ( instr.operands[ 0 ].xmm, result );
			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto pcmpgtb ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 || instr.operands[ 0 ].type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 16 ] = { };
			if ( !instructions::sse::read_xmm_or_mem ( cpu, mem, instr, instr.operands[ 1 ], src_data ) )
			{
				return false;
			}

			std::uint8_t dst_data[ 16 ];
			std::memcpy ( dst_data, cpu.read_xmm ( instr.operands[ 0 ].xmm ), 16 );

			std::uint8_t result[ 16 ];
			for ( int i = 0; i < 16; i++ )
			{
				result[ i ] = ( static_cast< std::int8_t > ( dst_data[ i ] ) > static_cast< std::int8_t >( src_data[ i ] ) ) ? 0xFF : 0x00;
			}

			cpu.write_xmm ( instr.operands[ 0 ].xmm, result );
			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto pcmpgtw ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 || instr.operands[ 0 ].type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 16 ] = { };
			if ( !instructions::sse::read_xmm_or_mem ( cpu, mem, instr, instr.operands[ 1 ], src_data ) )
			{
				return false;
			}

			std::uint8_t dst_data[ 16 ];
			std::memcpy ( dst_data, cpu.read_xmm ( instr.operands[ 0 ].xmm ), 16 );

			std::int16_t dst_words[ 8 ], src_words[ 8 ], result_words[ 8 ];
			std::memcpy ( dst_words, dst_data, 16 );
			std::memcpy ( src_words, src_data, 16 );

			for ( int i = 0; i < 8; i++ )
			{
				result_words[ i ] = ( dst_words[ i ] > src_words[ i ] ) ? static_cast< std::int16_t >( 0xFFFF ) : 0;
			}

			std::uint8_t result[ 16 ];
			std::memcpy ( result, result_words, 16 );

			cpu.write_xmm ( instr.operands[ 0 ].xmm, result );
			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto pcmpgtd ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 || instr.operands[ 0 ].type != operand_type::xmm )
			{
				return false;
			}

			std::uint8_t src_data[ 16 ] = { };
			if ( !instructions::sse::read_xmm_or_mem ( cpu, mem, instr, instr.operands[ 1 ], src_data ) )
			{
				return false;
			}

			std::uint8_t dst_data[ 16 ];
			std::memcpy ( dst_data, cpu.read_xmm ( instr.operands[ 0 ].xmm ), 16 );

			std::int32_t dst_dwords[ 4 ], src_dwords[ 4 ], result_dwords[ 4 ];
			std::memcpy ( dst_dwords, dst_data, 16 );
			std::memcpy ( src_dwords, src_data, 16 );

			for ( int i = 0; i < 4; i++ )
			{
				result_dwords[ i ] = ( dst_dwords[ i ] > src_dwords[ i ] ) ? -1 : 0;
			}

			std::uint8_t result[ 16 ];
			std::memcpy ( result, result_dwords, 16 );

			cpu.write_xmm ( instr.operands[ 0 ].xmm, result );
			cpu.rip = instr.address + instr.length;
			return true;
		}
	}
}