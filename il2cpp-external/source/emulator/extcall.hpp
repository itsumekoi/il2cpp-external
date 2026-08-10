#pragma once
#include <Windows.h>
#include <cstdio>
#include <array>
#include <functional>
#include <mutex>
#include <string>
#include <vector>
#include <unordered_map>
#include "emulator.hpp"
#include "state.hpp"
#include "handler.hpp"

struct managed_string
{
	std::string text;

	managed_string ( std::string value ) : text ( std::move ( value ) )
	{
	}
};

struct ansi_string
{
	std::string text;

	ansi_string ( std::string value ) : text ( std::move ( value ) )
	{
	}
};

struct wide_string
{
	std::wstring text;

	wide_string ( std::wstring value ) : text ( std::move ( value ) )
	{
	}
};

class extcall
{
public:
	using hook_fn = std::function< bool ( cpu_state&, const memory_handler& ) >;

	struct module_range
	{
		std::uint64_t base;
		std::uint64_t size;
	};

	struct injected_tag
	{
	};

	static constexpr injected_tag injected{ };

	static constexpr std::uint64_t stack_base = 0x7FFF00000000;
	static constexpr std::uint64_t initial_rsp = stack_base + 0xF0000;
	static constexpr std::uint64_t return_address = 0xDEADBEEFDEADBEEF;
	static constexpr std::uint64_t string_area = initial_rsp + 0x10000;
	static constexpr std::uint64_t result_area = initial_rsp + 0x100;
	static constexpr std::uint64_t malloc_area = stack_base + 0x180000;
	static constexpr std::uint64_t malloc_area_size = 0x280000;
	static constexpr std::uint64_t teb_area = stack_base + 0x1000;
	static constexpr std::uint64_t tls_pointers_area = stack_base + 0x2000;
	static constexpr std::uint64_t tls_data_area = stack_base + 0x3000;
	static constexpr std::uint32_t tls_slot_count = 64;
	static constexpr std::uint32_t tls_slot_size = 0x40;
	static constexpr std::uint32_t default_execute_limit = 10000;

	extcall ( ) = default;
	~extcall ( ) = default;

	extcall ( const extcall& ) = delete;
	extcall ( extcall&& ) = delete;
	auto operator= ( const extcall& ) -> extcall& = delete;
	auto operator= ( extcall&& ) -> extcall& = delete;

	static auto instance ( ) -> extcall&;

	auto set_emulator ( emulator* emu ) -> void;
	auto get_emulator ( ) const -> emulator*;

	auto set_execute_limit ( std::uint32_t limit ) -> void;
	auto get_execute_limit ( ) const -> std::uint32_t;

	auto last_call_ok ( ) const -> bool;

	auto push_argument ( std::size_t size ) -> std::uint64_t;

	auto set_string_class ( std::uint64_t klass ) -> void;
	auto get_string_class ( ) const -> std::uint64_t;

	auto heap_read ( std::uint64_t address, void* buffer, std::size_t size ) const -> bool;
	auto heap_write ( std::uint64_t address, const void* buffer, std::size_t size ) -> void;
	auto heap_clear ( ) -> void;

	auto allocate ( std::size_t size ) -> std::uint64_t;
	auto release ( std::uint64_t address ) -> bool;
	auto allocation_size ( std::uint64_t address ) const -> std::size_t;

	auto tls_override ( std::uint32_t index, std::uint64_t& out_value ) const -> bool;
	auto set_tls_override ( std::uint32_t index, std::uint64_t value ) -> void;

	auto register_functions ( ) -> void;

	static auto make_returning_hook ( std::uint64_t return_value = 0 ) -> hook_fn;

	auto register_hook ( std::uint64_t address, hook_fn hook ) -> void;
	auto unregister_hook ( std::uint64_t address ) -> void;
	auto register_module ( std::uint64_t base, std::uint64_t size ) -> void;
	auto register_module ( std::uint64_t base, const memory_handler& mem ) -> void;
	auto register_module ( std::uint64_t base ) -> void;
	auto register_modules ( const std::vector< std::uint64_t >& bases ) -> void;
	auto clear_modules ( ) -> void;
	auto is_known_address ( std::uint64_t address ) const -> bool;
	auto handle_external_call ( std::uint64_t address, cpu_state& cpu, const memory_handler& mem ) -> bool;
	auto auto_stubbed ( ) const -> std::vector< std::uint64_t >;

	auto find_pattern ( std::uint64_t module_base, const char* pattern, const char* mask, const memory_handler& mem ) const -> std::uint64_t;

	template< typename T, typename... Args >
	auto call ( std::uint64_t address, Args... args ) -> T;

	template< typename T >
	auto call ( std::uint64_t address, injected_tag ) -> T;

