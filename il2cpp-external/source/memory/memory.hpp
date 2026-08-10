#pragma once
#include <source/imports.hpp>

class mem_lib
{
public:
	auto read_memory ( std::uintptr_t address, void* buffer, std::size_t size ) -> bool
	{
		if ( !address || !buffer || !size )
		{
			return false;
		}

		return ReadProcessMemory ( this->handle, ( LPVOID )address, buffer, size, nullptr );
	}

	template< typename T >
	auto read ( std::uintptr_t address ) -> T
	{
		T buffer{ };

		if ( !address )
		{
			return buffer;
		}

		this->read_memory ( address, &buffer, sizeof ( T ) );
		return buffer;
	}

	auto read_string ( std::uintptr_t address, std::size_t max_size = 128 ) -> std::string
	{
		if ( !address || !max_size )
		{
			return { };
		}

		const auto limit = std::min ( max_size, static_cast< std::size_t > ( 1024 ) );

		std::string result;
		result.reserve ( limit );

		for ( std::size_t idx{ 0 }; idx < limit; ++idx )
		{
			char ch{ };

			if ( !this->read_memory ( address + idx, &ch, sizeof ( ch ) ) )
			{
				break;
			}

			if ( ch == '\0' )
			{
				break;
			}

			result.push_back ( ch );
		}

		return result;
	}

	auto get_exports ( std::uint64_t base ) -> std::vector< std::pair< std::string, std::uint32_t > >
	{
		const auto dos_header = this->read< IMAGE_DOS_HEADER > ( base );

		if ( dos_header.e_magic != IMAGE_DOS_SIGNATURE )
		{
			return { };
		}

		const auto nt_headers = this->read< IMAGE_NT_HEADERS64 > ( base + dos_header.e_lfanew );

		if ( nt_headers.Signature != IMAGE_NT_SIGNATURE )
		{
			return { };
		}

		const auto eat_va = nt_headers.OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_EXPORT ].VirtualAddress;

		if ( !eat_va )
		{
			return { };
		}

		const auto eat_header = this->read< IMAGE_EXPORT_DIRECTORY > ( base + eat_va );

		if ( !eat_header.NumberOfNames || !eat_header.AddressOfNames
			 || !eat_header.AddressOfNameOrdinals || !eat_header.AddressOfFunctions )
		{
			return { };
		}

		auto name_rvas = std::vector< std::uint32_t > ( eat_header.NumberOfNames );
		auto ordinals = std::vector< std::uint16_t > ( eat_header.NumberOfNames );

		if ( !this->read_memory ( base + eat_header.AddressOfNames, name_rvas.data ( ), name_rvas.size ( ) * sizeof ( std::uint32_t ) ) )
		{
			return { };
		}

		if ( !this->read_memory ( base + eat_header.AddressOfNameOrdinals, ordinals.data ( ), ordinals.size ( ) * sizeof ( std::uint16_t ) ) )
		{
			return { };
		}

		const auto rva_table = base + eat_header.AddressOfFunctions;

		std::vector< std::pair< std::string, std::uint32_t > > exports;
		exports.reserve ( eat_header.NumberOfNames );

		for ( std::uint32_t idx{ 0 }; idx < eat_header.NumberOfNames; ++idx )
		{
			if ( !name_rvas[ idx ] )
			{
				continue;
			}

			const auto name = this->read_string ( base + name_rvas[ idx ] );
			const auto func_rva = this->read< std::uint32_t > ( rva_table + ordinals[ idx ] * sizeof ( std::uint32_t ) );

			exports.emplace_back ( name, func_rva );
		}

