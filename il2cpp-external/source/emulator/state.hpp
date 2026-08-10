#pragma once
#include <Windows.h>
#include <cstdio>
#include <array>
#include "types.hpp"

class cpu_state
{
public:
	cpu_state ( );
	~cpu_state ( ) = default;

	std::array< std::uint64_t, 16 > gpr;
	std::uint64_t rip;
	std::uint64_t fs_base;
	std::uint64_t gs_base;
	std::array< std::array< std::uint8_t, 16 >, 16 > xmm;
	std::array< std::array< std::uint8_t, 32 >, 16 > ymm;
	std::array< std::array< std::uint8_t, 16 >, 16 > ymm_hi;
	std::uint64_t rflags;

	instruction instr;

	static constexpr std::uint8_t flag_cf = 0;
	static constexpr std::uint8_t flag_pf = 2;
	static constexpr std::uint8_t flag_af = 4;
	static constexpr std::uint8_t flag_zf = 6;
	static constexpr std::uint8_t flag_sf = 7;
	static constexpr std::uint8_t flag_of = 11;

	static constexpr std::uint64_t cl_sentinel = 0xFFFFFFFFFFFFFFFF;

	auto read_ymm_hi ( std::uint8_t index ) const -> const std::uint8_t*;
	auto write_ymm_hi ( std::uint8_t index, const std::uint8_t* data ) -> void;
	auto zero_ymm_hi ( std::uint8_t index ) -> void;

	auto get_flag ( std::uint8_t bit ) const -> bool;
	auto set_flag ( std::uint8_t bit, bool value ) -> void;
	auto update_flags_zsp ( std::uint64_t result, operand_size size ) -> void;
	auto update_flags_logic ( std::uint64_t result, operand_size size ) -> void;
	auto read_gpr ( std::uint8_t index ) const -> std::uint64_t;
	auto write_gpr ( std::uint8_t index, std::uint64_t value ) -> void;
	auto read_xmm ( std::uint8_t index ) const -> const std::uint8_t*;
	auto write_xmm ( std::uint8_t index, const std::uint8_t* data ) -> void;
	auto calculate_memory_address ( const memory_operand& mem, std::uint64_t next_rip ) const -> std::uint64_t;
	auto reset ( ) -> void;
};

inline cpu_state::cpu_state ( )
	: rip ( 0 ), fs_base ( 0 ), gs_base ( 0 ), rflags ( 0x202 )
{
	this->gpr.fill ( 0 );

	for ( auto& xmm_reg : this->xmm )
	{
		xmm_reg.fill ( 0 );
	}
}

inline auto cpu_state::get_flag ( std::uint8_t bit ) const -> bool
{
	return ( this->rflags >> bit ) & 1;
}

inline auto cpu_state::set_flag ( std::uint8_t bit, bool value ) -> void
{
	if ( value )
	{
		this->rflags |= ( 1ULL << bit );
	}
	else
	{
		this->rflags &= ~( 1ULL << bit );
	}
}

inline auto cpu_state::update_flags_zsp ( std::uint64_t result, operand_size size ) -> void
{
	std::uint64_t masked_result = result;

	if ( size == operand_size::byte )
	{
		masked_result &= 0xFF;
	}
	else if ( size == operand_size::word )
	{
		masked_result &= 0xFFFF;
	}
	else if ( size == operand_size::dword )
	{
		masked_result &= 0xFFFFFFFF;
	}

	this->set_flag ( this->flag_zf, masked_result == 0 );

	bool sign_bit = false;

	if ( size == operand_size::byte )
	{
		sign_bit = ( masked_result & 0x80 ) != 0;
	}
	else if ( size == operand_size::word )
	{
		sign_bit = ( masked_result & 0x8000 ) != 0;
	}
	else if ( size == operand_size::dword )
	{
		sign_bit = ( masked_result & 0x80000000 ) != 0;
	}
	else
	{
		sign_bit = ( masked_result & 0x8000000000000000ULL ) != 0;
	}

	this->set_flag ( this->flag_sf, sign_bit );

	std::uint8_t low_byte = static_cast< std::uint8_t > ( masked_result & 0xFF );
	std::uint8_t parity = 0;

	for ( int i = 0; i < 8; ++i )
	{
		parity ^= ( low_byte >> i ) & 1;
	}

	this->set_flag ( this->flag_pf, parity == 0 );
}

inline auto cpu_state::update_flags_logic ( std::uint64_t result, operand_size size ) -> void
{
	this->set_flag ( this->flag_cf, false );
	this->set_flag ( this->flag_of, false );
	this->update_flags_zsp ( result, size );
}

inline auto cpu_state::read_gpr ( std::uint8_t index ) const -> std::uint64_t
{
	if ( index >= 16 )
	{
		return 0;
	}

	return this->gpr[ index ];
}

inline auto cpu_state::write_gpr ( std::uint8_t index, std::uint64_t value ) -> void
{
	if ( index >= 16 )
	{
		return;
	}

	this->gpr[ index ] = value;
}

inline auto cpu_state::read_xmm ( std::uint8_t index ) const -> const std::uint8_t*
{
	if ( index >= 16 )
	{
		return nullptr;
	}

	return this->xmm[ index ].data ( );
}

inline auto cpu_state::write_xmm ( std::uint8_t index, const std::uint8_t* data ) -> void
{
	if ( index >= 16 || !data )
	{
		return;
	}

	std::memcpy ( this->xmm[ index ].data ( ), data, 16 );
}

inline auto cpu_state::read_ymm_hi ( std::uint8_t index ) const -> const std::uint8_t*
{
	if ( index >= 16 )
	{
		return nullptr;
	}

	return this->ymm_hi[ index ].data ( );
}

inline auto cpu_state::write_ymm_hi ( std::uint8_t index, const std::uint8_t* data ) -> void
{
	if ( index >= 16 || !data )
	{
		return;
	}

	std::memcpy ( this->ymm_hi[ index ].data ( ), data, 16 );
}

inline auto cpu_state::zero_ymm_hi ( std::uint8_t index ) -> void
{
	if ( index >= 16 )
	{
		return;
	}

	this->ymm_hi[ index ].fill ( 0 );
}

inline auto cpu_state::calculate_memory_address ( const memory_operand& mem, std::uint64_t next_rip ) const -> std::uint64_t
{
	std::uint64_t address = 0;

	if ( mem.base == 16 )
	{
		address = next_rip;
	}
	else if ( mem.base != 0xFF )
	{
		address = read_gpr ( mem.base );
	}

	if ( mem.index != 0xFF )
	{
		address += read_gpr ( mem.index ) * mem.scale;
	}

	address += mem.displacement;

	return address;
}

inline auto cpu_state::reset ( ) -> void
{
	this->gpr.fill ( 0 );
	this->rip = 0;
	this->rflags = 0x202;

	for ( auto& xmm_reg : this->xmm )
	{
		xmm_reg.fill ( 0 );
	}

	for ( auto& hi : this->ymm_hi )
	{
		hi.fill ( 0 );
	}
}