	template< typename T, typename... Args >
	auto call ( std::uint64_t address, injected_tag, std::uint64_t this_ptr, Args... args ) -> T;

private:
	template< typename T >
	static auto make_default ( ) -> T;

	static auto return_from_hook ( cpu_state& cpu, const memory_handler& mem ) -> bool;

	auto register_win32_hooks ( ) -> void;
	auto register_heap_hooks ( ) -> void;

	static auto read_c_string ( const memory_handler& mem, std::uint64_t address, std::size_t limit ) -> std::string;
	auto is_known_address_locked ( std::uint64_t address ) const -> bool;

	auto setup_fake_teb_locked ( emulator* emu ) const -> void;
	auto setup_stack_locked ( emulator* emu ) const -> std::uint64_t;
	auto prepare_locked ( emulator* emu ) -> std::uint64_t;

	template< typename T >
	auto extract_return_locked ( emulator* emu ) const -> T;

	template< typename T >
	auto read_result_area_locked ( emulator* emu ) const -> T;

	auto reserve_string_locked ( std::size_t size ) -> std::uint64_t;
	auto write_managed_string_locked ( const std::string& text ) -> std::uint64_t;
	auto write_ansi_string_locked ( const std::string& text ) -> std::uint64_t;
	auto write_wide_string_locked ( const std::wstring& text ) -> std::uint64_t;

	template< typename Arg >
	auto to_uint64_locked ( Arg arg ) -> std::uint64_t;

	template< std::size_t Index, typename Arg >
	auto set_argument_locked ( cpu_state& cpu, memory_handler& mem, std::uint64_t rsp, Arg arg ) -> void;

	template< std::size_t Index, typename Arg, typename... Rest >
	auto set_arguments_locked ( cpu_state& cpu, memory_handler& mem, std::uint64_t rsp, Arg arg, Rest... rest ) -> void;

	mutable std::mutex m_mutex;
	mutable std::mutex m_heap_mutex;
	mutable std::mutex m_hook_mutex;

	std::unordered_map< std::uint64_t, hook_fn > m_hooks;
	std::vector< module_range > m_known_modules;
	std::vector< std::uint64_t > m_auto_stubbed;

	emulator* m_emulator = nullptr;
	std::uint32_t m_execute_limit = default_execute_limit;
	std::uint64_t m_string_offset = 0;
	std::uint64_t m_string_class = 0;
	bool m_last_call_ok = false;
	std::uint64_t m_malloc_offset = 0;
	std::unordered_map< std::uint64_t, std::uint8_t > m_heap;
	std::unordered_map< std::uint64_t, std::size_t > m_allocations;
	std::unordered_map< std::uint32_t, std::uint64_t > m_tls_overrides;
};

inline auto extcall::instance ( ) -> extcall&
{
	static extcall singleton;

	return singleton;
}

inline auto extcall::set_emulator ( emulator* emu ) -> void
{
	const std::lock_guard< std::mutex > lock ( this->m_mutex );

	this->m_emulator = emu;
}

inline auto extcall::get_emulator ( ) const -> emulator*
{
	const std::lock_guard< std::mutex > lock ( this->m_mutex );

	return this->m_emulator;
}

inline auto extcall::set_execute_limit ( std::uint32_t limit ) -> void
{
	const std::lock_guard< std::mutex > lock ( this->m_mutex );

	this->m_execute_limit = limit;
}

inline auto extcall::get_execute_limit ( ) const -> std::uint32_t
{
	const std::lock_guard< std::mutex > lock ( this->m_mutex );

	return this->m_execute_limit;
}

inline auto extcall::last_call_ok ( ) const -> bool
{
	const std::lock_guard< std::mutex > lock ( this->m_mutex );

	return this->m_last_call_ok;
}

inline auto extcall::push_argument ( std::size_t size ) -> std::uint64_t
{
	const std::lock_guard< std::mutex > lock ( this->m_mutex );

	if ( this->m_emulator == nullptr )
	{
		return 0;
	}

	const auto address = extcall::string_area + this->m_string_offset;

	this->m_string_offset += size;

	return address;
}

inline auto extcall::set_string_class ( std::uint64_t klass ) -> void
{
	const std::lock_guard< std::mutex > lock ( this->m_mutex );

	this->m_string_class = klass;
}

inline auto extcall::get_string_class ( ) const -> std::uint64_t
{
	const std::lock_guard< std::mutex > lock ( this->m_mutex );

	return this->m_string_class;
}

inline auto extcall::reserve_string_locked ( std::size_t size ) -> std::uint64_t
{
	const auto address = extcall::string_area + this->m_string_offset;

	this->m_string_offset += ( size + 15 ) & ~static_cast< std::uint64_t > ( 15 );

	return address;
}

