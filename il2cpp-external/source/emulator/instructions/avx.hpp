#pragma once
#include <Windows.h>
#include <cstdio>
#include <array>
#include "../state.hpp"
#include "../handler.hpp"
#include "../decoder.hpp"

namespace instructions
{
	namespace vzeroupper
	{
		inline auto execute ( cpu_state& cpu, const instruction& instr ) -> bool
		{
			for ( std::uint8_t i = 0; i < 16; i++ )
			{
				cpu.zero_ymm_hi ( i );
			}

			return true;
		}
	}

	namespace vinsertf128
	{
		inline auto execute ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 3 )
			{
				return false;
			}

			const auto& dst = instr.operands[ 0 ];
			const auto& src2 = instr.operands[ 1 ];
			const auto  imm8 = static_cast< std::uint8_t > ( instr.operands[ 2 ].imm );

			std::uint8_t src2_data[ 16 ] = { };

			if ( src2.type == operand_type::xmm )
			{
				const auto* p = cpu.read_xmm ( src2.xmm );
				if ( p )
				{
					std::memcpy ( src2_data, p, 16 );
				}
			}
			else if ( src2.type == operand_type::mem )
			{
				const auto addr = cpu.calculate_memory_address ( src2.mem, instr.address + instr.length );
				mem.read ( addr, src2_data, 16 );
			}
			else
			{
				return false;
			}

			std::uint8_t result_lo[ 16 ] = { };
			std::uint8_t result_hi[ 16 ] = { };

			const auto* src1_lo = cpu.read_xmm ( instr.vex_vvvv );
			const auto* src1_hi = cpu.read_ymm_hi ( instr.vex_vvvv );

			if ( src1_lo )
			{
				std::memcpy ( result_lo, src1_lo, 16 );
			}

			if ( src1_hi )
			{
				std::memcpy ( result_hi, src1_hi, 16 );
			}

			if ( imm8 & 1 )
			{
				std::memcpy ( result_hi, src2_data, 16 );
			}
			else
			{
				std::memcpy ( result_lo, src2_data, 16 );
			}

			cpu.write_xmm ( dst.xmm, result_lo );
			cpu.write_ymm_hi ( dst.xmm, result_hi );

