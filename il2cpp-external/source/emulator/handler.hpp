#pragma once
#include <Windows.h>
#include <cstdio>
#include <array>
#include <functional>
#include <unordered_map>
#include "types.hpp"

using memory_read_fn = std::function< bool ( std::uint64_t address, void* buffer, std::size_t size ) >;
using memory_write_fn = std::function< bool ( std::uint64_t address, const void* buffer, std::size_t size ) >;

class memory_handler
{
public:
	memory_handler ( memory_read_fn read_callback, memory_write_fn write_callback );
	~memory_handler ( ) = default;

	auto read ( std::uint64_t address, void* buffer, std::size_t size ) const -> bool;

	template< typename T >
	auto read ( std::uint64_t address ) const -> T;

	auto write ( std::uint64_t address, const void* buffer, std::size_t size ) const -> bool;

	template< typename T >
	auto write ( std::uint64_t address, const T& value ) const -> bool;

	auto get_last_fault_address ( ) const -> std::uint64_t;
	auto get_last_fault_was_write ( ) const -> bool;
	auto clear_fault ( ) const -> void;

private:
	memory_read_fn m_read;
	memory_write_fn m_write;

	mutable std::uint64_t m_last_fault_address = 0;
	mutable bool m_last_fault_was_write = false;
};

inline memory_handler::memory_handler ( memory_read_fn read_callback, memory_write_fn write_callback )
	: m_read ( read_callback ), m_write ( write_callback )
{ }

inline auto memory_handler::read ( std::uint64_t address, void* buffer, std::size_t size ) const -> bool
{
	if ( !this->m_read || !this->m_read ( address, buffer, size ) )
	{
		this->m_last_fault_address = address;
		this->m_last_fault_was_write = false;
		return false;
	}

	return true;
}

template< typename T >
inline auto memory_handler::read ( std::uint64_t address ) const -> T
{
	T value{ };
	this->read ( address, &value, sizeof ( T ) );
	return value;
}

inline auto memory_handler::write ( std::uint64_t address, const void* buffer, std::size_t size ) const -> bool
{
	if ( !this->m_write || !this->m_write ( address, buffer, size ) )
	{
		this->m_last_fault_address = address;
		this->m_last_fault_was_write = true;
		return false;
	}

	return true;
}

inline auto memory_handler::get_last_fault_address ( ) const -> std::uint64_t
{
	return this->m_last_fault_address;
}

inline auto memory_handler::get_last_fault_was_write ( ) const -> bool
{
	return this->m_last_fault_was_write;
}

inline auto memory_handler::clear_fault ( ) const -> void
{
	this->m_last_fault_address = 0;
	this->m_last_fault_was_write = false;
}

template< typename T >
inline auto memory_handler::write ( std::uint64_t address, const T& value ) const -> bool
{
	return this->write ( address, &value, sizeof ( T ) );
}