inline auto extcall::write_managed_string_locked ( const std::string& text ) -> std::uint64_t
{
	if ( this->m_emulator == nullptr )
	{
		return 0;
	}

	const auto length = static_cast< std::uint32_t > ( text.size ( ) );
	const auto bytes = static_cast< std::size_t > ( 0x14 ) + ( static_cast< std::size_t > ( length ) + 1 ) * 2;

	std::vector< std::uint8_t > object ( bytes, 0 );

	std::memcpy ( object.data ( ), &this->m_string_class, 8 );
	std::memcpy ( object.data ( ) + 0x10, &length, 4 );

	for ( std::uint32_t index = 0; index < length; index++ )
	{
		const std::uint16_t wide = static_cast< std::uint8_t > ( text[ index ] );

		std::memcpy ( object.data ( ) + 0x14 + static_cast< std::size_t > ( index ) * 2, &wide, 2 );
	}

	const auto address = this->reserve_string_locked ( bytes );

	this->m_emulator->get_memory ( ).write ( address, object.data ( ), object.size ( ) );

	return address;
}

inline auto extcall::write_ansi_string_locked ( const std::string& text ) -> std::uint64_t
{
	if ( this->m_emulator == nullptr )
	{
		return 0;
	}

	const auto bytes = text.size ( ) + 1;
	const auto address = this->reserve_string_locked ( bytes );

	this->m_emulator->get_memory ( ).write ( address, text.c_str ( ), bytes );

	return address;
}

inline auto extcall::write_wide_string_locked ( const std::wstring& text ) -> std::uint64_t
{
	if ( this->m_emulator == nullptr )
	{
		return 0;
	}

	const auto bytes = ( text.size ( ) + 1 ) * sizeof ( wchar_t );
	const auto address = this->reserve_string_locked ( bytes );

	this->m_emulator->get_memory ( ).write ( address, text.c_str ( ), bytes );

	return address;
}

inline auto extcall::heap_read ( std::uint64_t address, void* buffer, std::size_t size ) const -> bool
{
	const std::lock_guard< std::mutex > lock ( this->m_heap_mutex );

	if ( this->m_heap.empty ( ) || buffer == nullptr )
	{
		return false;
	}

	auto* const bytes = static_cast< std::uint8_t* > ( buffer );

	for ( std::size_t index = 0; index < size; index++ )
	{
		const auto entry = this->m_heap.find ( address + index );

		if ( entry == this->m_heap.end ( ) )
		{
			return false;
		}

		bytes[ index ] = entry->second;
	}

	return true;
}

inline auto extcall::heap_write ( std::uint64_t address, const void* buffer, std::size_t size ) -> void
{
	const std::lock_guard< std::mutex > lock ( this->m_heap_mutex );

	if ( buffer == nullptr )
	{
		return;
	}

	const auto* const bytes = static_cast< const std::uint8_t* > ( buffer );

	for ( std::size_t index = 0; index < size; index++ )
	{
		this->m_heap[ address + index ] = bytes[ index ];
	}
}

inline auto extcall::heap_clear ( ) -> void
{
	const std::lock_guard< std::mutex > lock ( this->m_heap_mutex );

	this->m_heap.clear ( );
	this->m_allocations.clear ( );
	this->m_tls_overrides.clear ( );
	this->m_malloc_offset = 0;
}

inline auto extcall::allocate ( std::size_t size ) -> std::uint64_t
{
	const std::lock_guard< std::mutex > lock ( this->m_heap_mutex );

	if ( size == 0 )
	{
		size = 1;
	}

	const auto aligned = ( size + 15u ) & ~static_cast< std::size_t > ( 15u );

	if ( this->m_malloc_offset + aligned > extcall::malloc_area_size )
	{
		return 0;
	}

	const auto address = extcall::malloc_area + this->m_malloc_offset;

	this->m_malloc_offset += aligned;
	this->m_allocations[ address ] = size;

	return address;
}

inline auto extcall::release ( std::uint64_t address ) -> bool
{
	const std::lock_guard< std::mutex > lock ( this->m_heap_mutex );

	return this->m_allocations.erase ( address ) != 0;
}

inline auto extcall::allocation_size ( std::uint64_t address ) const -> std::size_t
{
	const std::lock_guard< std::mutex > lock ( this->m_heap_mutex );

	const auto entry = this->m_allocations.find ( address );

	if ( entry == this->m_allocations.end ( ) )
	{
		return 0;
	}

	return entry->second;
}

inline auto extcall::tls_override ( std::uint32_t index, std::uint64_t& out_value ) const -> bool
{
	const std::lock_guard< std::mutex > lock ( this->m_heap_mutex );

	const auto entry = this->m_tls_overrides.find ( index );

	if ( entry == this->m_tls_overrides.end ( ) )
	{
		return false;
	}

	out_value = entry->second;

	return true;
}

