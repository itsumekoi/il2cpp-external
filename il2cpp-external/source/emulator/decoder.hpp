#pragma once
#include <Windows.h>
#include <cstdio>
#include <array>
#include "types.hpp"

class decoder
{
public:
	decoder ( );
	~decoder ( ) = default;

	auto decode ( const std::uint8_t* bytes, std::size_t max_length, instruction& instr ) -> bool;

private:

	auto parse_prefixes ( const std::uint8_t* bytes, std::size_t& offset, instruction& instr ) -> void;

	auto parse_rex ( std::uint8_t byte, instruction& instr ) -> bool;

	auto parse_modrm ( const std::uint8_t* bytes, std::size_t& offset, std::size_t max_length,
					   instruction& instr, operand& op ) -> bool;

	auto parse_sib ( const std::uint8_t* bytes, std::size_t& offset, instruction& instr,
					 memory_operand& mem, std::uint8_t mod ) -> bool;

	auto parse_displacement ( const std::uint8_t* bytes, std::size_t& offset,
							  std::uint8_t mod, memory_operand& mem ) -> void;

	auto parse_immediate ( const std::uint8_t* bytes, std::size_t& offset,
						   operand_size size, std::uint64_t& imm ) -> void;

	auto get_register ( std::uint8_t modrm_field, bool rex_bit ) const -> std::uint8_t;

	auto decode_mov ( const std::uint8_t* bytes, std::size_t& offset,
					  std::size_t max_length, instruction& instr ) -> bool;

	auto decode_lea ( const std::uint8_t* bytes, std::size_t& offset,
					  std::size_t max_length, instruction& instr ) -> bool;

	auto decode_push_pop ( const std::uint8_t* bytes, std::size_t& offset,
						   instruction& instr ) -> bool;

	auto decode_ret ( const std::uint8_t* bytes, std::size_t& offset,
					  instruction& instr ) -> bool;

	auto decode_arithmetic ( const std::uint8_t* bytes, std::size_t& offset,
							 std::size_t max_length, instruction& instr ) -> bool;

	auto decode_jcc ( const std::uint8_t* bytes, std::size_t& offset,
					  instruction& instr ) -> bool;

	auto decode_call ( const std::uint8_t* bytes, std::size_t& offset,
					   std::size_t max_length, instruction& instr ) -> bool;

	auto decode_jmp ( const std::uint8_t* bytes, std::size_t& offset,
					  std::size_t max_length, instruction& instr ) -> bool;

	auto decode_two_byte ( const std::uint8_t* bytes, std::size_t& offset,
						   std::size_t max_length, instruction& instr ) -> bool;

	auto decode_vex ( const std::uint8_t* bytes, std::size_t& offset,
					  std::size_t max_length, instruction& instr ) -> bool;
};

inline decoder::decoder ( )
{ }

inline auto decoder::parse_rex ( std::uint8_t byte, instruction& instr ) -> bool
{
	if ( ( byte & 0xF0 ) != 0x40 )
	{
		return false;
	}

	instr.has_rex = true;
	instr.rex = byte;
	instr.rex_w = ( byte & 0x08 ) != 0;
	instr.rex_r = ( byte & 0x04 ) != 0;
	instr.rex_x = ( byte & 0x02 ) != 0;
	instr.rex_b = ( byte & 0x01 ) != 0;

	return true;
}

inline auto decoder::parse_prefixes ( const std::uint8_t* bytes, std::size_t& offset, instruction& instr ) -> void
{
	while ( offset < 15 )
	{
		const auto byte = bytes[ offset ];

		if ( byte == 0xF0 )
		{
			instr.has_lock = true;
			offset++;
			continue;
		}

		if ( byte == 0x66 )
		{
			instr.has_operand_size = true;
			offset++;
			continue;
		}

		if ( byte == 0x67 )
		{
			instr.has_address_size = true;
			offset++;
			continue;
		}

		if ( byte == 0xF2 )
		{
			instr.has_repne = true;
			offset++;
			continue;
		}

		if ( byte == 0xF3 )
		{
			instr.has_rep = true;
			offset++;
			continue;
		}

		if ( byte == 0x26 || byte == 0x2E || byte == 0x36 || byte == 0x3E )
		{
			offset++;
			continue;
		}

		if ( byte == 0x64 || byte == 0x65 )
		{
			instr.has_segment_prefix = true;
			instr.segment_prefix = byte;
			offset++;
			continue;
		}

		if ( this->parse_rex ( byte, instr ) )
		{
			offset++;
			continue;
		}

		if ( byte == 0xC5 && !instr.has_rex )
		{
			offset++;
			if ( offset >= 15 )
			{
				break;
			}

			const auto vex1 = bytes[ offset++ ];
			instr.has_vex = true;
			instr.vex_r = !( ( vex1 >> 7 ) & 1 );
			instr.vex_x = false;
			instr.vex_b = false;
			instr.vex_w = false;
			instr.vex_vvvv = ( ~( vex1 >> 3 ) ) & 0xF;
			instr.vex_l = ( vex1 >> 2 ) & 1;
			instr.vex_pp = vex1 & 0x3;
			instr.vex_map = 1;
			instr.rex_r = instr.vex_r;
			instr.rex_x = instr.vex_x;
			instr.rex_b = instr.vex_b;
			instr.rex_w = instr.vex_w;
			break;
		}

		if ( byte == 0xC4 && !instr.has_rex )
		{
			offset++;
			if ( offset + 1 >= 15 )
			{
				break;
			}

			const auto vex1 = bytes[ offset++ ];
			const auto vex2 = bytes[ offset++ ];
			instr.has_vex = true;
			instr.vex_r = !( ( vex1 >> 7 ) & 1 );
			instr.vex_x = !( ( vex1 >> 6 ) & 1 );
			instr.vex_b = !( ( vex1 >> 5 ) & 1 );
			instr.vex_map = vex1 & 0x1F;
			instr.vex_w = ( vex2 >> 7 ) & 1;
			instr.vex_vvvv = ( ~( vex2 >> 3 ) ) & 0xF;
			instr.vex_l = ( vex2 >> 2 ) & 1;
			instr.vex_pp = vex2 & 0x3;
			instr.rex_r = instr.vex_r;
			instr.rex_x = instr.vex_x;
			instr.rex_b = instr.vex_b;
			instr.rex_w = instr.vex_w;
			break;
		}

		break;
	}
}

inline auto decoder::get_register ( std::uint8_t modrm_field, bool rex_bit ) const -> std::uint8_t
{
	return modrm_field | ( rex_bit ? 0x08 : 0x00 );
}

inline auto decoder::parse_displacement ( const std::uint8_t* bytes, std::size_t& offset,
										  std::uint8_t mod, memory_operand& mem ) -> void
{
	if ( mod == 0x01 )
	{
		mem.displacement = static_cast< std::int8_t >( bytes[ offset ] );
		offset++;
	}
	else if ( mod == 0x02 )
	{
		std::memcpy ( &mem.displacement, &bytes[ offset ], 4 );
		offset += 4;
	}
}

inline auto decoder::parse_sib ( const std::uint8_t* bytes, std::size_t& offset, instruction& instr,
								 memory_operand& mem, std::uint8_t mod ) -> bool
{
	const auto sib = bytes[ offset++ ];

	const auto scale_bits = ( sib >> 6 ) & 0x03;
	const auto index_bits = ( sib >> 3 ) & 0x07;
	const auto base_bits = ( sib >> 0 ) & 0x07;

	mem.scale = 1 << scale_bits;

	if ( index_bits == 0x04 )
	{
		mem.index = 0xFF;
	}
	else
	{
		mem.index = this->get_register ( index_bits, instr.rex_x );
	}

	if ( mod == 0x00 && base_bits == 0x05 )
	{
		mem.base = 0xFF;
	}
	else
	{
		mem.base = this->get_register ( base_bits, instr.rex_b );
	}

	return true;
}

inline auto decoder::parse_modrm ( const std::uint8_t* bytes, std::size_t& offset, std::size_t max_length,
								   instruction& instr, operand& op ) -> bool
{
	if ( offset >= max_length )
	{
		return false;
	}

	const auto modrm = bytes[ offset++ ];
	instr.modrm_byte = modrm;
	instr.has_modrm = true;

	const auto mod = ( modrm >> 6 ) & 0x03;
	const auto reg = ( modrm >> 3 ) & 0x07;
	const auto rm = ( modrm >> 0 ) & 0x07;

	if ( mod == 0x03 )
	{
		op.type = operand_type::reg;
		op.reg = this->get_register ( rm, instr.rex_b );
	}

	else
	{
		op.type = operand_type::mem;
		op.mem = memory_operand ( );

		if ( mod == 0x00 && rm == 0x05 )
		{
			op.mem.base = 16;
			op.mem.index = 0xFF;
			op.mem.scale = 1;

			std::int32_t disp32 = 0;
			std::memcpy ( &disp32, &bytes[ offset ], 4 );
			offset += 4;
			op.mem.displacement = disp32;
		}

		else if ( rm == 0x04 )
		{
			if ( offset >= max_length )
			{
				return false;
			}

			this->parse_sib ( bytes, offset, instr, op.mem, mod );

			if ( mod == 0x00 && op.mem.base == 0xFF )
			{
				std::int32_t disp32 = 0;
				std::memcpy ( &disp32, &bytes[ offset ], 4 );
				offset += 4;
				op.mem.displacement = disp32;
			}
			else
			{
				this->parse_displacement ( bytes, offset, mod, op.mem );
			}
		}

		else
		{
			op.mem.base = this->get_register ( rm, instr.rex_b );
			this->parse_displacement ( bytes, offset, mod, op.mem );
		}
	}

	return true;
}

inline auto decoder::parse_immediate ( const std::uint8_t* bytes, std::size_t& offset,
									   operand_size size, std::uint64_t& imm ) -> void
{
	switch ( size )
	{
		case operand_size::byte:
		{
			imm = bytes[ offset ];
			offset += 1;
			break;
		}

		case operand_size::word:
		{
			std::memcpy ( &imm, &bytes[ offset ], 2 );
			offset += 2;
			break;
		}

		case operand_size::dword:
		{
			std::memcpy ( &imm, &bytes[ offset ], 4 );
			offset += 4;
			break;
		}

		case operand_size::qword:
		{
			std::memcpy ( &imm, &bytes[ offset ], 8 );
			offset += 8;
			break;
		}

		default:
		{
			break;
		}
	}
}