		return exports;
	}

	auto get_module_base ( std::uint32_t pid, const wchar_t* name ) -> std::uint64_t
	{
		if ( !pid || !name )
		{
			return 0;
		}

		const auto handle = OpenProcess ( PROCESS_QUERY_INFORMATION, FALSE, pid );
		if ( !handle || handle == INVALID_HANDLE_VALUE )
		{
			return 0;
		}

		const static auto ntdll = GetModuleHandleA ( "ntdll" );
		const static auto nt_query_virtual_memory = reinterpret_cast< NTSTATUS ( __stdcall* ) ( HANDLE, void*, std::int32_t, void*, std::size_t, std::size_t* ) > ( GetProcAddress ( ntdll, "NtQueryVirtualMemory" ) );

		if ( !nt_query_virtual_memory )
		{
			CloseHandle ( handle );
			return 0;
		}

		auto current = 0ull;
		MEMORY_BASIC_INFORMATION mbi{ };

		while ( VirtualQueryEx ( handle, reinterpret_cast< void* > ( current ), &mbi, sizeof ( mbi ) ) )
		{
			current = reinterpret_cast< std::uintptr_t > ( mbi.BaseAddress ) + mbi.RegionSize;

			if ( mbi.Type != MEM_MAPPED && mbi.Type != MEM_IMAGE )
			{
				continue;
			}

			auto buffer = std::vector< std::uint8_t > ( 1024 );
			auto bytes = std::size_t{ };

			if ( nt_query_virtual_memory ( handle, mbi.BaseAddress, 2, buffer.data ( ), buffer.size ( ), &bytes ) != 0 )
			{
				continue;
			}

			const auto unicode_str = reinterpret_cast< UNICODE_STRING* > ( buffer.data ( ) );

			if ( !unicode_str->Buffer || !std::wcsstr ( unicode_str->Buffer, name )
				 || std::wcsstr ( unicode_str->Buffer, L".mui" ) )
			{
				continue;
			}

			const auto result = reinterpret_cast< std::uint64_t > ( mbi.BaseAddress );
			CloseHandle ( handle );
			return result;
		}

		CloseHandle ( handle );
		return 0;
	}

	auto get_process_id ( const wchar_t* name ) -> std::uint32_t
	{
		if ( !name )
		{
			return 0;
		}

		const auto snapshot = CreateToolhelp32Snapshot ( TH32CS_SNAPPROCESS, 0 );
		if ( !snapshot || snapshot == INVALID_HANDLE_VALUE )
		{
			return 0;
		}

		PROCESSENTRY32W entry{ };
		entry.dwSize = sizeof ( PROCESSENTRY32W );

		if ( !Process32FirstW ( snapshot, &entry ) )
		{
			CloseHandle ( snapshot );
			return 0;
		}

		do
		{
			if ( !std::wcsstr ( entry.szExeFile, name ) )
			{
				continue;
			}

			const auto pid = entry.th32ProcessID;
			CloseHandle ( snapshot );

			return pid;
		} while ( Process32NextW ( snapshot, &entry ) );

		CloseHandle ( snapshot );
		return 0;
	}

	auto attach ( std::uint32_t pid ) -> bool
	{
		this->detach ( );

		if ( !pid )
		{
			return false;
		}

		const auto opened = OpenProcess ( PROCESS_ALL_ACCESS, FALSE, pid );

		if ( !opened || opened == INVALID_HANDLE_VALUE )
		{
			return false;
		}

		this->handle = opened;
		this->process_id = pid;

		this->unity_player = this->get_module_base ( pid, L"UnityPlayer.dll" );
		this->game_assembly = this->get_module_base ( pid, L"GameAssembly.dll" );

		return true;
	}

	auto attach ( const wchar_t* name ) -> bool
	{
		return this->attach ( this->get_process_id ( name ) );
	}

	auto detach ( ) -> void
	{
		if ( this->handle && this->handle != INVALID_HANDLE_VALUE )
		{
			CloseHandle ( this->handle );
		}

		this->handle = nullptr;
		this->process_id = 0;
		this->unity_player = 0;
		this->game_assembly = 0;
	}
public:
	HANDLE handle = nullptr;
	std::uint32_t process_id = 0;

	std::uint64_t unity_player = 0;
	std::uint64_t game_assembly = 0;
};

const auto memory = new mem_lib ( );