inline auto extcall::set_tls_override ( std::uint32_t index, std::uint64_t value ) -> void
{
	const std::lock_guard< std::mutex > lock ( this->m_heap_mutex );

	this->m_tls_overrides[ index ] = value;
}


inline auto extcall::setup_fake_teb_locked ( emulator* emu ) const -> void
{
	std::uint8_t teb[ 0x60 ] = { };

	const std::uint64_t stack_top = extcall::initial_rsp;
	const std::uint64_t stack_limit = extcall::stack_base;
	const std::uint64_t tls_pointers = extcall::tls_pointers_area;

	std::memcpy ( teb + 0x08, &stack_top, 8 );
	std::memcpy ( teb + 0x10, &stack_limit, 8 );
	std::memcpy ( teb + 0x48, &stack_top, 8 );
	std::memcpy ( teb + 0x58, &tls_pointers, 8 );

	emu->get_memory ( ).write ( extcall::teb_area, teb, sizeof ( teb ) );
	emu->get_cpu ( ).gs_base = extcall::teb_area;

	std::uint8_t zero_block[ extcall::tls_slot_size ] = { };

	for ( std::uint32_t index = 0; index < extcall::tls_slot_count; index++ )
	{
		const std::uint64_t slot_data_address = extcall::tls_data_area + ( static_cast< std::uint64_t > ( index ) * extcall::tls_slot_size );

		emu->get_memory ( ).write ( extcall::tls_pointers_area + ( index * 8ULL ), &slot_data_address, 8 );
		emu->get_memory ( ).write ( slot_data_address, zero_block, sizeof ( zero_block ) );
	}
}

inline auto extcall::setup_stack_locked ( emulator* emu ) const -> std::uint64_t
{
	const std::uint64_t rsp_at_entry = extcall::initial_rsp - 8;
	const std::uint64_t return_to = extcall::return_address;

	emu->get_memory ( ).write ( rsp_at_entry, &return_to, 8 );
	emu->get_cpu ( ).write_gpr ( 4, rsp_at_entry );

	this->setup_fake_teb_locked ( emu );

	return rsp_at_entry;
}

inline auto extcall::prepare_locked ( emulator* emu ) -> std::uint64_t
{
	this->m_string_offset = 0;

	this->heap_clear ( );

	emu->reset ( );

	return this->setup_stack_locked ( emu );
}

template< typename T >
inline auto extcall::make_default ( ) -> T
{
	if constexpr ( std::is_void_v< T > )
	{
		return;
	}
	else
	{
		return T{ };
	}
}

template< typename T >
inline auto extcall::read_result_area_locked ( emulator* emu ) const -> T
{
	if constexpr ( std::is_void_v< T > )
	{
		return;
	}
	else
	{
		T result{ };

		emu->get_memory ( ).read ( extcall::result_area, &result, sizeof ( T ) );

		return result;
	}
}

template< typename T >
inline auto extcall::extract_return_locked ( emulator* emu ) const -> T
{
	if constexpr ( std::is_void_v< T > )
	{
		return;
	}
	else if constexpr ( std::is_same_v< T, float > )
	{
		const auto xmm0 = emu->get_cpu ( ).read_xmm ( 0 );

		if ( !xmm0 )
		{
			return 0.00f;
		}

		float result;

		std::memcpy ( &result, xmm0, sizeof ( float ) );

		return result;
	}
	else if constexpr ( std::is_same_v< T, double > )
	{
		const auto xmm0 = emu->get_cpu ( ).read_xmm ( 0 );

		if ( !xmm0 )
		{
			return 0.00;
		}

		double result;

		std::memcpy ( &result, xmm0, sizeof ( double ) );

		return result;
	}
	else if constexpr ( std::is_integral_v< T > || std::is_pointer_v< T > )
	{
		const auto rax = emu->get_cpu ( ).read_gpr ( 0 );

		if constexpr ( std::is_pointer_v< T > )
		{
			return reinterpret_cast< T > ( static_cast< std::uintptr_t > ( rax ) );
		}
		else if constexpr ( sizeof ( T ) == 1 )
		{
			return static_cast< T > ( rax & 0xFF );
		}
		else if constexpr ( sizeof ( T ) == 2 )
		{
			return static_cast< T > ( rax & 0xFFFF );
		}
		else if constexpr ( sizeof ( T ) == 4 )
		{
			return static_cast< T > ( rax & 0xFFFFFFFF );
		}
		else
		{
			return static_cast< T > ( rax );
		}
	}
	else if constexpr ( sizeof ( T ) <= 8 )
	{
		const auto rax = emu->get_cpu ( ).read_gpr ( 0 );

		T result{ };

		std::memcpy ( &result, &rax, sizeof ( T ) );

		return result;
	}
	else
	{
		return T{ };
	}
}