inline auto decoder::decode_mov ( const std::uint8_t* bytes, std::size_t& offset,
								  std::size_t max_length, instruction& instr ) -> bool
{
	instr.type = instruction_type::mov;

	switch ( instr.opcode )
	{
		case 0x88:
		case 0x89:
		{
			if ( !this->parse_modrm ( bytes, offset, max_length, instr, instr.operands[ 0 ] ) )
			{
				return false;
			}

			const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;
			instr.operands[ 1 ].type = operand_type::reg;
			instr.operands[ 1 ].reg = this->get_register ( reg, instr.rex_r );

			if ( instr.opcode == 0x88 )
			{
				instr.operands[ 0 ].size = operand_size::byte;
				instr.operands[ 1 ].size = operand_size::byte;
			}
			else
			{
				instr.operands[ 0 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;
				instr.operands[ 1 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;
			}

			instr.operand_count = 2;
			break;
		}

		case 0x8A:
		case 0x8B:
		{
			operand rm_operand;
			if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
			{
				return false;
			}

			const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;
			instr.operands[ 0 ].type = operand_type::reg;
			instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );

			instr.operands[ 1 ] = rm_operand;

			if ( instr.opcode == 0x8A )
			{
				instr.operands[ 0 ].size = operand_size::byte;
				instr.operands[ 1 ].size = operand_size::byte;
			}
			else
			{
				instr.operands[ 0 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;
				instr.operands[ 1 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;
			}

			instr.operand_count = 2;
			break;
		}

		case 0xB0: case 0xB1: case 0xB2: case 0xB3:
		case 0xB4: case 0xB5: case 0xB6: case 0xB7:
		{
			instr.operands[ 0 ].type = operand_type::reg;
			instr.operands[ 0 ].reg = this->get_register ( instr.opcode & 0x07, instr.rex_b );
			instr.operands[ 0 ].size = operand_size::byte;

			instr.operands[ 1 ].type = operand_type::imm;
			instr.operands[ 1 ].imm = bytes[ offset++ ];
			instr.operands[ 1 ].size = operand_size::byte;

			instr.operand_count = 2;
			break;
		}

		case 0xB8: case 0xB9: case 0xBA: case 0xBB:
		case 0xBC: case 0xBD: case 0xBE: case 0xBF:
		{
			instr.operands[ 0 ].type = operand_type::reg;
			instr.operands[ 0 ].reg = this->get_register ( instr.opcode & 0x07, instr.rex_b );

			instr.operands[ 1 ].type = operand_type::imm;

			this->parse_immediate ( bytes, offset, instr.rex_w ? operand_size::qword : operand_size::dword,
							  instr.operands[ 1 ].imm );

			instr.operand_count = 2;
			break;
		}

		case 0xC6:
		case 0xC7:
		{
			if ( !this->parse_modrm ( bytes, offset, max_length, instr, instr.operands[ 0 ] ) )
			{
				return false;
			}

			instr.operands[ 1 ].type = operand_type::imm;

			if ( instr.opcode == 0xC6 )
			{
				instr.operands[ 1 ].imm = bytes[ offset++ ];
				instr.operands[ 0 ].size = operand_size::byte;
				instr.operands[ 1 ].size = operand_size::byte;
			}
			else
			{
				std::int32_t imm32 = 0;
				std::memcpy ( &imm32, &bytes[ offset ], 4 );
				offset += 4;

				if ( instr.rex_w )
				{
					instr.operands[ 1 ].imm = static_cast< std::uint64_t > ( static_cast< std::int64_t > ( imm32 ) );
					instr.operands[ 0 ].size = operand_size::qword;
				}
				else
				{
					instr.operands[ 1 ].imm = static_cast< std::uint32_t > ( imm32 );
					instr.operands[ 0 ].size = operand_size::dword;
				}

				instr.operands[ 1 ].size = operand_size::dword;
			}

			instr.operand_count = 2;
			instr.length = static_cast< std::uint8_t >( offset );
			return true;
		}

		default:
		{
			return false;
		}
	}

	instr.length = static_cast< std::uint8_t >( offset );
	return true;
}

inline auto decoder::decode_lea ( const std::uint8_t* bytes, std::size_t& offset,
								  std::size_t max_length, instruction& instr ) -> bool
{

	instr.type = instruction_type::lea;

	operand mem_operand;
	if ( !this->parse_modrm ( bytes, offset, max_length, instr, mem_operand ) )
	{
		return false;
	}

	const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;
	instr.operands[ 0 ].type = operand_type::reg;
	instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );

	instr.operands[ 1 ] = mem_operand;

	instr.operand_count = 2;
	instr.length = static_cast< std::uint8_t >( offset );

	return true;
}

inline auto decoder::decode_push_pop ( const std::uint8_t* bytes, std::size_t& offset,
									   instruction& instr ) -> bool
{

	if ( instr.opcode >= 0x50 && instr.opcode <= 0x57 )
	{
		instr.type = instruction_type::push;
		instr.operands[ 0 ].type = operand_type::reg;
		instr.operands[ 0 ].reg = this->get_register ( instr.opcode & 0x07, instr.rex_b );
		instr.operand_count = 1;
	}

	else if ( instr.opcode >= 0x58 && instr.opcode <= 0x5F )
	{
		instr.type = instruction_type::pop;
		instr.operands[ 0 ].type = operand_type::reg;
		instr.operands[ 0 ].reg = this->get_register ( instr.opcode & 0x07, instr.rex_b );
		instr.operand_count = 1;
	}
	else
	{
		return false;
	}

	instr.length = static_cast< std::uint8_t >( offset );
	return true;
}

inline auto decoder::decode_arithmetic ( const std::uint8_t* bytes, std::size_t& offset,
										 std::size_t max_length, instruction& instr ) -> bool
{

	if ( instr.opcode == 0x08 || instr.opcode == 0x0A )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		instr.type = instruction_type::or_instr;

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		if ( instr.opcode == 0x08 )
		{
			instr.operands[ 0 ] = rm_operand;
			instr.operands[ 1 ].type = operand_type::reg;
			instr.operands[ 1 ].reg = this->get_register ( reg, instr.rex_r );
		}
		else
		{
			instr.operands[ 0 ].type = operand_type::reg;
			instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
			instr.operands[ 1 ] = rm_operand;
		}

		instr.operands[ 0 ].size = operand_size::byte;
		instr.operands[ 1 ].size = operand_size::byte;

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( instr.opcode == 0x3C )
	{
		instr.type = instruction_type::cmp;

		instr.operands[ 0 ].type = operand_type::reg;
		instr.operands[ 0 ].reg = 0;
		instr.operands[ 0 ].size = operand_size::byte;

		instr.operands[ 1 ].type = operand_type::imm;
		instr.operands[ 1 ].imm = bytes[ offset++ ];
		instr.operands[ 1 ].size = operand_size::byte;

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( instr.opcode == 0x38 || instr.opcode == 0x3A )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		instr.type = instruction_type::cmp;

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		if ( instr.opcode == 0x38 )
		{
			instr.operands[ 0 ] = rm_operand;
			instr.operands[ 0 ].size = operand_size::byte;
			instr.operands[ 1 ].type = operand_type::reg;
			instr.operands[ 1 ].reg = this->get_register ( reg, instr.rex_r );
			instr.operands[ 1 ].size = operand_size::byte;
		}
		else
		{
			instr.operands[ 0 ].type = operand_type::reg;
			instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
			instr.operands[ 0 ].size = operand_size::byte;
			instr.operands[ 1 ] = rm_operand;
			instr.operands[ 1 ].size = operand_size::byte;
		}

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t > ( offset );
		return true;
	}

	if ( instr.opcode == 0x18 || instr.opcode == 0x19 ||
		 instr.opcode == 0x1A || instr.opcode == 0x1B )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		instr.type = instruction_type::sbb;

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		if ( instr.opcode == 0x18 || instr.opcode == 0x19 )
		{
			instr.operands[ 0 ] = rm_operand;
			instr.operands[ 1 ].type = operand_type::reg;
			instr.operands[ 1 ].reg = this->get_register ( reg, instr.rex_r );
		}
		else
		{
			instr.operands[ 0 ].type = operand_type::reg;
			instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
			instr.operands[ 1 ] = rm_operand;
		}

		if ( instr.opcode == 0x18 || instr.opcode == 0x1A )
		{
			instr.operands[ 0 ].size = operand_size::byte;
			instr.operands[ 1 ].size = operand_size::byte;
		}
		else
		{
			instr.operands[ 0 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;
			instr.operands[ 1 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;
		}

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( instr.opcode == 0x39 || instr.opcode == 0x3B )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		instr.type = instruction_type::cmp;

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		if ( instr.opcode == 0x39 )
		{
			instr.operands[ 0 ] = rm_operand;
			instr.operands[ 1 ].type = operand_type::reg;
			instr.operands[ 1 ].reg = this->get_register ( reg, instr.rex_r );
		}
		else
		{
			instr.operands[ 0 ].type = operand_type::reg;
			instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
			instr.operands[ 1 ] = rm_operand;
		}

		instr.operands[ 0 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;
		instr.operands[ 1 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( instr.opcode == 0xFE || instr.opcode == 0xFF )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		const auto reg_field = ( instr.modrm_byte >> 3 ) & 0x07;

		if ( reg_field == 0 )
		{
			instr.type = instruction_type::inc;
		}
		else if ( reg_field == 1 )
		{
			instr.type = instruction_type::dec;
		}
		else
		{
			return false;
		}

		instr.operands[ 0 ] = rm_operand;
		instr.operands[ 0 ].size = ( instr.opcode == 0xFE ) ? operand_size::byte :
			( instr.rex_w ? operand_size::qword : operand_size::dword );
		instr.operand_count = 1;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( instr.opcode == 0x09 || instr.opcode == 0x0B )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		instr.type = instruction_type::or_instr;

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		if ( instr.opcode == 0x09 )
		{
			instr.operands[ 0 ] = rm_operand;
			instr.operands[ 1 ].type = operand_type::reg;
			instr.operands[ 1 ].reg = this->get_register ( reg, instr.rex_r );
		}
		else
		{
			instr.operands[ 0 ].type = operand_type::reg;
			instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
			instr.operands[ 1 ] = rm_operand;
		}

		instr.operands[ 0 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;
		instr.operands[ 1 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( instr.opcode == 0x30 || instr.opcode == 0x32 )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		instr.type = instruction_type::xor_instr;

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		if ( instr.opcode == 0x30 )
		{
			instr.operands[ 0 ] = rm_operand;
			instr.operands[ 1 ].type = operand_type::reg;
			instr.operands[ 1 ].reg = this->get_register ( reg, instr.rex_r );
		}
		else
		{
			instr.operands[ 0 ].type = operand_type::reg;
			instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
			instr.operands[ 1 ] = rm_operand;
		}

		instr.operands[ 0 ].size = operand_size::byte;
		instr.operands[ 1 ].size = operand_size::byte;

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t > ( offset );
		return true;
	}

	if ( instr.opcode == 0x31 || instr.opcode == 0x33 )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		instr.type = instruction_type::xor_instr;

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		if ( instr.opcode == 0x31 )
		{
			instr.operands[ 0 ] = rm_operand;
			instr.operands[ 1 ].type = operand_type::reg;
			instr.operands[ 1 ].reg = this->get_register ( reg, instr.rex_r );
		}
		else
		{
			instr.operands[ 0 ].type = operand_type::reg;
			instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
			instr.operands[ 1 ] = rm_operand;
		}

		instr.operands[ 0 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;
		instr.operands[ 1 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( instr.opcode == 0x20 || instr.opcode == 0x22 )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		instr.type = instruction_type::and_instr;

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		if ( instr.opcode == 0x20 )
		{
			instr.operands[ 0 ] = rm_operand;
			instr.operands[ 1 ].type = operand_type::reg;
			instr.operands[ 1 ].reg = this->get_register ( reg, instr.rex_r );
		}
		else
		{
			instr.operands[ 0 ].type = operand_type::reg;
			instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
			instr.operands[ 1 ] = rm_operand;
		}

		instr.operands[ 0 ].size = operand_size::byte;
		instr.operands[ 1 ].size = operand_size::byte;

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( instr.opcode == 0x21 || instr.opcode == 0x23 )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		instr.type = instruction_type::and_instr;

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		if ( instr.opcode == 0x21 )
		{
			instr.operands[ 0 ] = rm_operand;
			instr.operands[ 1 ].type = operand_type::reg;
			instr.operands[ 1 ].reg = this->get_register ( reg, instr.rex_r );
		}
		else
		{
			instr.operands[ 0 ].type = operand_type::reg;
			instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
			instr.operands[ 1 ] = rm_operand;
		}

		instr.operands[ 0 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;
		instr.operands[ 1 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( instr.opcode == 0x28 || instr.opcode == 0x29 ||
		 instr.opcode == 0x2A || instr.opcode == 0x2B )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		instr.type = instruction_type::sub;

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		if ( instr.opcode == 0x2A || instr.opcode == 0x2B )
		{
			instr.operands[ 0 ].type = operand_type::reg;
			instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
			instr.operands[ 1 ] = rm_operand;
		}
		else
		{
			instr.operands[ 0 ] = rm_operand;
			instr.operands[ 1 ].type = operand_type::reg;
			instr.operands[ 1 ].reg = this->get_register ( reg, instr.rex_r );
		}

		if ( instr.opcode == 0x28 || instr.opcode == 0x2A )
		{
			instr.operands[ 0 ].size = operand_size::byte;
			instr.operands[ 1 ].size = operand_size::byte;
		}
		else
		{
			instr.operands[ 0 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;
			instr.operands[ 1 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;
		}

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( instr.opcode == 0x84 || instr.opcode == 0x85 )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		instr.type = instruction_type::test;

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		instr.operands[ 0 ] = rm_operand;
		instr.operands[ 1 ].type = operand_type::reg;
		instr.operands[ 1 ].reg = this->get_register ( reg, instr.rex_r );

		if ( instr.opcode == 0x84 )
		{
			instr.operands[ 0 ].size = operand_size::byte;
			instr.operands[ 1 ].size = operand_size::byte;
		}
		else
		{
			instr.operands[ 0 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;
			instr.operands[ 1 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;
		}

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( instr.opcode == 0xF6 || instr.opcode == 0xF7 )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		const auto reg_field = ( instr.modrm_byte >> 3 ) & 0x07;

		if ( reg_field == 0 )
		{
			instr.type = instruction_type::test;
			instr.operands[ 0 ] = rm_operand;

			instr.operands[ 1 ].type = operand_type::imm;
			if ( instr.opcode == 0xF6 )
			{
				instr.operands[ 1 ].imm = bytes[ offset++ ];
				instr.operands[ 0 ].size = operand_size::byte;
				instr.operands[ 1 ].size = operand_size::byte;
			}
			else
			{
				std::int32_t imm32 = 0;
				std::memcpy ( &imm32, &bytes[ offset ], 4 );
				offset += 4;
				instr.operands[ 1 ].imm = imm32;
				instr.operands[ 0 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;
				instr.operands[ 1 ].size = operand_size::dword;
			}

			instr.operand_count = 2;
			instr.length = static_cast< std::uint8_t >( offset );
			return true;
		}

		else if ( reg_field == 2 )
		{
			instr.type = instruction_type::not_instr;
			instr.operands[ 0 ] = rm_operand;
			instr.operands[ 0 ].size = ( instr.opcode == 0xF6 ) ? operand_size::byte :
				( instr.rex_w ? operand_size::qword : operand_size::dword );
			instr.operand_count = 1;
			instr.length = static_cast< std::uint8_t >( offset );
			return true;
		}

		else if ( reg_field == 3 )
		{
			instr.type = instruction_type::neg;
			instr.operands[ 0 ] = rm_operand;
			instr.operands[ 0 ].size = ( instr.opcode == 0xF6 ) ? operand_size::byte :
				( instr.rex_w ? operand_size::qword : operand_size::dword );
			instr.operand_count = 1;
			instr.length = static_cast< std::uint8_t >( offset );
			return true;
		}

		else if ( reg_field == 4 )
		{
			instr.type = instruction_type::mul;
			instr.operands[ 0 ] = rm_operand;
			instr.operands[ 0 ].size = ( instr.opcode == 0xF6 ) ? operand_size::byte :
				( instr.rex_w ? operand_size::qword : operand_size::dword );
			instr.operand_count = 1;
			instr.length = static_cast< std::uint8_t >( offset );
			return true;
		}

		else if ( reg_field == 5 )
		{
			instr.type = instruction_type::imul;
			instr.operands[ 0 ] = rm_operand;
			instr.operands[ 0 ].size = ( instr.opcode == 0xF6 ) ? operand_size::byte :
				( instr.rex_w ? operand_size::qword : operand_size::dword );
			instr.operand_count = 1;
			instr.length = static_cast< std::uint8_t >( offset );
			return true;
		}

		else if ( reg_field == 6 )
		{
			instr.type = instruction_type::div;
			instr.operands[ 0 ] = rm_operand;
			instr.operands[ 0 ].size = ( instr.opcode == 0xF6 ) ? operand_size::byte :
				( instr.rex_w ? operand_size::qword : operand_size::dword );
			instr.operand_count = 1;
			instr.length = static_cast< std::uint8_t >( offset );
			return true;
		}

		else if ( reg_field == 7 )
		{
			instr.type = instruction_type::idiv;
			instr.operands[ 0 ] = rm_operand;
			instr.operands[ 0 ].size = ( instr.opcode == 0xF6 ) ? operand_size::byte :
				( instr.rex_w ? operand_size::qword : operand_size::dword );
			instr.operand_count = 1;
			instr.length = static_cast< std::uint8_t >( offset );
			return true;
		}

		return false;
	}

	if ( instr.opcode == 0xC0 || instr.opcode == 0xC1 ||
		 instr.opcode == 0xD0 || instr.opcode == 0xD1 ||
		 instr.opcode == 0xD2 || instr.opcode == 0xD3 )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		const auto reg_field = ( instr.modrm_byte >> 3 ) & 0x07;

		if ( reg_field == 0 )
		{
			instr.type = instruction_type::rol;
		}
		else if ( reg_field == 1 )
		{
			instr.type = instruction_type::ror;
		}
		else if ( reg_field == 2 )
		{
			instr.type = instruction_type::rcl;
		}
		else if ( reg_field == 3 )
		{
			instr.type = instruction_type::rcr;
		}
		else if ( reg_field == 4 )
		{
			instr.type = instruction_type::shl;
		}
		else if ( reg_field == 5 )
		{
			instr.type = instruction_type::shr;
		}
		else if ( reg_field == 7 )
		{
			instr.type = instruction_type::sar;
		}
		else
		{
			return false;
		}

		const bool is_byte = ( instr.opcode == 0xC0 || instr.opcode == 0xD0 || instr.opcode == 0xD2 );

		instr.operands[ 0 ] = rm_operand;
		instr.operands[ 0 ].size = is_byte ? operand_size::byte :
			( instr.rex_w ? operand_size::qword : operand_size::dword );

		instr.operands[ 1 ].type = operand_type::imm;
		instr.operands[ 1 ].size = operand_size::byte;

		if ( instr.opcode == 0xC0 || instr.opcode == 0xC1 )
		{
			if ( offset >= max_length )
			{
				return false;
			}

			instr.operands[ 1 ].imm = bytes[ offset++ ];
		}
		else if ( instr.opcode == 0xD0 || instr.opcode == 0xD1 )
		{
			instr.operands[ 1 ].imm = 1;
		}
		else
		{
			instr.operands[ 1 ].imm = cpu_state::cl_sentinel;
		}

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t > ( offset );
		return true;
	}

	if ( instr.opcode == 0x80 )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		const auto operation = ( instr.modrm_byte >> 3 ) & 0x07;

		if ( operation == 0 )
		{
			instr.type = instruction_type::add;
		}
		else if ( operation == 1 )
		{
			instr.type = instruction_type::or_instr;
		}
		else if ( operation == 2 )
		{
			instr.type = instruction_type::adc;
		}
		else if ( operation == 3 )
		{
			instr.type = instruction_type::sbb;
		}
		else if ( operation == 4 )
		{
			instr.type = instruction_type::and_instr;
		}
		else if ( operation == 5 )
		{
			instr.type = instruction_type::sub;
		}
		else if ( operation == 6 )
		{
			instr.type = instruction_type::xor_instr;
		}
		else if ( operation == 7 )
		{
			instr.type = instruction_type::cmp;
		}
		else
		{
			return false;
		}

		instr.operands[ 0 ] = rm_operand;
		instr.operands[ 0 ].size = operand_size::byte;

		instr.operands[ 1 ].type = operand_type::imm;
		instr.operands[ 1 ].imm = bytes[ offset++ ];
		instr.operands[ 1 ].size = operand_size::byte;

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( instr.opcode == 0x81 || instr.opcode == 0x83 )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		const auto operation = ( instr.modrm_byte >> 3 ) & 0x07;

		if ( operation == 0 )
		{
			instr.type = instruction_type::add;
		}
		else if ( operation == 1 )
		{
			instr.type = instruction_type::or_instr;
		}
		else if ( operation == 2 )
		{
			instr.type = instruction_type::adc;
		}
		else if ( operation == 3 )
		{
			instr.type = instruction_type::sbb;
		}
		else if ( operation == 4 )
		{
			instr.type = instruction_type::and_instr;
		}
		else if ( operation == 5 )
		{
			instr.type = instruction_type::sub;
		}
		else if ( operation == 6 )
		{
			instr.type = instruction_type::xor_instr;
		}
		else if ( operation == 7 )
		{
			instr.type = instruction_type::cmp;
		}
		else
		{
			return false;
		}

		instr.operands[ 0 ] = rm_operand;
		instr.operands[ 0 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;

		instr.operands[ 1 ].type = operand_type::imm;
		if ( instr.opcode == 0x83 )
		{
			std::int8_t imm8 = static_cast< std::int8_t >( bytes[ offset++ ] );
			instr.operands[ 1 ].imm = static_cast< std::int64_t >( imm8 );
		}
		else
		{
			std::int32_t imm32 = 0;
			std::memcpy ( &imm32, &bytes[ offset ], 4 );
			offset += 4;
			instr.operands[ 1 ].imm = imm32;
		}

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( instr.opcode == 0x00 || instr.opcode == 0x01 ||
		 instr.opcode == 0x02 || instr.opcode == 0x03 )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		instr.type = instruction_type::add;

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		if ( instr.opcode == 0x02 || instr.opcode == 0x03 )
		{
			instr.operands[ 0 ].type = operand_type::reg;
			instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
			instr.operands[ 1 ] = rm_operand;
		}
		else
		{
			instr.operands[ 0 ] = rm_operand;
			instr.operands[ 1 ].type = operand_type::reg;
			instr.operands[ 1 ].reg = this->get_register ( reg, instr.rex_r );
		}

		if ( instr.opcode == 0x00 || instr.opcode == 0x02 )
		{
			instr.operands[ 0 ].size = operand_size::byte;
			instr.operands[ 1 ].size = operand_size::byte;
		}
		else
		{
			instr.operands[ 0 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;
			instr.operands[ 1 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;
		}

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	return false;
}

inline auto decoder::decode_ret ( const std::uint8_t* bytes, std::size_t& offset,
								  instruction& instr ) -> bool
{

	instr.type = instruction_type::ret;
	instr.operand_count = 0;
	instr.length = static_cast< std::uint8_t >( offset );
	return true;
}

inline auto decoder::decode_jcc ( const std::uint8_t* bytes, std::size_t& offset,
								  instruction& instr ) -> bool
{

	switch ( instr.opcode )
	{
		case 0x70: instr.type = instruction_type::jo; break;
		case 0x71: instr.type = instruction_type::jno; break;
		case 0x72: instr.type = instruction_type::jb; break;
		case 0x73: instr.type = instruction_type::jae; break;
		case 0x74: instr.type = instruction_type::je; break;
		case 0x75: instr.type = instruction_type::jne; break;
		case 0x76: instr.type = instruction_type::jbe; break;
		case 0x77: instr.type = instruction_type::ja; break;
		case 0x78: instr.type = instruction_type::js; break;
		case 0x79: instr.type = instruction_type::jns; break;
		case 0x7A: instr.type = instruction_type::jp; break;
		case 0x7B: instr.type = instruction_type::jnp; break;
		case 0x7C: instr.type = instruction_type::jl; break;
		case 0x7D: instr.type = instruction_type::jge; break;
		case 0x7E: instr.type = instruction_type::jle; break;
		case 0x7F: instr.type = instruction_type::jg; break;
		default: return false;
	}

	std::int8_t rel8 = static_cast< std::int8_t >( bytes[ offset++ ] );

	instr.operands[ 0 ].type = operand_type::imm;
	instr.operands[ 0 ].imm = static_cast< std::int64_t >( rel8 );
	instr.operand_count = 1;

	instr.length = static_cast< std::uint8_t >( offset );
	return true;
}

inline auto decoder::decode_call ( const std::uint8_t* bytes, std::size_t& offset,
								   std::size_t max_length, instruction& instr ) -> bool
{
	instr.type = instruction_type::call;

	if ( instr.opcode == 0xE8 )
	{
		std::int32_t rel32 = 0;
		std::memcpy ( &rel32, &bytes[ offset ], 4 );
		offset += 4;

		instr.operands[ 0 ].type = operand_type::imm;
		instr.operands[ 0 ].imm = static_cast< std::int64_t >( rel32 );
		instr.operand_count = 1;

		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( instr.opcode == 0xFF )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		const auto reg_field = ( instr.modrm_byte >> 3 ) & 0x07;
		if ( reg_field != 2 )
		{
			return false;
		}

		instr.operands[ 0 ] = rm_operand;
		instr.operands[ 0 ].size = operand_size::qword;
		instr.operand_count = 1;

		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	return false;
}

inline auto decoder::decode_jmp ( const std::uint8_t* bytes, std::size_t& offset,
								  std::size_t max_length, instruction& instr ) -> bool
{
	instr.type = instruction_type::jmp;

	if ( instr.opcode == 0xEB )
	{
		std::int8_t rel8 = static_cast< std::int8_t >( bytes[ offset++ ] );
		instr.operands[ 0 ].type = operand_type::imm;
		instr.operands[ 0 ].imm = static_cast< std::int64_t >( rel8 );
		instr.operand_count = 1;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( instr.opcode == 0xE9 )
	{
		std::int32_t rel32 = 0;
		std::memcpy ( &rel32, &bytes[ offset ], 4 );
		offset += 4;
		instr.operands[ 0 ].type = operand_type::imm;
		instr.operands[ 0 ].imm = static_cast< std::int64_t >( rel32 );
		instr.operand_count = 1;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( instr.opcode == 0xFF )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		const auto reg_field = ( instr.modrm_byte >> 3 ) & 0x07;
		if ( reg_field != 4 )
		{
			return false;
		}

		instr.operands[ 0 ] = rm_operand;
		instr.operands[ 0 ].size = operand_size::qword;
		instr.operand_count = 1;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	return false;
}

inline auto decoder::decode_two_byte ( const std::uint8_t* bytes, std::size_t& offset,
									   std::size_t max_length, instruction& instr ) -> bool
{
	if ( offset >= max_length )
	{
		return false;
	}

	const auto second_byte = bytes[ offset++ ];

	if ( second_byte == 0x01 )
	{
		if ( offset >= max_length )
		{
			return false;
		}

		const auto third_byte = bytes[ offset++ ];

		if ( third_byte == 0xF9 )
		{
			instr.type = instruction_type::rdtscp;
			instr.operand_count = 0;
			instr.length = static_cast< std::uint8_t >( offset );
			return true;
		}

		return false;
	}

	if ( second_byte == 0x05 )
	{
		instr.type = instruction_type::syscall;
		instr.operand_count = 0;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0xC7 )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		if ( reg == 1 )
		{
			instr.type = instr.rex_w ? instruction_type::cmpxchg16b : instruction_type::cmpxchg8b;

			instr.operands[ 0 ] = rm_operand;
			instr.operands[ 0 ].size = instr.rex_w ? operand_size::oword : operand_size::qword;
			instr.operand_count = 1;
		}
		else
		{
			instr.type = instruction_type::nop;
			instr.operand_count = 0;
		}

		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0x2E || second_byte == 0x2F )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		instr.type = instruction_type::nop;
		instr.operand_count = 0;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0x2E || second_byte == 0x2F )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		instr.type = instr.has_operand_size
			? ( second_byte == 0x2E ? instruction_type::ucomisd : instruction_type::comisd )
			: ( second_byte == 0x2E ? instruction_type::ucomiss : instruction_type::comiss );

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		instr.operands[ 0 ].type = operand_type::xmm;
		instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
		instr.operands[ 0 ].size = operand_size::qword;

		if ( rm_operand.type == operand_type::reg )
		{
			instr.operands[ 1 ].type = operand_type::xmm;
			instr.operands[ 1 ].reg = rm_operand.reg;
			instr.operands[ 1 ].size = operand_size::qword;
		}
		else
		{
			instr.operands[ 1 ] = rm_operand;
			instr.operands[ 1 ].size = operand_size::qword;
		}

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0xEF )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		instr.type = instruction_type::pxor;

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		instr.operands[ 0 ].type = operand_type::xmm;
		instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
		instr.operands[ 0 ].size = operand_size::oword;

		if ( rm_operand.type == operand_type::reg )
		{
			instr.operands[ 1 ].type = operand_type::xmm;
			instr.operands[ 1 ].reg = rm_operand.reg;
			instr.operands[ 1 ].size = operand_size::oword;
		}
		else
		{
			instr.operands[ 1 ] = rm_operand;
			instr.operands[ 1 ].size = operand_size::oword;
		}

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0xDB )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		instr.type = instruction_type::pand;

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		instr.operands[ 0 ].type = operand_type::xmm;
		instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
		instr.operands[ 0 ].size = operand_size::oword;

		if ( rm_operand.type == operand_type::reg )
		{
			instr.operands[ 1 ].type = operand_type::xmm;
			instr.operands[ 1 ].reg = rm_operand.reg;
			instr.operands[ 1 ].size = operand_size::oword;
		}
		else
		{
			instr.operands[ 1 ] = rm_operand;
			instr.operands[ 1 ].size = operand_size::oword;
		}

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0xEB )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		instr.type = instruction_type::por;

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		instr.operands[ 0 ].type = operand_type::xmm;
		instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
		instr.operands[ 0 ].size = operand_size::oword;

		if ( rm_operand.type == operand_type::reg )
		{
			instr.operands[ 1 ].type = operand_type::xmm;
			instr.operands[ 1 ].reg = rm_operand.reg;
			instr.operands[ 1 ].size = operand_size::oword;
		}
		else
		{
			instr.operands[ 1 ] = rm_operand;
			instr.operands[ 1 ].size = operand_size::oword;
		}

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0x5B )
	{
		instr.type = instr.has_operand_size
			? instruction_type::cvtps2dq
			: instr.has_rep
			? instruction_type::cvttps2dq
			: instruction_type::cvtdq2ps;

		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		if ( rm_operand.type == operand_type::reg )
		{
			rm_operand.type = operand_type::xmm;
			rm_operand.xmm = rm_operand.reg;
			rm_operand.size = operand_size::oword;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		instr.operands[ 0 ].type = operand_type::xmm;
		instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
		instr.operands[ 0 ].size = operand_size::oword;
		instr.operands[ 1 ] = rm_operand;
		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t > ( offset );
		return true;
	}

	if ( second_byte == 0x5D )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		if ( instr.has_rep )
		{
			instr.type = instruction_type::minss;
		}
		else if ( instr.has_repne )
		{
			instr.type = instruction_type::minsd;
		}
		else if ( instr.has_operand_size )
		{
			instr.type = instruction_type::minpd;
		}
		else
		{
			instr.type = instruction_type::minps;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		instr.operands[ 0 ].type = operand_type::xmm;
		instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
		instr.operands[ 0 ].size = operand_size::qword;

		if ( rm_operand.type == operand_type::reg )
		{
			instr.operands[ 1 ].type = operand_type::xmm;
			instr.operands[ 1 ].reg = rm_operand.reg;
			instr.operands[ 1 ].size = operand_size::qword;
		}
		else
		{
			instr.operands[ 1 ] = rm_operand;
			instr.operands[ 1 ].size = operand_size::qword;
		}

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0x5F )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		if ( instr.has_rep )
		{
			instr.type = instruction_type::maxss;
		}
		else if ( instr.has_repne )
		{
			instr.type = instruction_type::maxsd;
		}
		else if ( instr.has_operand_size )
		{
			instr.type = instruction_type::maxpd;
		}
		else
		{
			instr.type = instruction_type::maxps;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		instr.operands[ 0 ].type = operand_type::xmm;
		instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
		instr.operands[ 0 ].size = operand_size::qword;

		if ( rm_operand.type == operand_type::reg )
		{
			instr.operands[ 1 ].type = operand_type::xmm;
			instr.operands[ 1 ].reg = rm_operand.reg;
			instr.operands[ 1 ].size = operand_size::qword;
		}
		else
		{
			instr.operands[ 1 ] = rm_operand;
			instr.operands[ 1 ].size = operand_size::qword;
		}

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0x70 )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		if ( instr.has_operand_size )
		{
			instr.type = instruction_type::pshufd;
		}
		else if ( instr.has_rep )
		{
			instr.type = instruction_type::pshufhw;
		}
		else if ( instr.has_repne )
		{
			instr.type = instruction_type::pshuflw;
		}
		else
		{
			return false;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		instr.operands[ 0 ].type = operand_type::xmm;
		instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
		instr.operands[ 0 ].size = operand_size::oword;

		if ( rm_operand.type == operand_type::reg )
		{
			instr.operands[ 1 ].type = operand_type::xmm;
			instr.operands[ 1 ].reg = rm_operand.reg;
			instr.operands[ 1 ].size = operand_size::oword;
		}
		else
		{
			instr.operands[ 1 ] = rm_operand;
			instr.operands[ 1 ].size = operand_size::oword;
		}

		instr.operands[ 2 ].type = operand_type::imm;
		instr.operands[ 2 ].imm = bytes[ offset++ ];
		instr.operand_count = 3;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0x71 || second_byte == 0x72 || second_byte == 0x73 )
	{
		if ( offset >= max_length )
		{
			return false;
		}

		const auto modrm = bytes[ offset++ ];
		instr.modrm_byte = modrm;
		instr.has_modrm = true;

		const auto reg_field = ( modrm >> 3 ) & 0x07;
		const auto rm = modrm & 0x07;

		if ( second_byte == 0x71 )
		{
			if ( reg_field == 2 ) 
			{
				instr.type = instruction_type::psrlw;
			}
			else if ( reg_field == 4 ) 
			{
				instr.type = instruction_type::psraw;
			}
			else if ( reg_field == 6 ) 
			{
				instr.type = instruction_type::psllw;
			}
			else return false;
		}
		else if ( second_byte == 0x72 )
		{
			if ( reg_field == 2 )
			{
				instr.type = instruction_type::psrld;
			}
			else if ( reg_field == 4 )
			{
				instr.type = instruction_type::psrad;
			}
			else if ( reg_field == 6 )
			{
				instr.type = instruction_type::pslld;
			}
			else return false;
		}
		else
		{
			if ( reg_field == 2 ) 
			{
				instr.type = instruction_type::psrlq;
			}
			else if ( reg_field == 3 ) 
			{
				instr.type = instruction_type::psrldq;
			}
			else if ( reg_field == 6 ) 
			{
				instr.type = instruction_type::pslldq;
			}
			else if ( reg_field == 7 ) 
			{
				instr.type = instruction_type::psllq;
			}
			else return false;
		}

		instr.operands[ 0 ].type = operand_type::xmm;
		instr.operands[ 0 ].reg = this->get_register ( rm, instr.rex_b );
		instr.operands[ 0 ].size = operand_size::oword;

		instr.operands[ 1 ].type = operand_type::imm;
		instr.operands[ 1 ].imm = bytes[ offset++ ];

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0xAE )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		if ( reg == 3 )
		{
			instr.type = instruction_type::stmxcsr;
			instr.operands[ 0 ] = rm_operand;
			instr.operands[ 0 ].size = operand_size::dword;
			instr.operand_count = 1;
		}
		else
		{
			instr.type = instruction_type::nop;
			instr.operand_count = 0;
		}

		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0xC0 || second_byte == 0xC1 )
	{
		instr.type = instruction_type::xadd;

		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		instr.operands[ 0 ] = rm_operand;
		instr.operands[ 0 ].size = ( second_byte == 0xC0 ) ? operand_size::byte :
			( instr.rex_w ? operand_size::qword : operand_size::dword );

		instr.operands[ 1 ].type = operand_type::reg;
		instr.operands[ 1 ].reg = this->get_register ( reg, instr.rex_r );
		instr.operands[ 1 ].size = instr.operands[ 0 ].size;

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0xB0 || second_byte == 0xB1 )
	{
		instr.type = instruction_type::cmpxchg;

		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		instr.operands[ 0 ] = rm_operand;
		instr.operands[ 0 ].size = ( second_byte == 0xB0 ) ? operand_size::byte :
			( instr.rex_w ? operand_size::qword : operand_size::dword );

		instr.operands[ 1 ].type = operand_type::reg;
		instr.operands[ 1 ].reg = this->get_register ( reg, instr.rex_r );
		instr.operands[ 1 ].size = instr.operands[ 0 ].size;

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0xBA )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		const auto reg_field = ( instr.modrm_byte >> 3 ) & 0x07;

		if ( reg_field == 4 )
		{
			instr.type = instruction_type::bt;
		}
		else if ( reg_field == 5 )
		{
			instr.type = instruction_type::bts;
		}
		else if ( reg_field == 6 )
		{
			instr.type = instruction_type::btr;
		}
		else if ( reg_field == 7 )
		{
			instr.type = instruction_type::btc;
		}
		else
		{
			return false;
		}

		instr.operands[ 0 ] = rm_operand;
		instr.operands[ 0 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;

		instr.operands[ 1 ].type = operand_type::imm;
		instr.operands[ 1 ].imm = bytes[ offset++ ];
		instr.operands[ 1 ].size = operand_size::byte;

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0x1F )
	{
		instr.type = instruction_type::nop;

		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		instr.operand_count = 0;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0x31 )
	{
		instr.type = instruction_type::rdtsc;
		instr.operand_count = 0;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0xA2 )
	{
		instr.type = instruction_type::cpuid;
		instr.operand_count = 0;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte >= 0x80 && second_byte <= 0x8F )
	{

		switch ( second_byte )
		{
			case 0x80: instr.type = instruction_type::jo; break;
			case 0x81: instr.type = instruction_type::jno; break;
			case 0x82: instr.type = instruction_type::jb; break;
			case 0x83: instr.type = instruction_type::jae; break;
			case 0x84: instr.type = instruction_type::je; break;
			case 0x85: instr.type = instruction_type::jne; break;
			case 0x86: instr.type = instruction_type::jbe; break;
			case 0x87: instr.type = instruction_type::ja; break;
			case 0x88: instr.type = instruction_type::js; break;
			case 0x89: instr.type = instruction_type::jns; break;
			case 0x8A: instr.type = instruction_type::jp; break;
			case 0x8B: instr.type = instruction_type::jnp; break;
			case 0x8C: instr.type = instruction_type::jl; break;
			case 0x8D: instr.type = instruction_type::jge; break;
			case 0x8E: instr.type = instruction_type::jle; break;
			case 0x8F: instr.type = instruction_type::jg; break;
			default: return false;
		}

		if ( offset + 4 > max_length )
		{
			return false;
		}

		const auto rel32 = *reinterpret_cast< const std::int32_t* >( &bytes[ offset ] );
		offset += 4;

		instr.operands[ 0 ].type = operand_type::imm;
		instr.operands[ 0 ].imm = static_cast< std::uint64_t >( static_cast< std::int64_t >( rel32 ) );
		instr.operand_count = 1;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte >= 0x90 && second_byte <= 0x9F )
	{

		switch ( second_byte )
		{
			case 0x90: instr.type = instruction_type::seto; break;
			case 0x91: instr.type = instruction_type::setno; break;
			case 0x92: instr.type = instruction_type::setb; break;
			case 0x93: instr.type = instruction_type::setae; break;
			case 0x94: instr.type = instruction_type::sete; break;
			case 0x95: instr.type = instruction_type::setne; break;
			case 0x96: instr.type = instruction_type::setbe; break;
			case 0x97: instr.type = instruction_type::seta; break;
			case 0x98: instr.type = instruction_type::sets; break;
			case 0x99: instr.type = instruction_type::setns; break;
			case 0x9A: instr.type = instruction_type::sets; break;
			case 0x9B: instr.type = instruction_type::setns; break;
			case 0x9C: instr.type = instruction_type::setl; break;
			case 0x9D: instr.type = instruction_type::setge; break;
			case 0x9E: instr.type = instruction_type::setle; break;
			case 0x9F: instr.type = instruction_type::setg; break;
			default: return false;
		}

		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		instr.operands[ 0 ] = rm_operand;
		instr.operands[ 0 ].size = operand_size::byte;
		instr.operand_count = 1;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte >= 0x40 && second_byte <= 0x4F )
	{

		switch ( second_byte )
		{
			case 0x40: instr.type = instruction_type::cmovo; break;
			case 0x41: instr.type = instruction_type::cmovno; break;
			case 0x42: instr.type = instruction_type::cmovb; break;
			case 0x43: instr.type = instruction_type::cmovae; break;
			case 0x44: instr.type = instruction_type::cmove; break;
			case 0x45: instr.type = instruction_type::cmovne; break;
			case 0x46: instr.type = instruction_type::cmovbe; break;
			case 0x47: instr.type = instruction_type::cmova; break;
			case 0x48: instr.type = instruction_type::cmovs; break;
			case 0x49: instr.type = instruction_type::cmovns; break;
			case 0x4C: instr.type = instruction_type::cmovl; break;
			case 0x4D: instr.type = instruction_type::cmovge; break;
			case 0x4E: instr.type = instruction_type::cmovle; break;
			case 0x4F: instr.type = instruction_type::cmovg; break;
			default: return false;
		}

		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;
		instr.operands[ 0 ].type = operand_type::reg;
		instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
		instr.operands[ 0 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;

		instr.operands[ 1 ] = rm_operand;
		instr.operands[ 1 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0x7E || second_byte == 0x7F )
	{
		if ( instr.has_rep )
		{
			operand rm_operand;
			if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
			{
				return false;
			}

			const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

			if ( second_byte == 0x7F )
			{
				instr.type = instruction_type::movdqu;
				instr.operands[ 0 ] = rm_operand;
				instr.operands[ 1 ].type = operand_type::xmm;
				instr.operands[ 1 ].xmm = this->get_register ( reg, instr.rex_r );
			}
			else
			{
				instr.type = instruction_type::movq;
				instr.operands[ 0 ].type = operand_type::xmm;
				instr.operands[ 0 ].xmm = this->get_register ( reg, instr.rex_r );
				instr.operands[ 1 ] = rm_operand;
			}

			instr.operands[ 0 ].size = operand_size::oword;
			instr.operands[ 1 ].size = operand_size::oword;
			instr.operand_count = 2;
			instr.length = static_cast< std::uint8_t >( offset );
			return true;
		}
	}

	if ( second_byte == 0xBE || second_byte == 0xBF )
	{
		instr.type = instruction_type::movsx;

		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;
		instr.operands[ 0 ].type = operand_type::reg;
		instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
		instr.operands[ 0 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;

		instr.operands[ 1 ] = rm_operand;
		instr.operands[ 1 ].size = ( second_byte == 0xBE ) ? operand_size::byte : operand_size::word;

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0xB6 || second_byte == 0xB7 )
	{
		instr.type = instruction_type::movzx;

		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;
		instr.operands[ 0 ].type = operand_type::reg;
		instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
		instr.operands[ 0 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;

		instr.operands[ 1 ] = rm_operand;
		instr.operands[ 1 ].size = ( second_byte == 0xB6 ) ? operand_size::byte : operand_size::word;

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0xAF )
	{
		instr.type = instruction_type::imul;

		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;
		instr.operands[ 0 ].type = operand_type::reg;
		instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
		instr.operands[ 0 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;

		instr.operands[ 1 ] = rm_operand;
		instr.operands[ 1 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0xA3 || second_byte == 0xAB ||
		 second_byte == 0xB3 || second_byte == 0xBB )
	{
		if ( second_byte == 0xA3 )
		{
			instr.type = instruction_type::bt;
		}
		else if ( second_byte == 0xAB )
		{
			instr.type = instruction_type::bts;
		}
		else if ( second_byte == 0xB3 )
		{
			instr.type = instruction_type::btr;
		}
		else
		{
			instr.type = instruction_type::btc;
		}

		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		instr.operands[ 0 ] = rm_operand;
		instr.operands[ 0 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;

		instr.operands[ 1 ].type = operand_type::reg;
		instr.operands[ 1 ].reg = this->get_register ( reg, instr.rex_r );
		instr.operands[ 1 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte >= 0xC8 && second_byte <= 0xCF )
	{
		instr.type = instruction_type::bswap;

		instr.operands[ 0 ].type = operand_type::reg;
		instr.operands[ 0 ].reg = this->get_register ( second_byte & 0x07, instr.rex_b );
		instr.operands[ 0 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;

		instr.operand_count = 1;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0xBC || second_byte == 0xBD )
	{
		instr.type = ( second_byte == 0xBC ) ? instruction_type::bsf : instruction_type::bsr;

		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;
		instr.operands[ 0 ].type = operand_type::reg;
		instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
		instr.operands[ 0 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;

		instr.operands[ 1 ] = rm_operand;
		instr.operands[ 1 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0x2A )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		if ( instr.has_repne )
		{
			instr.type = instruction_type::cvtsi2sd;
		}
		else if ( instr.has_rep )
		{
			instr.type = instruction_type::cvtsi2ss;
		}
		else
		{
			return false;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		instr.operands[ 0 ].type = operand_type::xmm;
		instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
		instr.operands[ 0 ].size = operand_size::oword;

		instr.operands[ 1 ] = rm_operand;
		instr.operands[ 1 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0x5A )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		if ( instr.has_repne )
		{
			instr.type = instruction_type::cvtsd2ss;
		}
		else if ( instr.has_rep )
		{
			instr.type = instruction_type::cvtss2sd;
		}
		else if ( instr.has_operand_size )
		{
			instr.type = instruction_type::cvtpd2ps;
		}
		else
		{
			instr.type = instruction_type::cvtps2pd;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;
		const auto xmm_reg = this->get_register ( reg, instr.rex_r );

		instr.operands[ 0 ].type = operand_type::xmm;
		instr.operands[ 0 ].xmm = xmm_reg;

		if ( rm_operand.type == operand_type::reg )
		{
			instr.operands[ 1 ].type = operand_type::xmm;
			instr.operands[ 1 ].xmm = rm_operand.reg;
		}
		else
		{
			instr.operands[ 1 ] = rm_operand;
		}

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0x2C || second_byte == 0x2D )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		if ( instr.has_repne )
		{
			instr.type = instruction_type::cvttsd2si;
		}
		else if ( instr.has_rep )
		{
			instr.type = instruction_type::cvttss2si;
		}
		else
		{
			return false;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		instr.operands[ 0 ].type = operand_type::reg;
		instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
		instr.operands[ 0 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;

		instr.operands[ 1 ] = rm_operand;
		if ( rm_operand.type == operand_type::reg )
		{
			instr.operands[ 1 ].type = operand_type::xmm;
		}

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0x55 )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		if ( instr.has_operand_size )
		{
			instr.type = instruction_type::andnpd;
		}
		else
		{
			instr.type = instruction_type::andnps;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		instr.operands[ 0 ].type = operand_type::xmm;
		instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
		instr.operands[ 0 ].size = operand_size::oword;

		if ( rm_operand.type == operand_type::reg )
		{
			instr.operands[ 1 ].type = operand_type::xmm;
			instr.operands[ 1 ].reg = rm_operand.reg;
			instr.operands[ 1 ].size = operand_size::oword;
		}
		else
		{
			instr.operands[ 1 ] = rm_operand;
			instr.operands[ 1 ].size = operand_size::oword;
		}

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0x58 )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		if ( instr.has_rep )
		{
			instr.type = instruction_type::addss;
		}
		else if ( instr.has_repne )
		{
			instr.type = instruction_type::addsd;
		}
		else if ( instr.has_operand_size )
		{
			instr.type = instruction_type::addpd;
		}
		else
		{
			instr.type = instruction_type::addps;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		instr.operands[ 0 ].type = operand_type::xmm;
		instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
		instr.operands[ 0 ].size = operand_size::oword;

		if ( rm_operand.type == operand_type::reg )
		{
			instr.operands[ 1 ].type = operand_type::xmm;
			instr.operands[ 1 ].reg = rm_operand.reg;
			instr.operands[ 1 ].size = operand_size::oword;
		}
		else
		{
			instr.operands[ 1 ] = rm_operand;
			instr.operands[ 1 ].size = operand_size::oword;
		}

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0x59 )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		if ( instr.has_rep )
		{
			instr.type = instruction_type::mulss;
		}
		else if ( instr.has_repne )
		{
			instr.type = instruction_type::mulsd;
		}
		else if ( instr.has_operand_size )
		{
			instr.type = instruction_type::mulpd;
		}
		else
		{
			instr.type = instruction_type::mulps;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		instr.operands[ 0 ].type = operand_type::xmm;
		instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
		instr.operands[ 0 ].size = operand_size::oword;

		if ( rm_operand.type == operand_type::reg )
		{
			instr.operands[ 1 ].type = operand_type::xmm;
			instr.operands[ 1 ].reg = rm_operand.reg;
			instr.operands[ 1 ].size = operand_size::oword;
		}
		else
		{
			instr.operands[ 1 ] = rm_operand;
			instr.operands[ 1 ].size = operand_size::oword;
		}

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0x5C )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		if ( instr.has_rep )
		{
			instr.type = instruction_type::subss;
		}
		else if ( instr.has_repne )
		{
			instr.type = instruction_type::subsd;
		}
		else if ( instr.has_operand_size )
		{
			instr.type = instruction_type::subpd;
		}
		else
		{
			instr.type = instruction_type::subps;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		instr.operands[ 0 ].type = operand_type::xmm;
		instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
		instr.operands[ 0 ].size = operand_size::oword;

		if ( rm_operand.type == operand_type::reg )
		{
			instr.operands[ 1 ].type = operand_type::xmm;
			instr.operands[ 1 ].reg = rm_operand.reg;
			instr.operands[ 1 ].size = operand_size::oword;
		}
		else
		{
			instr.operands[ 1 ] = rm_operand;
			instr.operands[ 1 ].size = operand_size::oword;
		}

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0x5E )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		if ( instr.has_rep )
		{
			instr.type = instruction_type::divss;
		}
		else if ( instr.has_repne )
		{
			instr.type = instruction_type::divsd;
		}
		else if ( instr.has_operand_size )
		{
			instr.type = instruction_type::divpd;
		}
		else
		{
			instr.type = instruction_type::divps;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		instr.operands[ 0 ].type = operand_type::xmm;
		instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
		instr.operands[ 0 ].size = operand_size::oword;

		if ( rm_operand.type == operand_type::reg )
		{
			instr.operands[ 1 ].type = operand_type::xmm;
			instr.operands[ 1 ].reg = rm_operand.reg;
			instr.operands[ 1 ].size = operand_size::oword;
		}
		else
		{
			instr.operands[ 1 ] = rm_operand;
			instr.operands[ 1 ].size = operand_size::oword;
		}

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0x51 )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		if ( instr.has_rep )
		{
			instr.type = instruction_type::sqrtss;
		}
		else if ( instr.has_repne )
		{
			instr.type = instruction_type::sqrtsd;
		}
		else
		{
			return false;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		instr.operands[ 0 ].type = operand_type::xmm;
		instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );

		if ( rm_operand.type == operand_type::reg )
		{
			instr.operands[ 1 ].type = operand_type::xmm;
			instr.operands[ 1 ].xmm = rm_operand.reg;
		}
		else
		{
			instr.operands[ 1 ] = rm_operand;
		}

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0x53 )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		if ( instr.has_rep )
		{
			instr.type = instruction_type::rcpss;
		}
		else
		{
			instr.type = instruction_type::rcpps;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;
		const auto src_size = ( instr.type == instruction_type::rcpss )
			? operand_size::dword
			: operand_size::oword;

		instr.operands[ 0 ].type = operand_type::xmm;
		instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
		instr.operands[ 0 ].size = operand_size::oword;

		if ( rm_operand.type == operand_type::reg )
		{
			instr.operands[ 1 ].type = operand_type::xmm;
			instr.operands[ 1 ].reg = rm_operand.reg;
			instr.operands[ 1 ].size = src_size;
		}
		else
		{
			instr.operands[ 1 ] = rm_operand;
			instr.operands[ 1 ].size = src_size;
		}

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0x10 || second_byte == 0x11 )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		if ( instr.has_rep )
		{
			instr.type = instruction_type::movss;
		}
		else if ( instr.has_repne )
		{
			instr.type = instruction_type::movsd;
		}
		else if ( !instr.has_operand_size )
		{
			instr.type = instruction_type::movups;
		}
		else if ( instr.has_operand_size )
		{
			instr.type = instruction_type::movapd;
		}
		else
		{
			return false;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;
		const auto xmm_reg = this->get_register ( reg, instr.rex_r );

		if ( second_byte == 0x10 )
		{
			instr.operands[ 0 ].type = operand_type::xmm;
			instr.operands[ 0 ].xmm = xmm_reg;

			if ( rm_operand.type == operand_type::reg )
			{
				instr.operands[ 1 ].type = operand_type::xmm;
				instr.operands[ 1 ].xmm = rm_operand.reg;
			}
			else
			{
				instr.operands[ 1 ] = rm_operand;
			}
		}
		else
		{
			if ( rm_operand.type == operand_type::reg )
			{
				instr.operands[ 0 ].type = operand_type::xmm;
				instr.operands[ 0 ].xmm = rm_operand.reg;
			}
			else
			{
				instr.operands[ 0 ] = rm_operand;
			}

			instr.operands[ 1 ].type = operand_type::xmm;
			instr.operands[ 1 ].xmm = xmm_reg;
		}

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0x28 || second_byte == 0x29 )
	{
		instr.type = instruction_type::movaps;

		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;
		const auto xmm_reg = this->get_register ( reg, instr.rex_r );

		if ( second_byte == 0x28 )
		{
			instr.operands[ 0 ].type = operand_type::xmm;
			instr.operands[ 0 ].xmm = xmm_reg;

			if ( rm_operand.type == operand_type::reg )
			{
				instr.operands[ 1 ].type = operand_type::xmm;
				instr.operands[ 1 ].xmm = rm_operand.reg;
			}
			else
			{
				instr.operands[ 1 ] = rm_operand;
			}
		}
		else
		{
			if ( rm_operand.type == operand_type::reg )
			{
				instr.operands[ 0 ].type = operand_type::xmm;
				instr.operands[ 0 ].xmm = rm_operand.reg;
			}
			else
			{
				instr.operands[ 0 ] = rm_operand;
			}

			instr.operands[ 1 ].type = operand_type::xmm;
			instr.operands[ 1 ].xmm = xmm_reg;
		}

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte >= 0x60 && second_byte <= 0x6D )
	{
		if ( !instr.has_operand_size )
		{
			return false;
		}

		instruction_type type;

		switch ( second_byte )
		{
			case 0x60: type = instruction_type::punpcklbw;  break;
			case 0x61: type = instruction_type::punpcklwd;  break;
			case 0x62: type = instruction_type::punpckldq;  break;
			case 0x63: type = instruction_type::packsswb;   break;
			case 0x64: type = instruction_type::pcmpgtb;    break;
			case 0x65: type = instruction_type::pcmpgtw;    break;
			case 0x66: type = instruction_type::pcmpgtd;    break;
			case 0x67: type = instruction_type::packuswb;   break;
			case 0x68: type = instruction_type::punpckhbw;  break;
			case 0x69: type = instruction_type::punpckhwd;  break;
			case 0x6A: type = instruction_type::punpckhdq;  break;
			case 0x6B: type = instruction_type::packssdw;   break;
			case 0x6C: type = instruction_type::punpcklqdq; break;
			case 0x6D: type = instruction_type::punpckhqdq; break;
			default:   return false;
		}

		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		instr.type = type;

		instr.operands[ 0 ].type = operand_type::xmm;
		instr.operands[ 0 ].xmm = this->get_register ( reg, instr.rex_r );
		instr.operands[ 0 ].size = operand_size::oword;

		if ( rm_operand.type == operand_type::reg )
		{
			instr.operands[ 1 ].type = operand_type::xmm;
			instr.operands[ 1 ].xmm = rm_operand.reg;
			instr.operands[ 1 ].size = operand_size::oword;
		}
		else
		{
			instr.operands[ 1 ] = rm_operand;
			instr.operands[ 1 ].size = operand_size::oword;
		}

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0x6F || second_byte == 0x7F )
	{
		if ( !instr.has_operand_size && !instr.has_rep )
		{
			return false;
		}

		instr.type = instr.has_rep ? instruction_type::movdqu : instruction_type::movdqa;

		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;
		const auto xmm_reg = this->get_register ( reg, instr.rex_r );

		if ( second_byte == 0x6F )
		{
			instr.operands[ 0 ].type = operand_type::xmm;
			instr.operands[ 0 ].xmm = xmm_reg;

			if ( rm_operand.type == operand_type::reg )
			{
				instr.operands[ 1 ].type = operand_type::xmm;
				instr.operands[ 1 ].xmm = rm_operand.reg;
			}
			else
			{
				instr.operands[ 1 ] = rm_operand;
			}
		}
		else
		{
			if ( rm_operand.type == operand_type::reg )
			{
				instr.operands[ 0 ].type = operand_type::xmm;
				instr.operands[ 0 ].xmm = rm_operand.reg;
			}
			else
			{
				instr.operands[ 0 ] = rm_operand;
			}

			instr.operands[ 1 ].type = operand_type::xmm;
			instr.operands[ 1 ].xmm = xmm_reg;
		}

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0x7E )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;
		const auto xmm_reg = this->get_register ( reg, instr.rex_r );

		if ( instr.has_operand_size )
		{
			instr.type = instruction_type::movq;

			instr.operands[ 0 ] = rm_operand;
			instr.operands[ 1 ].type = operand_type::xmm;
			instr.operands[ 1 ].xmm = xmm_reg;
		}
		else if ( instr.has_rep )
		{
			instr.type = instruction_type::movq;

			instr.operands[ 0 ].type = operand_type::xmm;
			instr.operands[ 0 ].xmm = xmm_reg;

			if ( rm_operand.type == operand_type::reg )
			{
				instr.operands[ 1 ].type = operand_type::xmm;
				instr.operands[ 1 ].xmm = rm_operand.reg;
			}
			else
			{
				instr.operands[ 1 ] = rm_operand;
			}
		}
		else
		{
			return false;
		}

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0x6E )
	{
		if ( !instr.has_operand_size )
		{
			return false;
		}

		instr.type = instruction_type::movq;

		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		instr.operands[ 0 ].type = operand_type::xmm;
		instr.operands[ 0 ].xmm = this->get_register ( reg, instr.rex_r );
		instr.operands[ 0 ].size = operand_size::oword;

		instr.operands[ 1 ] = rm_operand;
		instr.operands[ 1 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0x50 )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		if ( instr.has_operand_size )
		{
			instr.type = instruction_type::movmskpd;
		}
		else
		{
			instr.type = instruction_type::movmskps;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		instr.operands[ 0 ].type = operand_type::reg;
		instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
		instr.operands[ 0 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;

		instr.operands[ 1 ].type = operand_type::xmm;
		instr.operands[ 1 ].xmm = rm_operand.reg;
		instr.operands[ 1 ].size = operand_size::qword;

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0xD7 )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		instr.type = instruction_type::pmovmskb;

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		instr.operands[ 0 ].type = operand_type::reg;
		instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
		instr.operands[ 0 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;

		instr.operands[ 1 ].type = operand_type::xmm;
		instr.operands[ 1 ].xmm = rm_operand.reg;
		instr.operands[ 1 ].size = operand_size::qword;

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0x57 )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		if ( instr.has_operand_size )
		{
			instr.type = instruction_type::xorpd;
		}
		else
		{
			instr.type = instruction_type::xorps;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		instr.operands[ 0 ].type = operand_type::xmm;
		instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
		instr.operands[ 0 ].size = operand_size::oword;

		if ( rm_operand.type == operand_type::reg )
		{
			instr.operands[ 1 ].type = operand_type::xmm;
			instr.operands[ 1 ].reg = rm_operand.reg;
			instr.operands[ 1 ].size = operand_size::oword;
		}
		else
		{
			instr.operands[ 1 ] = rm_operand;
			instr.operands[ 1 ].size = operand_size::oword;
		}

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0x54 )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		if ( instr.has_operand_size )
		{
			instr.type = instruction_type::andpd;
		}
		else
		{
			instr.type = instruction_type::andps;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		instr.operands[ 0 ].type = operand_type::xmm;
		instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );

		if ( rm_operand.type == operand_type::reg )
		{
			instr.operands[ 1 ].type = operand_type::xmm;
			instr.operands[ 1 ].xmm = rm_operand.reg;
		}
		else
		{
			instr.operands[ 1 ] = rm_operand;
		}

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0x56 )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		if ( instr.has_operand_size )
		{
			instr.type = instruction_type::orpd;
		}
		else
		{
			instr.type = instruction_type::orps;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		instr.operands[ 0 ].type = operand_type::xmm;
		instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );

		if ( rm_operand.type == operand_type::reg )
		{
			instr.operands[ 1 ].type = operand_type::xmm;
			instr.operands[ 1 ].xmm = rm_operand.reg;
		}
		else
		{
			instr.operands[ 1 ] = rm_operand;
		}

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0xC2 )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		if ( instr.has_rep )
		{
			instr.type = instruction_type::cmpss;
		}
		else if ( instr.has_repne )
		{
			instr.type = instruction_type::cmpsd;
		}
		else if ( instr.has_operand_size )
		{
			instr.type = instruction_type::cmppd;
		}
		else
		{
			instr.type = instruction_type::cmpps;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;
		const auto xmm_reg = this->get_register ( reg, instr.rex_r );

		instr.operands[ 0 ].type = operand_type::xmm;
		instr.operands[ 0 ].xmm = xmm_reg;

		if ( rm_operand.type == operand_type::reg )
		{
			instr.operands[ 1 ].type = operand_type::xmm;
			instr.operands[ 1 ].xmm = rm_operand.reg;
		}
		else
		{
			instr.operands[ 1 ] = rm_operand;
		}

		if ( offset >= max_length )
		{
			return false;
		}

		instr.operands[ 2 ].type = operand_type::imm;
		instr.operands[ 2 ].imm = bytes[ offset++ ];

		instr.operand_count = 3;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0xC6 )
	{
		if ( instr.has_operand_size )
		{
			instr.type = instruction_type::shufpd;
		}
		else
		{
			instr.type = instruction_type::shufps;
		}

		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;
		const auto xmm_reg = this->get_register ( reg, instr.rex_r );

		instr.operands[ 0 ].type = operand_type::xmm;
		instr.operands[ 0 ].xmm = xmm_reg;

		if ( rm_operand.type == operand_type::reg )
		{
			instr.operands[ 1 ].type = operand_type::xmm;
			instr.operands[ 1 ].xmm = rm_operand.reg;
		}
		else
		{
			instr.operands[ 1 ] = rm_operand;
		}

		if ( offset >= max_length )
		{
			return false;
		}

		instr.operands[ 2 ].type = operand_type::imm;
		instr.operands[ 2 ].imm = bytes[ offset++ ];

		instr.operand_count = 3;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0x12 || second_byte == 0x16 )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		if ( second_byte == 0x12 )
		{
			instr.type = instruction_type::movlps;
		}
		else
		{
			instr.type = instruction_type::movhps;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		instr.operands[ 0 ].type = operand_type::xmm;
		instr.operands[ 0 ].xmm = this->get_register ( reg, instr.rex_r );

		if ( rm_operand.type == operand_type::reg )
		{
			instr.operands[ 1 ].type = operand_type::xmm;
			instr.operands[ 1 ].xmm = rm_operand.reg;
		}
		else
		{
			instr.operands[ 1 ] = rm_operand;
		}

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0x14 || second_byte == 0x15 )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}

		if ( second_byte == 0x14 )
		{
			instr.type = instruction_type::unpcklps;
		}
		else
		{
			instr.type = instruction_type::unpckhps;
		}

		const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

		instr.operands[ 0 ].type = operand_type::xmm;
		instr.operands[ 0 ].xmm = this->get_register ( reg, instr.rex_r );

		if ( rm_operand.type == operand_type::reg )
		{
			instr.operands[ 1 ].type = operand_type::xmm;
			instr.operands[ 1 ].xmm = rm_operand.reg;
		}
		else
		{
			instr.operands[ 1 ] = rm_operand;
		}

		instr.operand_count = 2;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	if ( second_byte == 0x0C || second_byte == 0x0D ||
		 second_byte == 0x18 ||
		 ( second_byte >= 0x19 && second_byte <= 0x1E ) )
	{
		operand rm_operand;
		if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
		{
			return false;
		}
		instr.type = instruction_type::nop;
		instr.operand_count = 0;
		instr.length = static_cast< std::uint8_t >( offset );
		return true;
	}

	return false;
}

inline auto decoder::decode_vex ( const std::uint8_t* bytes, std::size_t& offset,
								  std::size_t max_length, instruction& instr ) -> bool
{
	if ( offset >= max_length )
	{
		return false;
	}

	instr.opcode = bytes[ offset++ ];

	if ( instr.vex_map == 1 )
	{
		if ( instr.opcode == 0x6E )
		{
			if ( instr.vex_pp != 1 )
			{
				return false;
			}

			operand rm_operand;
			if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
			{
				return false;
			}

			const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

			instr.type = instruction_type::movd;

			instr.operands[ 0 ].type = operand_type::xmm;
			instr.operands[ 0 ].xmm = this->get_register ( reg, instr.vex_r );
			instr.operands[ 0 ].size = operand_size::oword;

			if ( rm_operand.type == operand_type::reg )
			{
				instr.operands[ 1 ].type = operand_type::reg;
				instr.operands[ 1 ].reg = rm_operand.reg;
				instr.operands[ 1 ].size = instr.vex_w ? operand_size::qword : operand_size::dword;
			}
			else
			{
				instr.operands[ 1 ] = rm_operand;
				instr.operands[ 1 ].size = instr.vex_w ? operand_size::qword : operand_size::dword;
			}

			instr.operand_count = 2;
			instr.length = static_cast< std::uint8_t >( offset );
			return true;
		}

		if ( instr.opcode == 0x58 || instr.opcode == 0x59 ||
			 instr.opcode == 0x5C || instr.opcode == 0x5D ||
			 instr.opcode == 0x5E || instr.opcode == 0x5F ||
			 instr.opcode == 0x51 )
		{
			instruction_type type;
			switch ( instr.opcode )
			{
				case 0x58:
					if ( instr.vex_pp == 0 ) 
					{
						type = instruction_type::addps;
					}
					else if ( instr.vex_pp == 1 )
					{
						type = instruction_type::addpd;
					}
					else if ( instr.vex_pp == 2 )
					{
						type = instruction_type::addss;
					}
					else
					{
						type = instruction_type::addsd;
					}
					break;
				case 0x59:
					if ( instr.vex_pp == 0 ) 
					{
						type = instruction_type::mulps;
					}
					else if ( instr.vex_pp == 1 )
					{
						type = instruction_type::mulpd;
					}
					else if ( instr.vex_pp == 2 ) 
					{
						type = instruction_type::mulss;
					}
					else
					{
						type = instruction_type::mulsd;
					}
					break;
				case 0x5C:
					if ( instr.vex_pp == 0 ) 
					{
						type = instruction_type::subps;
					}
					else if ( instr.vex_pp == 1 ) 
					{
						type = instruction_type::subpd;
					}
					else if ( instr.vex_pp == 2 ) 
					{
						type = instruction_type::subss;
					}
					else 
					{
						type = instruction_type::subsd;
					}
					break;
				case 0x5D:
					if ( instr.vex_pp == 0 ) 
					{
						type = instruction_type::minps;
					}
					else if ( instr.vex_pp == 1 ) 
					{
						type = instruction_type::minpd;
					}
					else if ( instr.vex_pp == 2 ) 
					{
						type = instruction_type::minss;
					}
					else 
					{
						type = instruction_type::minsd;
					}
					break;
				case 0x5E:
					if ( instr.vex_pp == 0 )
					{
						type = instruction_type::divps;
					}
					else if ( instr.vex_pp == 1 ) 
					{
						type = instruction_type::divpd;
					}
					else if ( instr.vex_pp == 2 )
					{
						type = instruction_type::divss;
					}
					else 
					{
						type = instruction_type::divsd;
					}
					break;
				case 0x5F:
					if ( instr.vex_pp == 0 ) 
					{
						type = instruction_type::maxps;
					}
					else if ( instr.vex_pp == 1 ) 
					{
						type = instruction_type::maxpd;
					}
					else if ( instr.vex_pp == 2 ) 
					{
						type = instruction_type::maxss;
					}
					else 
					{
						type = instruction_type::maxsd;
					}
					break;
				case 0x51:
					if ( instr.vex_pp == 2 ) 
					{
						type = instruction_type::sqrtss;
					}
					else if ( instr.vex_pp == 3 ) 
					{
						type = instruction_type::sqrtsd;
					}
					else 
					{
						return false;
					}
					break;
				default: return false;
			}

			operand rm_operand;
			if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
			{
				return false;
			}

			const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

			instr.type = type;

			instr.operands[ 0 ].type = operand_type::xmm;
			instr.operands[ 0 ].xmm = this->get_register ( reg, instr.vex_r );
			instr.operands[ 0 ].size = operand_size::oword;

			instr.operands[ 1 ].type = operand_type::xmm;
			instr.operands[ 1 ].xmm = instr.vex_vvvv;
			instr.operands[ 1 ].size = operand_size::oword;

			if ( rm_operand.type == operand_type::reg )
			{
				instr.operands[ 2 ].type = operand_type::xmm;
				instr.operands[ 2 ].xmm = rm_operand.reg;
				instr.operands[ 2 ].size = operand_size::oword;
			}
			else
			{
				instr.operands[ 2 ] = rm_operand;
				instr.operands[ 2 ].size = operand_size::oword;
			}

			instr.operand_count = 3;
			instr.length = static_cast< std::uint8_t >( offset );
			return true;
		}

		if ( instr.opcode == 0xEF )
		{
			if ( instr.vex_pp != 1 )
			{
				return false;
			}

			operand rm_operand;
			if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
			{
				return false;
			}

			const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

			instr.type = instruction_type::vpxor;

			instr.operands[ 0 ].type = operand_type::xmm;
			instr.operands[ 0 ].xmm = this->get_register ( reg, instr.vex_r );
			instr.operands[ 0 ].size = instr.vex_l ? operand_size::yword : operand_size::oword;

			instr.operands[ 1 ].type = operand_type::xmm;
			instr.operands[ 1 ].xmm = instr.vex_vvvv;
			instr.operands[ 1 ].size = instr.vex_l ? operand_size::yword : operand_size::oword;

			if ( rm_operand.type == operand_type::reg )
			{
				instr.operands[ 2 ].type = operand_type::xmm;
				instr.operands[ 2 ].xmm = rm_operand.reg;
				instr.operands[ 2 ].size = instr.vex_l ? operand_size::yword : operand_size::oword;
			}
			else
			{
				instr.operands[ 2 ] = rm_operand;
				instr.operands[ 2 ].size = instr.vex_l ? operand_size::yword : operand_size::oword;
			}

			instr.operand_count = 3;
			instr.length = static_cast< std::uint8_t >( offset );
			return true;
		}

		if ( instr.opcode == 0x74 || instr.opcode == 0x75 || instr.opcode == 0x76 )
		{
			if ( instr.vex_pp != 1 )
			{
				return false;
			}

			instruction_type type;
			switch ( instr.opcode )
			{
				case 0x74: type = instruction_type::vpcmpeqb; break;
				case 0x75: type = instruction_type::vpcmpeqw; break;
				case 0x76: type = instruction_type::vpcmpeqd; break;
				default:   return false;
			}

			operand rm_operand;
			if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
			{
				return false;
			}

			const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

			instr.type = type;

			instr.operands[ 0 ].type = operand_type::xmm;
			instr.operands[ 0 ].xmm = this->get_register ( reg, instr.vex_r );
			instr.operands[ 0 ].size = instr.vex_l ? operand_size::yword : operand_size::oword;

			instr.operands[ 1 ].type = operand_type::xmm;
			instr.operands[ 1 ].xmm = instr.vex_vvvv;
			instr.operands[ 1 ].size = instr.vex_l ? operand_size::yword : operand_size::oword;

			if ( rm_operand.type == operand_type::reg )
			{
				instr.operands[ 2 ].type = operand_type::xmm;
				instr.operands[ 2 ].xmm = rm_operand.reg;
				instr.operands[ 2 ].size = instr.vex_l ? operand_size::yword : operand_size::oword;
			}
			else
			{
				instr.operands[ 2 ] = rm_operand;
				instr.operands[ 2 ].size = instr.vex_l ? operand_size::yword : operand_size::oword;
			}

			instr.operand_count = 3;
			instr.length = static_cast< std::uint8_t >( offset );
			return true;
		}

		if ( instr.opcode == 0xD7 )
		{
			if ( instr.vex_pp != 1 )
			{
				return false;
			}

			operand rm_operand;
			if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
			{
				return false;
			}

			const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

			instr.type = instruction_type::vpmovmskb;

			instr.operands[ 0 ].type = operand_type::reg;
			instr.operands[ 0 ].reg = this->get_register ( reg, instr.vex_r );
			instr.operands[ 0 ].size = operand_size::dword;

			instr.operands[ 1 ].type = operand_type::xmm;
			instr.operands[ 1 ].xmm = rm_operand.reg;
			instr.operands[ 1 ].size = instr.vex_l ? operand_size::yword : operand_size::oword;

			instr.operand_count = 2;
			instr.length = static_cast< std::uint8_t >( offset );
			return true;
		}

		if ( instr.opcode == 0x77 )
		{
			instr.type = instruction_type::vzeroupper;
			instr.operand_count = 0;
			instr.length = static_cast< std::uint8_t > ( offset );
			return true;
		}

		if ( instr.opcode == 0x6F || instr.opcode == 0x7F )
		{
			if ( instr.vex_pp == 1 )
			{
				instr.type = instr.vex_l ? instruction_type::vmovdqa256 : instruction_type::movdqa;
			}
			else if ( instr.vex_pp == 2 )
			{
				instr.type = instr.vex_l ? instruction_type::vmovdqu256 : instruction_type::movdqu;
			}
			else
			{
				return false;
			}

			operand rm_operand;
			if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
			{
				return false;
			}

			const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;
			const auto xmm_reg = this->get_register ( reg, instr.vex_r );

			if ( instr.opcode == 0x6F )
			{
				instr.operands[ 0 ].type = operand_type::xmm;
				instr.operands[ 0 ].xmm = xmm_reg;
				instr.operands[ 0 ].size = operand_size::oword;

				if ( rm_operand.type == operand_type::reg )
				{
					instr.operands[ 1 ].type = operand_type::xmm;
					instr.operands[ 1 ].xmm = rm_operand.reg;
					instr.operands[ 1 ].size = operand_size::oword;
				}
				else
				{
					instr.operands[ 1 ] = rm_operand;
					instr.operands[ 1 ].size = operand_size::oword;
				}
			}
			else
			{
				if ( rm_operand.type == operand_type::reg )
				{
					instr.operands[ 0 ].type = operand_type::xmm;
					instr.operands[ 0 ].xmm = rm_operand.reg;
					instr.operands[ 0 ].size = operand_size::oword;
				}
				else
				{
					instr.operands[ 0 ] = rm_operand;
					instr.operands[ 0 ].size = operand_size::oword;
				}

				instr.operands[ 1 ].type = operand_type::xmm;
				instr.operands[ 1 ].xmm = xmm_reg;
				instr.operands[ 1 ].size = operand_size::oword;
			}

			instr.operand_count = 2;
			instr.length = static_cast< std::uint8_t > ( offset );
			return true;
		}

		if ( instr.opcode == 0x28 || instr.opcode == 0x29 )
		{
			if ( instr.vex_pp == 0 )
			{
				instr.type = instr.vex_l ? instruction_type::vmovaps256 : instruction_type::movaps;
			}
			else if ( instr.vex_pp == 1 )
			{
				instr.type = instr.vex_l ? instruction_type::vmovapd256 : instruction_type::movapd;
			}
			else
			{
				return false;
			}

			operand rm_operand;
			if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
			{
				return false;
			}

			const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;
			const auto xmm_reg = this->get_register ( reg, instr.vex_r );

			if ( instr.opcode == 0x28 )
			{
				instr.operands[ 0 ].type = operand_type::xmm;
				instr.operands[ 0 ].xmm = xmm_reg;
				instr.operands[ 0 ].size = operand_size::oword;

				if ( rm_operand.type == operand_type::reg )
				{
					instr.operands[ 1 ].type = operand_type::xmm;
					instr.operands[ 1 ].xmm = rm_operand.reg;
					instr.operands[ 1 ].size = operand_size::oword;
				}
				else
				{
					instr.operands[ 1 ] = rm_operand;
					instr.operands[ 1 ].size = operand_size::oword;
				}
			}
			else
			{
				if ( rm_operand.type == operand_type::reg )
				{
					instr.operands[ 0 ].type = operand_type::xmm;
					instr.operands[ 0 ].xmm = rm_operand.reg;
					instr.operands[ 0 ].size = operand_size::oword;
				}
				else
				{
					instr.operands[ 0 ] = rm_operand;
					instr.operands[ 0 ].size = operand_size::oword;
				}

				instr.operands[ 1 ].type = operand_type::xmm;
				instr.operands[ 1 ].xmm = xmm_reg;
				instr.operands[ 1 ].size = operand_size::oword;
			}

			instr.operand_count = 2;
			instr.length = static_cast< std::uint8_t > ( offset );
			return true;
		}

		if ( instr.opcode == 0x10 || instr.opcode == 0x11 )
		{
			if ( instr.vex_pp == 0 )
			{
				instr.type = instr.vex_l ? instruction_type::vmovups256 : instruction_type::movups;
			}
			else if ( instr.vex_pp == 1 )
			{
				instr.type = instr.vex_l ? instruction_type::vmovupd256 : instruction_type::movupd;
			}
			else
			{
				return false;
			}

			operand rm_operand;
			if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
			{
				return false;
			}

			const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;
			const auto xmm_reg = this->get_register ( reg, instr.vex_r );

			if ( instr.opcode == 0x10 )
			{
				instr.operands[ 0 ].type = operand_type::xmm;
				instr.operands[ 0 ].xmm = xmm_reg;
				instr.operands[ 0 ].size = operand_size::oword;

				if ( rm_operand.type == operand_type::reg )
				{
					instr.operands[ 1 ].type = operand_type::xmm;
					instr.operands[ 1 ].xmm = rm_operand.reg;
					instr.operands[ 1 ].size = operand_size::oword;
				}
				else
				{
					instr.operands[ 1 ] = rm_operand;
					instr.operands[ 1 ].size = operand_size::oword;
				}
			}
			else
			{
				if ( rm_operand.type == operand_type::reg )
				{
					instr.operands[ 0 ].type = operand_type::xmm;
					instr.operands[ 0 ].xmm = rm_operand.reg;
					instr.operands[ 0 ].size = operand_size::oword;
				}
				else
				{
					instr.operands[ 0 ] = rm_operand;
					instr.operands[ 0 ].size = operand_size::oword;
				}

				instr.operands[ 1 ].type = operand_type::xmm;
				instr.operands[ 1 ].xmm = xmm_reg;
				instr.operands[ 1 ].size = operand_size::oword;
			}

			instr.operand_count = 2;
			instr.length = static_cast< std::uint8_t > ( offset );
			return true;
		}

		if ( instr.opcode == 0x7E )
		{
			operand rm_operand;
			if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
			{
				return false;
			}

			const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;
			const auto xmm_reg = this->get_register ( reg, instr.vex_r );

			if ( instr.vex_pp == 1 )
			{
				instr.type = instruction_type::movd;

				instr.operands[ 0 ].type = operand_type::xmm;
				instr.operands[ 0 ].xmm = xmm_reg;
				instr.operands[ 0 ].size = operand_size::oword;

				if ( rm_operand.type == operand_type::reg )
				{
					instr.operands[ 1 ].type = operand_type::reg;
					instr.operands[ 1 ].reg = rm_operand.reg;
					instr.operands[ 1 ].size = instr.vex_w ? operand_size::qword : operand_size::dword;
				}
				else
				{
					instr.operands[ 1 ] = rm_operand;
					instr.operands[ 1 ].size = instr.vex_w ? operand_size::qword : operand_size::dword;
				}
			}
			else if ( instr.vex_pp == 2 )
			{
				instr.type = instruction_type::movq;

				instr.operands[ 0 ].type = operand_type::xmm;
				instr.operands[ 0 ].xmm = xmm_reg;
				instr.operands[ 0 ].size = operand_size::oword;

				if ( rm_operand.type == operand_type::reg )
				{
					instr.operands[ 1 ].type = operand_type::xmm;
					instr.operands[ 1 ].xmm = rm_operand.reg;
					instr.operands[ 1 ].size = operand_size::qword;
				}
				else
				{
					instr.operands[ 1 ] = rm_operand;
					instr.operands[ 1 ].size = operand_size::qword;
				}
			}
			else
			{
				return false;
			}

			instr.operand_count = 2;
			instr.length = static_cast< std::uint8_t >( offset );
			return true;
		}

		if ( instr.opcode == 0x5A )
		{
			if ( instr.vex_pp == 2 )
			{
				instr.type = instruction_type::vcvtss2sd;
			}
			else if ( instr.vex_pp == 3 )
			{
				instr.type = instruction_type::vcvtsd2ss;
			}
			else
			{
				return false;
			}

			operand rm_operand;
			if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
			{
				return false;
			}

			const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;
			const auto xmm_reg = this->get_register ( reg, instr.vex_r );

			instr.operands[ 0 ].type = operand_type::xmm;
			instr.operands[ 0 ].xmm = xmm_reg;
			instr.operands[ 0 ].size = operand_size::oword;

			instr.operands[ 1 ].type = operand_type::xmm;
			instr.operands[ 1 ].xmm = instr.vex_vvvv;
			instr.operands[ 1 ].size = operand_size::oword;

			if ( rm_operand.type == operand_type::reg )
			{
				instr.operands[ 2 ].type = operand_type::xmm;
				instr.operands[ 2 ].xmm = rm_operand.reg;
				instr.operands[ 2 ].size = instr.vex_pp == 3 ? operand_size::qword : operand_size::dword;
			}
			else
			{
				instr.operands[ 2 ] = rm_operand;
				instr.operands[ 2 ].size = instr.vex_pp == 3 ? operand_size::qword : operand_size::dword;
			}

			instr.operand_count = 3;
			instr.length = static_cast< std::uint8_t > ( offset );
			return true;
		}

		if ( instr.opcode == 0xE7 )
		{
			instr.type = instr.vex_l ? instruction_type::vmovdqu256 : instruction_type::movdqu;

			operand rm_operand;
			if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
			{
				return false;
			}

			const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

			instr.operands[ 0 ] = rm_operand;
			instr.operands[ 0 ].size = operand_size::oword;

			instr.operands[ 1 ].type = operand_type::xmm;
			instr.operands[ 1 ].xmm = this->get_register ( reg, instr.vex_r );
			instr.operands[ 1 ].size = operand_size::oword;

			instr.operand_count = 2;
			instr.length = static_cast< std::uint8_t > ( offset );
			return true;
		}
	}

	if ( instr.vex_map == 2 )
	{
		if ( instr.opcode == 0x00 && instr.vex_pp == 1 )
		{
			operand rm_operand;

			if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
			{
				return false;
			}

			const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;
			const auto width = instr.vex_l ? operand_size::yword : operand_size::oword;

			instr.type = instruction_type::vpshufb;

			instr.operands[ 0 ].type = operand_type::xmm;
			instr.operands[ 0 ].xmm = this->get_register ( reg, instr.vex_r );
			instr.operands[ 0 ].size = width;

			instr.operands[ 1 ].type = operand_type::xmm;
			instr.operands[ 1 ].xmm = instr.vex_vvvv;
			instr.operands[ 1 ].size = width;

			if ( rm_operand.type == operand_type::reg )
			{
				instr.operands[ 2 ].type = operand_type::xmm;
				instr.operands[ 2 ].xmm = rm_operand.reg;
				instr.operands[ 2 ].size = width;
			}
			else
			{
				instr.operands[ 2 ] = rm_operand;
				instr.operands[ 2 ].size = width;
			}

			instr.operand_count = 3;
			instr.length = static_cast< std::uint8_t >( offset );
			return true;
		}

		return false;
	}

	if ( instr.vex_map == 3 )
	{
		if ( instr.opcode == 0x18 )
		{
			instr.type = instruction_type::vinsertf128;

			operand rm_operand;
			if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
			{
				return false;
			}

			if ( offset >= max_length )
			{
				return false;
			}

			const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

			instr.operands[ 0 ].type = operand_type::xmm;
			instr.operands[ 0 ].xmm = this->get_register ( reg, instr.vex_r );
			instr.operands[ 0 ].size = operand_size::oword;

			if ( rm_operand.type == operand_type::reg )
			{
				instr.operands[ 1 ].type = operand_type::xmm;
				instr.operands[ 1 ].xmm = rm_operand.reg;
				instr.operands[ 1 ].size = operand_size::oword;
			}
			else
			{
				instr.operands[ 1 ] = rm_operand;
				instr.operands[ 1 ].size = operand_size::oword;
			}

			instr.operands[ 2 ].type = operand_type::imm;
			instr.operands[ 2 ].imm = bytes[ offset++ ];
			instr.operands[ 2 ].size = operand_size::byte;

			instr.operand_count = 3;
			instr.length = static_cast< std::uint8_t > ( offset );
			return true;
		}
	}

	return false;
}


inline auto decoder::decode ( const std::uint8_t* bytes, std::size_t max_length, instruction& instr ) -> bool
{
	if ( !bytes || max_length < 2 )
	{
		return false;
	}

	std::size_t offset = 0;
	instr = instruction ( );

	this->parse_prefixes ( bytes, offset, instr );

	if ( offset >= max_length )
	{
		return false;
	}
	if ( instr.has_vex )
	{
		return this->decode_vex ( bytes, offset, max_length, instr );
	}

	instr.opcode = bytes[ offset++ ];

	if ( offset >= max_length )
	{
		return false;
	}

	switch ( instr.opcode )
	{
		case 0x04:
		case 0x05:
		{
			instr.type = instruction_type::add;

			instr.operands[ 0 ].type = operand_type::reg;
			instr.operands[ 0 ].reg = 0;

			instr.operands[ 1 ].type = operand_type::imm;

			if ( instr.opcode == 0x04 )
			{
				instr.operands[ 0 ].size = operand_size::byte;
				instr.operands[ 1 ].imm = bytes[ offset++ ];
				instr.operands[ 1 ].size = operand_size::byte;
			}
			else
			{
				instr.operands[ 0 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;
				std::uint32_t imm32;
				std::memcpy ( &imm32, &bytes[ offset ], 4 );
				offset += 4;
				instr.operands[ 1 ].imm = imm32;
				instr.operands[ 1 ].size = operand_size::dword;
			}

			instr.operand_count = 2;
			instr.length = static_cast< std::uint8_t >( offset );
			return true;
		}

		case 0x14:
		case 0x15:
		{
			instr.type = instruction_type::adc;

			instr.operands[ 0 ].type = operand_type::reg;
			instr.operands[ 0 ].reg = 0;

			instr.operands[ 1 ].type = operand_type::imm;

			if ( instr.opcode == 0x14 )
			{
				instr.operands[ 0 ].size = operand_size::byte;
				instr.operands[ 1 ].imm = bytes[ offset++ ];
				instr.operands[ 1 ].size = operand_size::byte;
			}
			else
			{
				instr.operands[ 0 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;
				std::int32_t imm32 = 0;
				std::memcpy ( &imm32, &bytes[ offset ], 4 );
				offset += 4;
				instr.operands[ 1 ].imm = static_cast< std::int64_t > ( imm32 );
				instr.operands[ 1 ].size = operand_size::dword;
			}

			instr.operand_count = 2;
			instr.length = static_cast< std::uint8_t >( offset );
			return true;
		}

		case 0x0C:
		case 0x0D:
		{
			instr.type = instruction_type::or_instr;

			instr.operands[ 0 ].type = operand_type::reg;
			instr.operands[ 0 ].reg = 0;

			instr.operands[ 1 ].type = operand_type::imm;

			if ( instr.opcode == 0x0C )
			{
				instr.operands[ 0 ].size = operand_size::byte;
				instr.operands[ 1 ].imm = bytes[ offset++ ];
				instr.operands[ 1 ].size = operand_size::byte;
			}
			else
			{
				instr.operands[ 0 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;
				std::uint32_t imm32;
				std::memcpy ( &imm32, &bytes[ offset ], 4 );
				offset += 4;
				instr.operands[ 1 ].imm = imm32;
				instr.operands[ 1 ].size = operand_size::dword;
			}

			instr.operand_count = 2;
			instr.length = static_cast< std::uint8_t >( offset );
			return true;
		}

		case 0x2C:
		case 0x2D:
		{
			instr.type = instruction_type::sub;

			instr.operands[ 0 ].type = operand_type::reg;
			instr.operands[ 0 ].reg = 0;

			instr.operands[ 1 ].type = operand_type::imm;

			if ( instr.opcode == 0x2C )
			{
				instr.operands[ 0 ].size = operand_size::byte;
				instr.operands[ 1 ].imm = bytes[ offset++ ];
				instr.operands[ 1 ].size = operand_size::byte;
			}
			else
			{
				instr.operands[ 0 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;
				std::uint32_t imm32;
				std::memcpy ( &imm32, &bytes[ offset ], 4 );
				offset += 4;
				instr.operands[ 1 ].imm = imm32;
				instr.operands[ 1 ].size = operand_size::dword;
			}

			instr.operand_count = 2;
			instr.length = static_cast< std::uint8_t >( offset );
			return true;
		}

		case 0x34:
		case 0x35:
		{
			instr.type = instruction_type::xor_instr;

			instr.operands[ 0 ].type = operand_type::reg;
			instr.operands[ 0 ].reg = 0;

			instr.operands[ 1 ].type = operand_type::imm;

			if ( instr.opcode == 0x34 )
			{
				instr.operands[ 0 ].size = operand_size::byte;
				instr.operands[ 1 ].imm = bytes[ offset++ ];
				instr.operands[ 1 ].size = operand_size::byte;
			}
			else
			{
				instr.operands[ 0 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;
				std::uint32_t imm32;
				std::memcpy ( &imm32, &bytes[ offset ], 4 );
				offset += 4;
				instr.operands[ 1 ].imm = imm32;
				instr.operands[ 1 ].size = operand_size::dword;
			}

			instr.operand_count = 2;
			instr.length = static_cast< std::uint8_t >( offset );
			return true;
		}

		case 0x3D:
		{
			instr.type = instruction_type::cmp;

			instr.operands[ 0 ].type = operand_type::reg;
			instr.operands[ 0 ].reg = 0;
			instr.operands[ 0 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;

			instr.operands[ 1 ].type = operand_type::imm;
			std::uint32_t imm32;
			std::memcpy ( &imm32, &bytes[ offset ], 4 );
			offset += 4;
			instr.operands[ 1 ].imm = imm32;
			instr.operands[ 1 ].size = operand_size::dword;

			instr.operand_count = 2;
			instr.length = static_cast< std::uint8_t >( offset );
			return true;
		}

		case 0x24:
		case 0x25:
		{
			instr.type = instruction_type::and_instr;

			instr.operands[ 0 ].type = operand_type::reg;
			instr.operands[ 0 ].reg = 0;

			instr.operands[ 1 ].type = operand_type::imm;

			if ( instr.opcode == 0x24 )
			{
				instr.operands[ 0 ].size = operand_size::byte;
				instr.operands[ 1 ].imm = bytes[ offset++ ];
				instr.operands[ 1 ].size = operand_size::byte;
			}
			else
			{
				instr.operands[ 0 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;
				std::uint32_t imm32;
				std::memcpy ( &imm32, &bytes[ offset ], 4 );
				offset += 4;
				instr.operands[ 1 ].imm = imm32;
				instr.operands[ 1 ].size = operand_size::dword;
			}

			instr.operand_count = 2;
			instr.length = static_cast< std::uint8_t >( offset );
			return true;
		}

		case 0x0F:
		{
			return this->decode_two_byte ( bytes, offset, max_length, instr );
		}

		case 0x88: case 0x89: case 0x8A: case 0x8B:
		case 0xB0: case 0xB1: case 0xB2: case 0xB3:
		case 0xB4: case 0xB5: case 0xB6: case 0xB7:
		case 0xB8: case 0xB9: case 0xBA: case 0xBB:
		case 0xBC: case 0xBD: case 0xBE: case 0xBF:
		case 0xC6: case 0xC7:
		{
			return this->decode_mov ( bytes, offset, max_length, instr );
		}

		case 0x8D:
		{
			return this->decode_lea ( bytes, offset, max_length, instr );
		}

		case 0x90:
		{
			instr.type = instruction_type::nop;
			instr.operand_count = 0;
			instr.length = static_cast< std::uint8_t >( offset );
			return true;
		}

		case 0x50: case 0x51: case 0x52: case 0x53:
		case 0x54: case 0x55: case 0x56: case 0x57:
		case 0x58: case 0x59: case 0x5A: case 0x5B:
		case 0x5C: case 0x5D: case 0x5E: case 0x5F:
		{
			return this->decode_push_pop ( bytes, offset, instr );
		}

		case 0x63:
		{
			instr.type = instruction_type::movsx;

			operand rm_operand;
			if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
			{
				return false;
			}

			const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;
			instr.operands[ 0 ].type = operand_type::reg;
			instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
			instr.operands[ 0 ].size = operand_size::qword;

			instr.operands[ 1 ] = rm_operand;
			instr.operands[ 1 ].size = operand_size::dword;

			instr.operand_count = 2;
			instr.length = static_cast< std::uint8_t >( offset );
			return true;
		}

		case 0x86:
		case 0x87:
		{
			instr.type = instruction_type::xchg;

			operand rm_operand;
			if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
			{
				return false;
			}

			const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

			instr.operands[ 0 ] = rm_operand;
			instr.operands[ 0 ].size = ( instr.opcode == 0x86 ) ? operand_size::byte :
				( instr.rex_w ? operand_size::qword : operand_size::dword );

			instr.operands[ 1 ].type = operand_type::reg;
			instr.operands[ 1 ].reg = this->get_register ( reg, instr.rex_r );
			instr.operands[ 1 ].size = instr.operands[ 0 ].size;

			instr.operand_count = 2;
			instr.length = static_cast< std::uint8_t >( offset );
			return true;
		}

		case 0x00: case 0x01: case 0x02: case 0x03:
		case 0x08: case 0x0A:
		case 0x09: case 0x0B:
		case 0x18: case 0x19: case 0x1A: case 0x1B:
		case 0x20: case 0x21: case 0x22: case 0x23:
		case 0x28: case 0x29: case 0x2A: case 0x2B:
		case 0x30: case 0x31: case 0x32: case 0x33:
		case 0x38:
		case 0x3A: case 0x39: case 0x3B:
		case 0x3C:
		case 0x80:
		case 0x81: case 0x83:
		case 0x84: case 0x85:
		case 0xC0: case 0xC1: case 0xD1:
		case 0xD0: case 0xD2: case 0xD3:
		case 0xF6: case 0xF7:
		case 0xFE:
		{
			return this->decode_arithmetic ( bytes, offset, max_length, instr );
		}

		case 0xD9:
		{
			operand rm_operand;
			if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
			{
				return false;
			}

			const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;

			if ( reg == 7 )
			{
				instr.type = instruction_type::fstcw;
				instr.operands[ 0 ] = rm_operand;
				instr.operands[ 0 ].size = operand_size::word;
				instr.operand_count = 1;
				instr.length = static_cast< std::uint8_t >( offset );
				return true;
			}

			return false;
		}

		case 0x8C:
		{
			operand rm_operand;
			if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
			{
				return false;
			}

			instr.type = instruction_type::nop;
			instr.operand_count = 0;
			instr.length = static_cast< std::uint8_t >( offset );
			return true;
		}

		case 0x9C:
		{
			instr.type = instruction_type::pushfq;
			instr.operand_count = 0;
			instr.length = static_cast< std::uint8_t >( offset );
			return true;
		}

		case 0xA8:
		{
			instr.type = instruction_type::test;
			instr.operand_count = 2;

			instr.operands[ 0 ].type = operand_type::reg;
			instr.operands[ 0 ].reg = 0;
			instr.operands[ 0 ].size = operand_size::byte;

			instr.operands[ 1 ].type = operand_type::imm;
			instr.operands[ 1 ].imm = bytes[ offset ];
			instr.operands[ 1 ].size = operand_size::byte;
			offset += 1;

			instr.length = static_cast< std::uint8_t > ( offset );
			return true;
		}

		case 0xA9:
		{
			instr.type = instruction_type::test;
			instr.operand_count = 2;

			instr.operands[ 0 ].type = operand_type::reg;
			instr.operands[ 0 ].reg = 0;
			instr.operands[ 0 ].size = instr.rex_w ? operand_size::qword : ( instr.has_operand_size ? operand_size::word : operand_size::dword );

			std::int32_t imm32 = 0;
			std::memcpy ( &imm32, &bytes[ offset ], 4 );
			offset += 4;

			instr.operands[ 1 ].type = operand_type::imm;
			instr.operands[ 1 ].imm = static_cast< std::uint64_t > ( static_cast< std::int64_t > ( imm32 ) );
			instr.operands[ 1 ].size = instr.operands[ 0 ].size;

			instr.length = static_cast< std::uint8_t > ( offset );
			return true;
		}

		case 0x69:
		case 0x6B:
		{
			instr.type = instruction_type::imul;

			operand rm_operand;
			if ( !this->parse_modrm ( bytes, offset, max_length, instr, rm_operand ) )
			{
				return false;
			}

			const auto reg = ( instr.modrm_byte >> 3 ) & 0x07;
			instr.operands[ 0 ].type = operand_type::reg;
			instr.operands[ 0 ].reg = this->get_register ( reg, instr.rex_r );
			instr.operands[ 0 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;

			instr.operands[ 1 ] = rm_operand;
			instr.operands[ 1 ].size = instr.rex_w ? operand_size::qword : operand_size::dword;

			instr.operands[ 2 ].type = operand_type::imm;
			if ( instr.opcode == 0x6B )
			{
				std::int8_t imm8 = static_cast< std::int8_t >( bytes[ offset++ ] );
				instr.operands[ 2 ].imm = static_cast< std::int64_t >( imm8 );
			}
			else
			{
				std::int32_t imm32 = 0;
				std::memcpy ( &imm32, &bytes[ offset ], 4 );
				offset += 4;
				instr.operands[ 2 ].imm = static_cast< std::int64_t >( imm32 );
			}

			instr.operand_count = 3;
			instr.length = static_cast< std::uint8_t >( offset );
			return true;
		}

		case 0x98:
		{
			if ( instr.rex_w )
			{
				instr.type = instruction_type::cdqe;
			}
			else
			{
				instr.type = instruction_type::cwde;
			}

			instr.operand_count = 0;
			instr.length = static_cast< std::uint8_t >( offset );
			return true;
		}

		case 0x99:
		{
			if ( instr.rex_w )
			{
				instr.type = instruction_type::cqo;
			}
			else
			{
				instr.type = instruction_type::cdq;
			}

			instr.operand_count = 0;
			instr.length = static_cast< std::uint8_t >( offset );
			return true;
		}

		case 0xC2:
		case 0xC3:
		{
			return this->decode_ret ( bytes, offset, instr );
		}

		case 0x70: case 0x71: case 0x72: case 0x73:
		case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7A: case 0x7B:
		case 0x7C: case 0x7D: case 0x7E: case 0x7F:
		{
			return this->decode_jcc ( bytes, offset, instr );
		}

		case 0xE8:
		{
			return this->decode_call ( bytes, offset, max_length, instr );
		}

		case 0xE9:
		case 0xEB:
		{
			return this->decode_jmp ( bytes, offset, max_length, instr );
		}

		case 0xFF:
		{
			if ( offset >= max_length )
			{
				return false;
			}

			const auto modrm = bytes[ offset ];
			const auto reg_field = ( modrm >> 3 ) & 0x07;

			if ( reg_field == 0 || reg_field == 1 )
			{
				return this->decode_arithmetic ( bytes, offset, max_length, instr );
			}

			else if ( reg_field == 2 )
			{
				return this->decode_call ( bytes, offset, max_length, instr );
			}

			else if ( reg_field == 4 )
			{
				return this->decode_jmp ( bytes, offset, max_length, instr );
			}
			else
			{
				return false;
			}
		}

		default:
		{
			instr.length = static_cast< std::uint8_t >( offset );
			return false;
		}
	}
}