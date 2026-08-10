#include <source/imports.hpp>

auto main ( ) -> std::int32_t
{
	if ( !memory->attach ( L"game.exe" ) )
	{
		return 0;
	}

	std::vector< std::uint8_t > fake_stack ( 0x400000 );

	auto cpu = emulator
	(
		[ &fake_stack ]( std::uint64_t address, void* buffer, std::size_t size ) -> bool
		{
			if ( address >= extcall::stack_base && address + size <= extcall::stack_base + fake_stack.size ( ) )
			{
				std::memcpy ( buffer, &fake_stack[ address - extcall::stack_base ], size );

				return true;
			}

			if ( caller->heap_read ( address, buffer, size ) )
			{
				return true;
			}

			return memory->read_memory ( address, buffer, size );
		},
		[ &fake_stack ]( std::uint64_t address, const void* buffer, std::size_t size ) -> bool
		{
			if ( address >= extcall::stack_base && address + size <= extcall::stack_base + fake_stack.size ( ) )
			{
				std::memcpy ( &fake_stack[ address - extcall::stack_base ], buffer, size );

				return true;
			}

			caller->heap_write ( address, buffer, size );

			return true;
		}
	);

	caller->set_emulator ( &cpu );

	caller->register_functions ( );

	caller->register_modules ( { memory->game_assembly, memory->unity_player } );

	if ( !il2cpp->initialize ( ) )
	{
		return 0;
	}
	
	/* example */
	const auto screen = il2cpp->get_class ( "Screen", "UnityEngine" );
	if ( !screen )
	{
		return 0;
	}

	const auto width = caller->call<int> ( screen->get_method ( "get_width", 0 )->il2cpp_method_get_pointer ( ) );
	const auto height = caller->call<int> ( screen->get_method ( "get_height", 0 )->il2cpp_method_get_pointer ( ) );
	std::printf ( "%d, %d\n", width, height );

	caller->set_emulator ( nullptr );

	memory->detach ( );

	return 1;
}