template< typename Arg >
inline auto extcall::to_uint64_locked ( Arg arg ) -> std::uint64_t
{
	if constexpr ( std::is_same_v< Arg, managed_string > )
	{
		return this->write_managed_string_locked ( arg.text );
	}
	else if constexpr ( std::is_same_v< Arg, ansi_string > )
	{
		return this->write_ansi_string_locked ( arg.text );
	}
	else if constexpr ( std::is_same_v< Arg, wide_string > )
	{
		return this->write_wide_string_locked ( arg.text );
	}
	else if constexpr ( std::is_pointer_v< Arg > )
	{
		return static_cast< std::uint64_t > ( reinterpret_cast< std::uintptr_t > ( arg ) );
	}
	else if constexpr ( sizeof ( Arg ) <= 8 )
	{
		std::uint64_t value = 0;

		std::memcpy ( &value, &arg, sizeof ( Arg ) );

		return value;
	}
	else
	{
		const auto address = extcall::string_area + this->m_string_offset;

		this->m_string_offset += ( sizeof ( Arg ) + 7 ) & ~7;

		this->m_emulator->get_memory ( ).write ( address, &arg, sizeof ( Arg ) );

		return address;
	}
}

template< std::size_t Index, typename Arg >
inline auto extcall::set_argument_locked ( cpu_state& cpu, memory_handler& mem, std::uint64_t rsp, Arg arg ) -> void
{
	constexpr std::uint8_t reg_map[ ] = { 1, 2, 8, 9 };

	if constexpr ( Index < 4 )
	{
		if constexpr ( std::is_same_v< Arg, float > )
		{
			std::uint8_t xmm_data[ 16 ] = { };

			std::memcpy ( xmm_data, &arg, sizeof ( float ) );

			cpu.write_xmm ( Index, xmm_data );
		}
		else if constexpr ( std::is_same_v< Arg, double > )
		{
			std::uint8_t xmm_data[ 16 ] = { };

			std::memcpy ( xmm_data, &arg, sizeof ( double ) );

			cpu.write_xmm ( Index, xmm_data );
		}
		else
		{
			cpu.write_gpr ( reg_map[ Index ], this->to_uint64_locked ( arg ) );
		}
	}
	else
	{
		const std::uint64_t stack_slot = rsp + 32 + 8 * ( Index - 4 );
		const std::uint64_t value = this->to_uint64_locked ( arg );

		mem.write ( stack_slot, &value, 8 );
	}
}

template< std::size_t Index, typename Arg, typename... Rest >
inline auto extcall::set_arguments_locked ( cpu_state& cpu, memory_handler& mem, std::uint64_t rsp, Arg arg, Rest... rest ) -> void
{
	this->set_argument_locked< Index > ( cpu, mem, rsp, arg );

	if constexpr ( sizeof...( Rest ) > 0 )
	{
		this->set_arguments_locked< Index + 1 > ( cpu, mem, rsp, rest... );
	}
}

template< typename T, typename... Args >
inline auto extcall::call ( std::uint64_t address, Args... args ) -> T
{
	const std::lock_guard< std::mutex > lock ( this->m_mutex );

	auto* const emu = this->m_emulator;

	this->m_last_call_ok = false;

	if ( emu == nullptr )
	{
		return extcall::make_default< T > ( );
	}

	const auto rsp = this->prepare_locked ( emu );

	if constexpr ( std::is_void_v< T > )
	{
		if constexpr ( sizeof...( Args ) > 0 )
		{
			this->set_arguments_locked< 0 > ( emu->get_cpu ( ), emu->get_memory ( ), rsp, args... );
		}

		this->m_last_call_ok = emu->run ( address, extcall::return_address, this->m_execute_limit );

		return;
	}
	else if constexpr ( sizeof ( T ) > 8 )
	{
		emu->get_cpu ( ).write_gpr ( 1, extcall::result_area );

		if constexpr ( sizeof...( Args ) > 0 )
		{
			this->set_arguments_locked< 1 > ( emu->get_cpu ( ), emu->get_memory ( ), rsp, args... );
		}

		this->m_last_call_ok = emu->run ( address, extcall::return_address, this->m_execute_limit );

		if ( !this->m_last_call_ok )
		{
			return T{ };
		}

		return this->read_result_area_locked< T > ( emu );
	}
	else
	{
		if constexpr ( sizeof...( Args ) > 0 )
		{
			this->set_arguments_locked< 0 > ( emu->get_cpu ( ), emu->get_memory ( ), rsp, args... );
		}

		this->m_last_call_ok = emu->run ( address, extcall::return_address, this->m_execute_limit );

		if ( !this->m_last_call_ok )
		{
			return T{ };
		}

		return this->extract_return_locked< T > ( emu );
	}
}