			return true;
		}
	}

	namespace avx256
	{
		inline auto read_src ( const cpu_state& cpu, const memory_handler& mem,
							   const operand& op, const instruction& instr,
							   std::uint8_t* lo, std::uint8_t* hi ) -> bool
		{
			if ( op.type == operand_type::xmm )
			{
				const auto* p = cpu.read_xmm ( op.xmm );
				const auto* q = cpu.read_ymm_hi ( op.xmm );

				if ( !p || !q )
				{
					return false;
				}

				std::memcpy ( lo, p, 16 );
				std::memcpy ( hi, q, 16 );
				return true;
			}

			if ( op.type == operand_type::mem )
			{
				const auto addr = cpu.calculate_memory_address ( op.mem, instr.address + instr.length );
				return mem.read ( addr, lo, 16 ) && mem.read ( addr + 16, hi, 16 );
			}

			return false;
		}

		inline auto write_dst ( cpu_state& cpu, const memory_handler& mem,
								const operand& op, const instruction& instr,
								const std::uint8_t* lo, const std::uint8_t* hi ) -> bool
		{
			if ( op.type == operand_type::xmm )
			{
				cpu.write_xmm ( op.xmm, lo );
				cpu.write_ymm_hi ( op.xmm, hi );
				return true;
			}

			if ( op.type == operand_type::mem )
			{
				const auto addr = cpu.calculate_memory_address ( op.mem, instr.address + instr.length );
				return mem.write ( addr, lo, 16 ) && mem.write ( addr + 16, hi, 16 );
			}

			return false;
		}

		inline auto execute_move ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 )
			{
				return false;
			}

			std::uint8_t lo[ 16 ] = { };
			std::uint8_t hi[ 16 ] = { };

			if ( !instructions::avx256::read_src ( cpu, mem, instr.operands[ 1 ], instr, lo, hi ) )
			{
				return false;
			}

			return instructions::avx256::write_dst ( cpu, mem, instr.operands[ 0 ], instr, lo, hi );
		}

		inline auto vpshufb ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 3 || instr.operands[ 0 ].type != operand_type::xmm )
			{
				return false;
			}

			const auto is_ymm = instr.operands[ 0 ].size == operand_size::yword;

			std::uint8_t src1_lo[ 16 ] = { }, src1_hi[ 16 ] = { };
			std::uint8_t src2_lo[ 16 ] = { }, src2_hi[ 16 ] = { };

			if ( !instructions::avx256::read_src ( cpu, mem, instr.operands[ 1 ], instr, src1_lo, src1_hi ) )
			{
				return false;
			}

			if ( !instructions::avx256::read_src ( cpu, mem, instr.operands[ 2 ], instr, src2_lo, src2_hi ) )
			{
				return false;
			}

			std::uint8_t result_lo[ 16 ] = { }, result_hi[ 16 ] = { };

			for ( std::int32_t index = 0; index < 16; index++ )
			{
				const auto selector = src2_lo[ index ];

				result_lo[ index ] = ( selector & 0x80 ) != 0 ? 0 : src1_lo[ selector & 0x0F ];
			}

			if ( is_ymm )
			{
				for ( std::int32_t index = 0; index < 16; index++ )
				{
					const auto selector = src2_hi[ index ];

					result_hi[ index ] = ( selector & 0x80 ) != 0 ? 0 : src1_hi[ selector & 0x0F ];
				}
			}

			return instructions::avx256::write_dst ( cpu, mem, instr.operands[ 0 ], instr, result_lo, result_hi );
		}

		inline auto vpxor ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 3 || instr.operands[ 0 ].type != operand_type::xmm )
			{
				return false;
			}

			const auto is_ymm = instr.operands[ 0 ].size == operand_size::yword;

			std::uint8_t src1_lo[ 16 ] = { }, src1_hi[ 16 ] = { };
			std::uint8_t src2_lo[ 16 ] = { }, src2_hi[ 16 ] = { };

			const auto* p = cpu.read_xmm ( instr.operands[ 1 ].xmm );
			if ( !p ) 
			{
				return false;
			}
			std::memcpy ( src1_lo, p, 16 );

			if ( is_ymm )
			{
				const auto* q = cpu.read_ymm_hi ( instr.operands[ 1 ].xmm );
				if ( q ) 
				{
					std::memcpy ( src1_hi, q, 16 );
				}
			}

			if ( instr.operands[ 2 ].type == operand_type::xmm )
			{
				const auto* r = cpu.read_xmm ( instr.operands[ 2 ].xmm );
				if ( !r )
				{
					return false;
				}
				std::memcpy ( src2_lo, r, 16 );

				if ( is_ymm )
				{
					const auto* s = cpu.read_ymm_hi ( instr.operands[ 2 ].xmm );
					if ( s ) 
					{
						std::memcpy ( src2_hi, s, 16 );
					}
				}
			}
			else if ( instr.operands[ 2 ].type == operand_type::mem )
			{
				const auto addr = cpu.calculate_memory_address ( instr.operands[ 2 ].mem, instr.address + instr.length );
				if ( !mem.read ( addr, src2_lo, 16 ) ) 
				{
					return false;
				}
				if ( is_ymm && !mem.read ( addr + 16, src2_hi, 16 ) ) 
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			std::uint8_t result_lo[ 16 ], result_hi[ 16 ] = { };
			for ( int i = 0; i < 16; i++ ) 
			{
				result_lo[ i ] = src1_lo[ i ] ^ src2_lo[ i ];
			}

			if ( is_ymm )
			{
				for ( int i = 0; i < 16; i++ ) 
				{
					result_hi[ i ] = src1_hi[ i ] ^ src2_hi[ i ];
				}
			}

			cpu.write_xmm ( instr.operands[ 0 ].xmm, result_lo );
			if ( is_ymm ) 
			{
				cpu.write_ymm_hi ( instr.operands[ 0 ].xmm, result_hi );
			}
			else 
			{
				cpu.zero_ymm_hi ( instr.operands[ 0 ].xmm );
			}

			cpu.rip = instr.address + instr.length;
			return true;
		}

		template< typename T >
		inline auto vpcmpeq_impl ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 3 || instr.operands[ 0 ].type != operand_type::xmm )
			{
				return false;
			}

			const auto is_ymm = instr.operands[ 0 ].size == operand_size::yword;
			const auto elem_count = is_ymm ? ( 32 / sizeof ( T ) ) : ( 16 / sizeof ( T ) );

			std::uint8_t src1_lo[ 16 ] = { }, src1_hi[ 16 ] = { };
			std::uint8_t src2_lo[ 16 ] = { }, src2_hi[ 16 ] = { };

			const auto* p = cpu.read_xmm ( instr.operands[ 1 ].xmm );
			if ( !p ) 
			{
				return false;
			}
			std::memcpy ( src1_lo, p, 16 );

			if ( is_ymm )
			{
				const auto* q = cpu.read_ymm_hi ( instr.operands[ 1 ].xmm );
				if ( q ) 
				{
					std::memcpy ( src1_hi, q, 16 );
				}
			}

			if ( instr.operands[ 2 ].type == operand_type::xmm )
			{
				const auto* r = cpu.read_xmm ( instr.operands[ 2 ].xmm );
				if ( !r ) 
				{
					return false;
				}
				std::memcpy ( src2_lo, r, 16 );

				if ( is_ymm )
				{
					const auto* s = cpu.read_ymm_hi ( instr.operands[ 2 ].xmm );
					if ( s ) 
					{
						std::memcpy ( src2_hi, s, 16 );
					}
				}
			}
			else if ( instr.operands[ 2 ].type == operand_type::mem )
			{
				const auto addr = cpu.calculate_memory_address ( instr.operands[ 2 ].mem, instr.address + instr.length );
				if ( !mem.read ( addr, src2_lo, 16 ) ) 
				{
					return false;
				}
				if ( is_ymm && !mem.read ( addr + 16, src2_hi, 16 ) ) 
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			std::uint8_t src1_full[ 32 ] = { }, src2_full[ 32 ] = { }, result[ 32 ] = { };
			std::memcpy ( src1_full, src1_lo, 16 );
			std::memcpy ( src1_full + 16, src1_hi, 16 );
			std::memcpy ( src2_full, src2_lo, 16 );
			std::memcpy ( src2_full + 16, src2_hi, 16 );

			const std::size_t bytes = is_ymm ? 32 : 16;
			for ( std::size_t i = 0; i < bytes / sizeof ( T ); i++ )
			{
				T a, b;
				std::memcpy ( &a, src1_full + i * sizeof ( T ), sizeof ( T ) );
				std::memcpy ( &b, src2_full + i * sizeof ( T ), sizeof ( T ) );
				const T mask = ( a == b ) ? static_cast< T > ( -1 ) : static_cast< T > ( 0 );
				std::memcpy ( result + i * sizeof ( T ), &mask, sizeof ( T ) );
			}

			cpu.write_xmm ( instr.operands[ 0 ].xmm, result );
			if ( is_ymm ) 
			{
				cpu.write_ymm_hi ( instr.operands[ 0 ].xmm, result + 16 );
			}
			else
			{
				cpu.zero_ymm_hi ( instr.operands[ 0 ].xmm );
			}

			cpu.rip = instr.address + instr.length;
			return true;
		}

		inline auto vpcmpeqb ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			return instructions::avx256::vpcmpeq_impl< std::uint8_t > ( cpu, mem, instr );
		}

		inline auto vpcmpeqw ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			return instructions::avx256::vpcmpeq_impl< std::uint16_t > ( cpu, mem, instr );
		}

		inline auto vpcmpeqd ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			return instructions::avx256::vpcmpeq_impl< std::uint32_t > ( cpu, mem, instr );
		}

		inline auto vpmovmskb ( cpu_state& cpu, const memory_handler& mem, const instruction& instr ) -> bool
		{
			if ( instr.operand_count < 2 || instr.operands[ 0 ].type != operand_type::reg
				 || instr.operands[ 1 ].type != operand_type::xmm )
			{
				return false;
			}

			const auto is_ymm = instr.operands[ 1 ].size == operand_size::yword;

			std::uint8_t lo[ 16 ] = { }, hi[ 16 ] = { };

			const auto* p = cpu.read_xmm ( instr.operands[ 1 ].xmm );
			if ( !p ) 
			{
				return false;
			}
			std::memcpy ( lo, p, 16 );

			if ( is_ymm )
			{
				const auto* q = cpu.read_ymm_hi ( instr.operands[ 1 ].xmm );
				if ( q ) 
				{
					std::memcpy ( hi, q, 16 );
				}
			}

			std::uint32_t mask = 0;
			for ( int i = 0; i < 16; i++ )
			{
				if ( lo[ i ] & 0x80 ) 
				{
					mask |= ( 1u << i );
				}
			}


			if ( is_ymm )
			{
				for ( int i = 0; i < 16; i++ )
				{
					if ( hi[ i ] & 0x80 ) 
					{
						mask |= ( 1u << ( i + 16 ) );
					}
				}
			}

			cpu.write_gpr ( instr.operands[ 0 ].reg, static_cast< std::uint64_t > ( mask ) );
			cpu.rip = instr.address + instr.length;
			return true;
		}
	}
}