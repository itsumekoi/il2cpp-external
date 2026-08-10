# il2cpp-external

An external IL2CPP API. It enumerates classes, fields and methods in a live IL2CPP game and calls them, from outside the process — nothing is injected, no thread is hijacked, and the target is only ever read from.

It works by resolving the IL2CPP runtime through `GameAssembly.dll`'s own export table and then running each runtime call on a bundled x86-64 emulator ([emucall](https://github.com/Itsumekoi/emucall-main)), which interprets the game's own code in your address space against a virtual CPU and a fake stack. Because the runtime API is reached through exports rather than hardcoded addresses, nothing here needs re-deriving when the game updates.

## Usage

Header-only. Add the project directory to your include path and include `source/imports.hpp`.

```cpp
if ( !memory->attach ( L"game.exe" ) )
{
    return 0;
}

std::vector< std::uint8_t > fake_stack ( 0x400000 );

auto cpu = emulator ( /* read callback */, /* write callback */ );

caller->set_emulator ( &cpu );
caller->register_functions ( );
caller->register_modules ( { memory->game_assembly, memory->unity_player } );

if ( !il2cpp->initialize ( ) )
{
    return 0;
}
```

`initialize` resolves every `il2cpp_*` export, walks the domain's assemblies and images, and builds a class cache. From there:

```cpp
const auto screen = il2cpp->get_class ( "Screen", "UnityEngine" );

const auto width  = caller->call< int > ( screen->get_method ( "get_width",  0 )->il2cpp_method_get_pointer ( ) );
const auto height = caller->call< int > ( screen->get_method ( "get_height", 0 )->il2cpp_method_get_pointer ( ) );
```

`main.cpp` is a complete working skeleton of the above, including the two memory callbacks that route each access to the fake stack, the emulated heap or the real process.

## API

| | |
|---|---|
| `il2cpp->initialize ( )` | resolve exports, walk assemblies, build the class cache |
| `il2cpp->get_class ( name, namespace )` | look up a class by name, namespace optional |
| `class->get_method ( name, argc )` | look up a method, overloads resolved by argument count |
| `class->get_field ( name )` | look up a field |
| `method->il2cpp_method_get_pointer ( )` | native entry point, ready to pass to `caller->call` |
| `caller->call< T > ( address, args... )` | run it and get the return value |

Class, field, method and type queries are thin wrappers over the corresponding `il2cpp_*` exports, each one executed through the emulator.

## Offsets

The library reads exactly one raw struct offset: `methodPointer` at `MethodInfo + 0x00`, in `il2cpp_method_get_pointer`. IL2CPP intentionally exposes no export that returns it — the sanctioned path is `il2cpp_runtime_invoke`, which would mean boxing arguments and unboxing results for every call — and `methodPointer` is the first member of `MethodInfo` in every IL2CPP version, so offset zero is a property of the layout rather than a value that drifts between builds.

Everything else, including class parents and static field data, goes through the exported API.

## Building

Windows x64, C++20, MSVC. Open `il2cpp-external.slnx` and build Release|x64, or:

```
msbuild il2cpp-external.slnx /p:Configuration=Release /p:Platform=x64
```

Set the process name in `main.cpp` to your target before running.

## License

MIT. See [LICENSE](LICENSE).