template< typename T >
inline auto extcall::call ( std::uint64_t address, injected_tag ) -> T
{
	const std::lock_guard< std::mutex > lock ( this->m_mutex );

	auto* const emu = this->m_emulator;

	this->m_last_call_ok = false;

	if ( emu == nullptr )
	{
		return extcall::make_default< T > ( );
	}

	this->prepare_locked ( emu );

	emu->get_cpu ( ).write_gpr ( 1, extcall::result_area );

	this->m_last_call_ok = emu->run ( address, extcall::return_address, this->m_execute_limit );

	if ( !this->m_last_call_ok )
	{
		return extcall::make_default< T > ( );
	}

	return this->read_result_area_locked< T > ( emu );
}

template< typename T, typename... Args >
inline auto extcall::call ( std::uint64_t address, injected_tag, std::uint64_t this_ptr, Args... args ) -> T

{
	const std::lock_guard< std::mutex > lock ( this->m_mutex );

	auto* const emu = this->m_emulator;

	this->m_last_call_ok = false;

	if ( emu == nullptr )
	{
		return extcall::make_default< T > ( );
	}

	const auto rsp = this->prepare_locked ( emu );

	emu->get_cpu ( ).write_gpr ( 1, this_ptr );
	emu->get_cpu ( ).write_gpr ( 2, extcall::result_area );

	if constexpr ( sizeof...( Args ) > 0 )
	{
		this->set_arguments_locked< 2 > ( emu->get_cpu ( ), emu->get_memory ( ), rsp, args... );
	}

	this->m_last_call_ok = emu->run ( address, extcall::return_address, this->m_execute_limit );

	if ( !this->m_last_call_ok )
	{
		return extcall::make_default< T > ( );
	}

	return this->read_result_area_locked< T > ( emu );
}

inline extcall* const caller = &extcall::instance ( );

inline auto extcall::make_returning_hook ( std::uint64_t return_value ) -> extcall::hook_fn
{
	return [ return_value ] ( cpu_state& cpu, const memory_handler& mem ) -> bool
	{
		cpu.write_gpr ( 0, return_value );

		return extcall::return_from_hook ( cpu, mem );
	};
}

inline auto extcall::register_hook ( std::uint64_t address, extcall::hook_fn hook ) -> void
{
	const std::lock_guard< std::mutex > lock ( this->m_hook_mutex );

	this->m_hooks[ address ] = std::move ( hook );
}

inline auto extcall::unregister_hook ( std::uint64_t address ) -> void
{
	const std::lock_guard< std::mutex > lock ( this->m_hook_mutex );

	this->m_hooks.erase ( address );
}

inline auto extcall::register_module ( std::uint64_t base, std::uint64_t size ) -> void
{
	const std::lock_guard< std::mutex > lock ( this->m_hook_mutex );

	this->m_known_modules.push_back ( extcall::module_range{ base, size } );
}

inline auto extcall::register_module ( std::uint64_t base, const memory_handler& mem ) -> void
{
	std::uint32_t e_lfanew = 0;

	if ( !mem.read ( base + 0x3C, &e_lfanew, 4 ) )
	{
		return;
	}

	std::uint32_t size = 0;

	if ( !mem.read ( base + e_lfanew + 0x50, &size, 4 ) )
	{
		return;
	}

	this->register_module ( base, static_cast< std::uint64_t > ( size ) );
}

inline auto extcall::register_module ( std::uint64_t base ) -> void
{
	auto* const emu = this->get_emulator ( );

	if ( base == 0 || emu == nullptr )
	{
		return;
	}

	this->register_module ( base, emu->get_memory ( ) );
}

inline auto extcall::register_modules ( const std::vector< std::uint64_t >& bases ) -> void
{
	for ( const auto base : bases )
	{
		this->register_module ( base );
	}
}

inline auto extcall::clear_modules ( ) -> void
{
	const std::lock_guard< std::mutex > lock ( this->m_hook_mutex );

	this->m_known_modules.clear ( );
}

inline auto extcall::is_known_address_locked ( std::uint64_t address ) const -> bool
{
	for ( const auto& entry : this->m_known_modules )
	{
		if ( address >= entry.base && address < entry.base + entry.size )
		{
			return true;
		}
	}

	return false;
}

inline auto extcall::is_known_address ( std::uint64_t address ) const -> bool
{
	const std::lock_guard< std::mutex > lock ( this->m_hook_mutex );

	return this->is_known_address_locked ( address );
}

inline auto extcall::auto_stubbed ( ) const -> std::vector< std::uint64_t >
{
	const std::lock_guard< std::mutex > lock ( this->m_hook_mutex );

	return this->m_auto_stubbed;
}

inline auto extcall::handle_external_call ( std::uint64_t address, cpu_state& cpu, const memory_handler& mem ) -> bool
{
	extcall::hook_fn hook;

	{
		const std::lock_guard< std::mutex > lock ( this->m_hook_mutex );

		const auto entry = this->m_hooks.find ( address );

		if ( entry != this->m_hooks.end ( ) )
		{
			hook = entry->second;
		}
		else
		{
			if ( this->m_known_modules.empty ( ) || this->is_known_address_locked ( address ) )
			{
				return false;
			}

			this->m_auto_stubbed.push_back ( address );

			hook = extcall::make_returning_hook ( 0 );

			this->m_hooks[ address ] = hook;
		}
	}

	return hook ( cpu, mem );
}

inline auto extcall::find_pattern ( std::uint64_t module_base, const char* pattern, const char* mask, const memory_handler& mem ) const -> std::uint64_t
{
	std::uint32_t e_lfanew = 0;

	if ( !mem.read ( module_base + 0x3C, &e_lfanew, 4 ) )
	{
		return 0;
	}

	std::uint32_t size = 0;

	if ( !mem.read ( module_base + e_lfanew + 0x50, &size, 4 ) )
	{
		return 0;
	}

	const auto bytes = reinterpret_cast< const std::uint8_t* > ( pattern );
	const auto mask_length = std::strlen ( mask );

	for ( std::size_t offset = 0; offset + mask_length < size; offset++ )
	{
		auto found = true;

		for ( std::size_t index = 0; index < mask_length; index++ )
		{
			if ( mask[ index ] != 'x' )
			{
				continue;
			}

			std::uint8_t current = 0;

			if ( !mem.read ( module_base + offset + index, &current, 1 ) || current != bytes[ index ] )
			{
				found = false;
				break;
			}
		}

		if ( found )
		{
			return module_base + offset;
		}
	}

	return 0;
}

inline auto extcall::return_from_hook ( cpu_state& cpu, const memory_handler& mem ) -> bool
{
	const auto rsp = cpu.read_gpr ( 4 );

	std::uint64_t return_address = 0;

	if ( !mem.read ( rsp, &return_address, sizeof ( std::uint64_t ) ) )
	{
		return false;
	}

	cpu.write_gpr ( 4, rsp + 8 );
	cpu.rip = return_address;

	return true;
}

inline auto extcall::register_functions ( ) -> void
{
	if ( this->get_emulator ( ) == nullptr )
	{
		return;
	}

	g_external_call_handler = [ ] ( std::uint64_t address, cpu_state& cpu, const memory_handler& mem ) -> bool
	{
		return caller->handle_external_call ( address, cpu, mem );
	};

	this->register_win32_hooks ( );
	this->register_heap_hooks ( );
}

inline auto extcall::read_c_string ( const memory_handler& mem, std::uint64_t address, std::size_t limit ) -> std::string
{
	if ( address == 0 )
	{
		return std::string ( );
	}

	std::string text;

	for ( std::size_t index = 0; index < limit; index++ )
	{
		char character = 0;

		if ( !mem.read ( address + index, &character, 1 ) )
		{
			break;
		}

		if ( character == '\0' )
		{
			break;
		}

		text.push_back ( character );
	}

	return text;
}

inline auto extcall::register_win32_hooks ( ) -> void
{
	const auto kernel32 = GetModuleHandleA ( "kernel32.dll" );

	if ( kernel32 == nullptr )
	{
		return;
	}

	const auto query_performance_counter = reinterpret_cast< std::uint64_t > ( GetProcAddress ( kernel32, "QueryPerformanceCounter" ) );

	if ( query_performance_counter != 0 )
	{
		caller->register_hook ( query_performance_counter, [ ] ( cpu_state& cpu, const memory_handler& mem ) -> bool
		{
			const auto output = cpu.read_gpr ( 1 );

			LARGE_INTEGER current{ };

			if ( !QueryPerformanceCounter ( &current ) )
			{
				return false;
			}

			if ( !mem.write ( output, &current.QuadPart, sizeof ( LONGLONG ) ) )
			{
				return false;
			}

			cpu.write_gpr ( 0, 1 );

			return extcall::return_from_hook ( cpu, mem );
		} );
	}

	struct simple_hook
	{
		const char* name;
		std::uint64_t result;
	};

	const simple_hook simple_hooks[ ] =
	{
		{ "WaitForSingleObjectEx", 0 },
		{ "EnterCriticalSection", 0 },
		{ "LeaveCriticalSection", 0 },
		{ "FlsSetValue", 1 },
		{ "FlsGetValue", 0 },
		{ "GetLastError", 0 },
		{ "SetLastError", 0 },
	};

	for ( const auto& entry : simple_hooks )
	{
		const auto address = reinterpret_cast< std::uint64_t > ( GetProcAddress ( kernel32, entry.name ) );

		if ( address != 0 )
		{
			caller->register_hook ( address, extcall::make_returning_hook ( entry.result ) );
		}
	}
}

inline auto extcall::register_heap_hooks ( ) -> void
{
	const auto ntdll = GetModuleHandleA ( "ntdll.dll" );

	if ( ntdll == nullptr )
	{
		return;
	}


	const auto allocate_heap = reinterpret_cast< std::uint64_t > ( GetProcAddress ( ntdll, "RtlAllocateHeap" ) );

	if ( allocate_heap != 0 )
	{
		caller->register_hook ( allocate_heap, [ ] ( cpu_state& cpu, const memory_handler& mem ) -> bool
		{
			const auto flags = cpu.read_gpr ( 2 );
			const auto size = static_cast< std::size_t > ( cpu.read_gpr ( 8 ) );
			const auto address = caller->allocate ( size );

			if ( address != 0 && ( flags & 0x8u ) != 0u )
			{
				const std::vector< std::uint8_t > zeros ( size, 0u );

				mem.write ( address, zeros.data ( ), zeros.size ( ) );
			}

			cpu.write_gpr ( 0, address );

			return extcall::return_from_hook ( cpu, mem );
		} );
	}

	const auto free_heap = reinterpret_cast< std::uint64_t > ( GetProcAddress ( ntdll, "RtlFreeHeap" ) );

	if ( free_heap != 0 )
	{
		caller->register_hook ( free_heap, [ ] ( cpu_state& cpu, const memory_handler& mem ) -> bool
		{
			caller->release ( cpu.read_gpr ( 8 ) );

			cpu.write_gpr ( 0, 1 );

			return extcall::return_from_hook ( cpu, mem );
		} );
	}

	const auto reallocate_heap = reinterpret_cast< std::uint64_t > ( GetProcAddress ( ntdll, "RtlReAllocateHeap" ) );

	if ( reallocate_heap != 0 )
	{
		caller->register_hook ( reallocate_heap, [ ] ( cpu_state& cpu, const memory_handler& mem ) -> bool
		{
			const auto original = cpu.read_gpr ( 8 );
			const auto size = static_cast< std::size_t > ( cpu.read_gpr ( 9 ) );
			const auto address = caller->allocate ( size );

			if ( address != 0 && original != 0 )
			{
				const auto previous = caller->allocation_size ( original );
				const auto copy_size = previous < size ? previous : size;

				if ( copy_size != 0 )
				{
					std::vector< std::uint8_t > scratch ( copy_size, 0u );

					if ( mem.read ( original, scratch.data ( ), scratch.size ( ) ) )
					{
						mem.write ( address, scratch.data ( ), scratch.size ( ) );
					}
				}

				caller->release ( original );
			}

			cpu.write_gpr ( 0, address );

			return extcall::return_from_hook ( cpu, mem );
		} );
	}

	const auto kernel32 = GetModuleHandleA ( "kernel32.dll" );

	if ( kernel32 != nullptr )
	{
		const auto heap_free = reinterpret_cast< std::uint64_t > ( GetProcAddress ( kernel32, "HeapFree" ) );

		if ( heap_free != 0 )
		{
			caller->register_hook ( heap_free, [ ] ( cpu_state& cpu, const memory_handler& mem ) -> bool
			{
				caller->release ( cpu.read_gpr ( 8 ) );

				cpu.write_gpr ( 0, 1 );

				return extcall::return_from_hook ( cpu, mem );
			} );
		}

		const auto heap_alloc = reinterpret_cast< std::uint64_t > ( GetProcAddress ( kernel32, "HeapAlloc" ) );

		if ( heap_alloc != 0 )
		{
			caller->register_hook ( heap_alloc, [ ] ( cpu_state& cpu, const memory_handler& mem ) -> bool
			{
				const auto flags = cpu.read_gpr ( 2 );
				const auto size = static_cast< std::size_t > ( cpu.read_gpr ( 8 ) );
				const auto address = caller->allocate ( size );

				if ( address != 0 && ( flags & 0x8u ) != 0u )
				{
					const std::vector< std::uint8_t > zeros ( size, 0u );

					mem.write ( address, zeros.data ( ), zeros.size ( ) );
				}

				cpu.write_gpr ( 0, address );

				return extcall::return_from_hook ( cpu, mem );
			} );
		}
	}

	const auto size_heap = reinterpret_cast< std::uint64_t > ( GetProcAddress ( ntdll, "RtlSizeHeap" ) );

	if ( size_heap != 0 )
	{
		caller->register_hook ( size_heap, [ ] ( cpu_state& cpu, const memory_handler& mem ) -> bool
		{
			cpu.write_gpr ( 0, caller->allocation_size ( cpu.read_gpr ( 8 ) ) );

			return extcall::return_from_hook ( cpu, mem );
		} );
	}
}