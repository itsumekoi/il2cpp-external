#pragma once

#include <source/imports.hpp>

enum class il2cpp_type_enum : std::uint8_t
{
    end = 0x00, // End marker
    void_type = 0x01, // void
    boolean = 0x02, // bool
    char_type = 0x03, // char
    i1 = 0x04, // sbyte (int8_t)
    u1 = 0x05, // byte (uint8_t)
    i2 = 0x06, // short (int16_t)
    u2 = 0x07, // ushort (uint16_t)
    i4 = 0x08, // int (int32_t)
    u4 = 0x09, // uint (uint32_t)
    i8 = 0x0A, // long (int64_t)
    u8 = 0x0B, // ulong (uint64_t)
    r4 = 0x0C, // float
    r8 = 0x0D, // double
    string = 0x0E, // string (System.String)
    ptr = 0x0F, // Pointer types (T*)
    byref = 0x10, // Reference types (ref/out parameters, T&)
    valuetype = 0x11, // Structs and enums (Vector3, ItemId, etc)
    class_type = 0x12, // Class reference types (BasePlayer, GameObject, etc)
    var = 0x13, // Generic type parameter (T in class Foo<T>)
    array = 0x14, // Multi-dimensional arrays (T[,] or T[,,])
    genericinst = 0x15, // Generic instances (List<T>, Dictionary<K,V>, Action<T>, etc)
    typedbyref = 0x16, // TypedReference
    i = 0x18, // IntPtr (native int)
    u = 0x19, // UIntPtr (native uint)
    fnptr = 0x1B, // Function pointer
    object = 0x1C, // System.Object
    szarray = 0x1D, // Single-dimension arrays (T[])
    mvar = 0x1E, // Method generic parameter (T in void Foo<T>())
    cmod_reqd = 0x1F, // Required custom modifier
    cmod_opt = 0x20, // Optional custom modifier
    internal = 0x21  // Internal compiler type
};

enum class field_attribute : std::uint16_t
{
    field_access_mask = 0x0007, // Mask to extract field access level
    compiler_controlled = 0x0000, // Member not referenceable
    private_access = 0x0001, // Accessible only by parent type
    fam_and_assem = 0x0002, // Accessible by sub-types in this assembly
    assembly = 0x0003, // Accessible by anyone in assembly
    family = 0x0004, // Accessible by type and sub-types
    fam_or_assem = 0x0005, // Accessible by sub-types or anyone in assembly
    public_access = 0x0006, // Accessible by anyone with visibility

    static_field = 0x0010, // Defined on type, not instance
    init_only = 0x0020, // Field can only be initialized, not written to after init
    literal = 0x0040, // Value is compile time constant
    not_serialized = 0x0080, // Field does not have to be serialized
    special_name = 0x0200, // Field is special
    pinvoke_impl = 0x2000, // Implementation is forwarded through PInvoke

    reserved_mask = 0x9500, // Reserved flags for runtime use
    rt_special_name = 0x0400, // CLI provides special behavior
    has_field_marshal = 0x1000, // Field has marshalling information
    has_default = 0x8000, // Field has default value
    has_field_rva = 0x0100, // Field has RVA
};

enum class method_impl_attribute : std::uint16_t
{
    code_type_mask = 0x0003, // Mask for code type
    il = 0x0000, // Method impl is IL
    native = 0x0001, // Method impl is native
    optil = 0x0002, // Method impl is OPTIL
    runtime = 0x0003, // Method impl is provided by runtime

    managed_mask = 0x0004, // Mask for managed/unmanaged
    unmanaged = 0x0004, // Method impl is unmanaged
    managed = 0x0000, // Method impl is managed

    forward_ref = 0x0010, // Method is defined, impl is not provided
    preserve_sig = 0x0080, // Method signature is exported exactly as declared
    internal_call = 0x1000, // Internal call to runtime
    synchronized = 0x0020, // Method is single threaded through body
    noinlining = 0x0008, // Method may not be inlined
    max_method_impl_val = 0xffff, // Range check value
};

enum class method_attribute : std::uint16_t
{
    member_access_mask = 0x0007, // Mask to extract member access level
    compiler_controlled = 0x0000, // Member not referenceable
    private_access = 0x0001, // Accessible only by parent type
    fam_and_assem = 0x0002, // Accessible by sub-types in this assembly
    assem = 0x0003, // Accessible by anyone in assembly
    family = 0x0004, // Accessible by type and sub-types
    fam_or_assem = 0x0005, // Accessible by sub-types or anyone in assembly
    public_access = 0x0006, // Accessible by anyone with visibility

    static_method = 0x0010, // Defined on type, not instance
    final = 0x0020, // Method cannot be overridden
    virtual_method = 0x0040, // Method is virtual
    hide_by_sig = 0x0080, // Method hides by name and signature

    vtable_layout_mask = 0x0100, // Mask for vtable layout
    reuse_slot = 0x0000, // Method reuses existing slot in vtable
    new_slot = 0x0100, // Method gets new slot in vtable

    strict = 0x0200, // Method can only be overridden if accessible
    abstract = 0x0400, // Method does not have implementation
    special_name = 0x0800, // Method is special

    pinvoke_impl = 0x2000, // Implementation is forwarded through PInvoke
    unmanaged_export = 0x0008, // Managed method exported via thunk to unmanaged code

    reserved_mask = 0xd000, // Reserved flags for runtime use
    rt_special_name = 0x1000, // CLI provides special behavior
    has_security = 0x4000, // Method has security attributes
    require_sec_object = 0x8000, // Method calls another method with security check
};

enum class type_attribute : std::uint32_t
{
    none = 0x67676767,
    visibility_mask = 0x00000007, // Mask to extract type visibility
    not_public = 0x00000000, // Type not public
    public_type = 0x00000001, // Type is public
    nested_public = 0x00000002, // Nested type public visibility
    nested_private = 0x00000003, // Nested type private visibility
    nested_family = 0x00000004, // Nested type family visibility
    nested_assembly = 0x00000005, // Nested type assembly visibility
    nested_fam_and_assem = 0x00000006, // Nested type family and assembly visibility
    nested_fam_or_assem = 0x00000007, // Nested type family or assembly visibility

    layout_mask = 0x00000018, // Mask for class layout
    auto_layout = 0x00000000, // Class fields are auto-laid out
    sequential_layout = 0x00000008, // Class fields are laid out sequentially
    explicit_layout = 0x00000010, // Class fields are laid out explicitly

    class_semantic_mask = 0x00000020, // Mask for class semantics
    class_type = 0x00000000, // Type is a class
    interface_type = 0x00000020, // Type is an interface

    abstract_type = 0x00000080, // Type is abstract
    sealed = 0x00000100, // Type is concrete and cannot be extended
    special_name = 0x00000400, // Type name is special

import = 0x00001000, // Class/interface imported from another module
    serializable = 0x00002000, // Type is serializable

    string_format_mask = 0x00030000, // Mask for string format
    ansi_class = 0x00000000, // LPSTR is interpreted as ANSI
    unicode_class = 0x00010000, // LPSTR is interpreted as UNICODE
    auto_class = 0x00020000, // LPSTR is interpreted automatically

    before_field_init = 0x00100000, // Initialize type any time before first static field access
    forwarder = 0x00200000, // Type is a forwarder to another assembly

    reserved_mask = 0x00040800, // Reserved flags for runtime use
    rt_special_name = 0x00000800, // CLI provides special behavior
    has_security = 0x00040000, // Type has security attributes
};

enum class param_attribute : std::uint16_t
{
    in = 0x0001, // Param is [In]
    out = 0x0002, // Param is [Out]
    optional = 0x0010, // Param is optional
    reserved_mask = 0xf000, // Reserved flags for runtime use
    has_default = 0x1000, // Param has default value
    has_field_marshal = 0x2000, // Param has field marshalling information
    unused = 0xcfe0, // Reserved (not used)
};

enum class generic_param_attribute : std::uint8_t
{
    non_variant = 0x00, // Generic parameter is non-variant
    covariant = 0x01, // Generic parameter is covariant (out)
    contravariant = 0x02, // Generic parameter is contravariant (in)
    variance_mask = 0x03, // Mask for variance flags

    reference_type = 0x04, // Generic parameter must be reference type
    not_nullable_value_type = 0x08, // Generic parameter must be non-nullable value type
    default_constructor = 0x10, // Generic parameter must have default constructor
    special_constraint_mask = 0x1c, // Mask for special constraints
};

enum class assembly_ref_flag : std::uint32_t
{
    full_public_key = 0x00000001, // Assembly ref holds full (unhashed) public key
    retargetable = 0x00000100, // Implementation of assembly can be retargeted at runtime
    enable_jit_tracking = 0x00008000, // Enable JIT compiler tracking
    disable_jit_optimizer = 0x00004000, // Disable JIT compiler optimizer
};

namespace il2cpp_stack
{
    std::uint32_t offset_counter = 0;
    std::uint32_t slot_size = 0x100;
    std::uint32_t slot_count = 16;
    std::uint32_t stack_start = 0x1000;

    auto get_unique_stack_ptr ( ) -> std::uint64_t
    {
        const auto slot = ( offset_counter++ ) % slot_count;
        const auto offset = stack_start + ( slot * slot_size );

        return extcall::initial_rsp + offset;
    }

    auto reset ( ) -> void
    {
        offset_counter = 0;
    }
}

class il2cpp_image;
class il2cpp_class;
class il2cpp_type;
class il2cpp_method;

class api
{
public:
    inline static std::vector<il2cpp_image*> images;
    inline static std::unordered_map<std::string, std::uint64_t> function;

    auto is_valid ( ) const -> bool
    {
        return reinterpret_cast< std::uint64_t >( this ) >= 0x10000 &&
            reinterpret_cast< std::uint64_t >( this ) <= 0x7FFFFFFFFFFF;
    }
};

class il2cpp_string : public api
{
public:
    auto buffer ( ) -> std::string
    {
        if ( !this->is_valid ( ) )
        {
            return std::string ( );
        }

        char buffer[ 256 ] = { 0 };
        memory->read_memory ( reinterpret_cast< std::uint64_t >( this ), buffer, sizeof ( buffer ) - 1 );
        return std::string ( buffer );
    }
};

class il2cpp_type : public api
{
public:
    auto il2cpp_type_get_attrs ( ) -> type_attribute
    {
        if ( !this->is_valid ( ) )
        {
            return static_cast< type_attribute >( 0 );
        }

        const auto result = caller->call<std::uint32_t>
            (
                this->function[ "il2cpp_type_get_attrs" ],
                reinterpret_cast< std::uint64_t >( this )
            );

        return static_cast< type_attribute >( result );
    }

    auto il2cpp_type_get_type ( ) -> il2cpp_type_enum
    {
        if ( !this->is_valid ( ) )
        {
            return static_cast< il2cpp_type_enum >( 0 );
        }

        const auto result = caller->call<std::uint8_t>
            (
                this->function[ "il2cpp_type_get_type" ],
                reinterpret_cast< std::uint64_t >( this )
            );

        return static_cast< il2cpp_type_enum >( result );
    }

    auto il2cpp_type_get_name ( ) -> il2cpp_string*
    {
        if ( !this->is_valid ( ) )
        {
            return nullptr;
        }

        const auto result = caller->call<std::uint64_t>
            (
                this->function[ "il2cpp_type_get_name" ],
                reinterpret_cast< std::uint64_t >( this )
            );

        return reinterpret_cast< il2cpp_string* >( result );
    }

    auto il2cpp_type_get_class_or_element_class ( ) -> il2cpp_class*
    {
        if ( !this->is_valid ( ) )
        {
            return nullptr;
        }

        const auto result = caller->call<std::uint64_t>
            (
                this->function[ "il2cpp_type_get_class_or_element_class" ],
                reinterpret_cast< std::uint64_t >( this )
            );

        return reinterpret_cast< il2cpp_class* >( result );
    }

    auto il2cpp_type_is_static ( ) -> bool
    {
        if ( !this->is_valid ( ) )
        {
            return false;
        }

        const auto result = caller->call<bool>
            (
                this->function[ "il2cpp_type_is_static" ],
                reinterpret_cast< std::uint64_t >( this )
            );

        return result;
    }

    auto get_type_as_string ( ) -> std::string
    {
        if ( !this->is_valid ( ) )
        {
            return {};
        }

        const auto type = this->il2cpp_type_get_type ( );

        switch ( type )
        {
            case il2cpp_type_enum::end:         return"end";
            case il2cpp_type_enum::void_type:   return"void";
            case il2cpp_type_enum::boolean:     return"bool";
            case il2cpp_type_enum::char_type:   return"char";
            case il2cpp_type_enum::i1:          return"std::int8_t";
            case il2cpp_type_enum::u1:          return"std::uint8_t";
            case il2cpp_type_enum::i2:          return"std::int16_t";
            case il2cpp_type_enum::u2:          return"std::uint16_t";
            case il2cpp_type_enum::i4:          return"std::int32_t";
            case il2cpp_type_enum::u4:          return"std::uint32_t";
            case il2cpp_type_enum::i8:          return"std::int64_t";
            case il2cpp_type_enum::u8:          return"std::uint64_t";
            case il2cpp_type_enum::r4:          return"float";
            case il2cpp_type_enum::r8:          return"double";
            case il2cpp_type_enum::string:      return"std::string";
            case il2cpp_type_enum::ptr:         return"void*";
            case il2cpp_type_enum::byref:       return"void&";
            case il2cpp_type_enum::valuetype:   return"valuetype";
            case il2cpp_type_enum::class_type:  return"class";
            case il2cpp_type_enum::var:         return"T";
            case il2cpp_type_enum::array:       return"array";
            case il2cpp_type_enum::genericinst: return"genericinst";
            case il2cpp_type_enum::typedbyref:  return"TypedReference";
            case il2cpp_type_enum::i:           return"std::intptr_t";
            case il2cpp_type_enum::u:           return"std::uintptr_t";
            case il2cpp_type_enum::fnptr:       return"fnptr";
            case il2cpp_type_enum::object:      return"void*";
            case il2cpp_type_enum::szarray:     return"szarray";
            case il2cpp_type_enum::mvar:        return"T";
            case il2cpp_type_enum::cmod_reqd:   return"cmod_reqd";
            case il2cpp_type_enum::cmod_opt:    return"cmod_opt";
            case il2cpp_type_enum::internal:    return"internal";
            default:                            return"unknown";
        }
    }
};

class il2cpp_field : public api
{
public:
    auto il2cpp_field_get_flags ( ) -> field_attribute
    {
        if ( !this->is_valid ( ) )
        {
            return static_cast< field_attribute >( 0 );
        }

        const auto result = caller->call<std::uint16_t>
            (
                this->function[ "il2cpp_field_get_flags" ],
                reinterpret_cast< std::uint64_t >( this )
            );

        return static_cast< field_attribute >( result );
    }

    auto il2cpp_field_get_offset ( ) -> std::uint64_t
    {
        if ( !this->is_valid ( ) )
        {
            return 0;
        }

        const auto result = caller->call<std::uint64_t>
            (
                this->function[ "il2cpp_field_get_offset" ],
                reinterpret_cast< std::uint64_t >( this )
            );

        return result;
    }

    auto il2cpp_field_get_type ( ) -> il2cpp_type*
    {
        if ( !this->is_valid ( ) )
        {
            return nullptr;
        }

        const auto result = caller->call<std::uint64_t>
            (
                this->function[ "il2cpp_field_get_type" ],
                reinterpret_cast< std::uint64_t >( this )
            );

        return reinterpret_cast< il2cpp_type* >( result );
    }

    auto il2cpp_field_get_name ( ) -> il2cpp_string*
    {
        if ( !this->is_valid ( ) )
        {
            return nullptr;
        }

        const auto result = caller->call<std::uint64_t>
            (
                this->function[ "il2cpp_field_get_name" ],
                reinterpret_cast< std::uint64_t >( this )
            );

        return reinterpret_cast< il2cpp_string* >( result );
    }
};

class il2cpp_method : public api
{
public:
    auto il2cpp_method_get_pointer ( ) -> std::uint64_t
    {
        if ( !this->is_valid ( ) )
        {
            return 0;
        }

        return memory->read<std::uint64_t> ( reinterpret_cast< std::uint64_t >( this ) + 0x00 );
    }

    auto il2cpp_method_get_param ( std::uint32_t index ) -> il2cpp_type*
    {
        if ( !this->is_valid ( ) )
        {
            return nullptr;
        }

        const auto result = caller->call<std::uint64_t>
            (
                this->function[ "il2cpp_method_get_param" ],
                reinterpret_cast< std::uint64_t >( this ),
                index
            );

        return reinterpret_cast< il2cpp_type* >( result );
    }

    auto il2cpp_method_get_flags ( ) -> std::uint16_t
    {
        if ( !this->is_valid ( ) )
        {
            return 0;
        }

        const auto result = caller->call<std::uint16_t>
            (
                this->function[ "il2cpp_method_get_flags" ],
                reinterpret_cast< std::uint64_t >( this )
            );

        return result;
    }

    auto il2cpp_method_get_name ( ) -> il2cpp_string*
    {
        if ( !this->is_valid ( ) )
        {
            return nullptr;
        }

        const auto result = caller->call<std::uint64_t>
            (
                this->function[ "il2cpp_method_get_name" ],
                reinterpret_cast< std::uint64_t >( this )
            );

        return reinterpret_cast< il2cpp_string* >( result );
    }

    auto il2cpp_method_get_class ( ) -> il2cpp_class*
    {
        if ( !this->is_valid ( ) )
        {
            return nullptr;
        }

        const auto result = caller->call<std::uint64_t>
            (
                this->function[ "il2cpp_method_get_class" ],
                reinterpret_cast< std::uint64_t >( this )
            );

        return reinterpret_cast< il2cpp_class* >( result );
    }

    auto il2cpp_method_get_param_count ( ) -> std::uint8_t
    {
        if ( !this->is_valid ( ) )
        {
            return 0;
        }

        const auto result = caller->call<std::uint8_t>
            (
                this->function[ "il2cpp_method_get_param_count" ],
                reinterpret_cast< std::uint64_t >( this )
            );

        return result;
    }

    auto il2cpp_method_get_return_type ( ) -> il2cpp_type*
    {
        if ( !this->is_valid ( ) )
        {
            return nullptr;
        }

        const auto result = caller->call<std::uint64_t>
            (
                this->function[ "il2cpp_method_get_return_type" ],
                reinterpret_cast< std::uint64_t >( this )
            );

        return reinterpret_cast< il2cpp_type* >( result );
    }
};

class il2cpp_class : public api
{
public:
    auto il2cpp_class_get_static_field_data ( ) -> std::uint64_t
    {
        if ( !this->is_valid ( ) )
        {
            return 0;
        }

        return caller->call<std::uint64_t>
            (
                this->function[ "il2cpp_class_get_static_field_data" ],
                reinterpret_cast< std::uint64_t >( this )
            );
    }

    auto il2cpp_class_get_parent ( ) -> il2cpp_class*
    {
        if ( !this->is_valid ( ) )
        {
            return { };
        }

        const auto result = caller->call<std::uint64_t>
            (
                this->function[ "il2cpp_class_get_parent" ],
                reinterpret_cast< std::uint64_t >( this )
            );

        return reinterpret_cast< il2cpp_class* >( result );
    }

    auto il2cpp_class_get_flags ( ) -> std::uint32_t
    {
        if ( !this->is_valid ( ) )
        {
            return static_cast< std::uint32_t >( type_attribute::none );
        }

        const auto result = caller->call<std::uint32_t>
            (
                this->function[ "il2cpp_class_get_flags" ],
                reinterpret_cast< std::uint64_t >( this )
            );

        return result;
    }

    auto il2cpp_class_get_name ( ) -> il2cpp_string*
    {
        if ( !this->is_valid ( ) )
        {
            return nullptr;
        }

        const auto result = caller->call<std::uint64_t>
            (
                this->function[ "il2cpp_class_get_name" ],
                reinterpret_cast< std::uint64_t >( this )
            );

        return reinterpret_cast< il2cpp_string* >( result );
    }

    auto il2cpp_class_get_namespace ( ) -> il2cpp_string*
    {
        if ( !this->is_valid ( ) )
        {
            return nullptr;
        }

        const auto result = caller->call<std::uint64_t>
            (
                this->function[ "il2cpp_class_get_namespace" ],
                reinterpret_cast< std::uint64_t >( this )
            );

        return reinterpret_cast< il2cpp_string* >( result );
    }

    auto il2cpp_class_get_declaring_type ( ) -> il2cpp_class*
    {
        if ( !this->is_valid ( ) )
        {
            return nullptr;
        }

        const auto result = caller->call<std::uint64_t>
            (
                this->function[ "il2cpp_class_get_declaring_type" ],
                reinterpret_cast< std::uint64_t >( this )
            );

        return reinterpret_cast< il2cpp_class* >( result );
    }

    auto il2cpp_class_get_type ( ) -> il2cpp_type*
    {
        if ( !this->is_valid ( ) )
        {
            return nullptr;
        }

        const auto result = caller->call<std::uint64_t>
            (
                this->function[ "il2cpp_class_get_type" ],
                reinterpret_cast< std::uint64_t >( this )
            );

        return reinterpret_cast< il2cpp_type* >( result );
    }

    auto il2cpp_class_get_nested_types ( ) -> std::vector<il2cpp_class*>
    {
        if ( !this->is_valid ( ) )
        {
            return {};
        }

        std::vector<il2cpp_class*> nested_types;
        const auto iterator_ptr = il2cpp_stack::get_unique_stack_ptr ( );

        std::uint64_t zero = 0;
        caller->get_emulator ( )->get_memory ( ).write ( iterator_ptr, &zero, sizeof ( std::uint64_t ) );

        while ( true )
        {
            const auto nested = caller->call<std::uint64_t>
                (
                    this->function[ "il2cpp_class_get_nested_types" ],
                    reinterpret_cast< std::uint64_t >( this ),
                    iterator_ptr
                );

            if ( !nested )
            {
                break;
            }

            nested_types.push_back ( reinterpret_cast< il2cpp_class* >( nested ) );
        }

        return nested_types;
    }

    auto il2cpp_class_get_methods ( ) -> std::vector<il2cpp_method*>
    {
        if ( !this->is_valid ( ) )
        {
            return {};
        }

        std::vector<il2cpp_method*> methods;
        const auto iterator_ptr = il2cpp_stack::get_unique_stack_ptr ( );

        std::uint64_t zero = 0;
        caller->get_emulator ( )->get_memory ( ).write ( iterator_ptr, &zero, sizeof ( std::uint64_t ) );

        while ( true )
        {
            const auto method = caller->call<std::uint64_t>
                (
                    this->function[ "il2cpp_class_get_methods" ],
                    reinterpret_cast< std::uint64_t >( this ),
                    iterator_ptr
                );

            if ( !method )
            {
                break;
            }

            methods.push_back ( reinterpret_cast< il2cpp_method* >( method ) );
        }

        return methods;
    }

    auto il2cpp_class_get_fields ( ) -> std::vector<il2cpp_field*>
    {
        if ( !this->is_valid ( ) )
        {
            return {};
        }

        std::vector<il2cpp_field*> fields;
        const auto iterator_ptr = il2cpp_stack::get_unique_stack_ptr ( );

        std::uint64_t zero = 0;
        caller->get_emulator ( )->get_memory ( ).write ( iterator_ptr, &zero, sizeof ( std::uint64_t ) );

        while ( true )
        {
            const auto field = caller->call<std::uint64_t>
                (
                    this->function[ "il2cpp_class_get_fields" ],
                    reinterpret_cast< std::uint64_t >( this ),
                    iterator_ptr
                );

            if ( !field )
            {
                break;
            }

            fields.push_back ( reinterpret_cast< il2cpp_field* >( field ) );
        }

        return fields;
    }

    auto get_method ( const char* method_name, std::uint8_t param = 0xFF ) -> il2cpp_method*
    {
        if ( !this->is_valid ( ) || !method_name )
        {
            return nullptr;
        }

        const auto methods = this->il2cpp_class_get_methods ( );
        for ( const auto& method : methods )
        {
            if ( !method )
            {
                continue;
            }

            const auto name = method->il2cpp_method_get_name ( );
            if ( !name )
            {
                continue;
            }

            const auto name_str = name->buffer ( );
            const auto param_count = method->il2cpp_method_get_param_count ( );

            if ( param != 0xFF && param_count != param )
            {
                continue;
            }


            if ( std::strcmp ( name_str.c_str ( ), method_name ) == 0 )
            {
                return method;
            }
        }

        return nullptr;
    }

    auto get_field ( const char* field_name ) -> il2cpp_field*
    {
        if ( !this->is_valid ( ) || !field_name )
        {
            return nullptr;
        }

        const auto fields = this->il2cpp_class_get_fields ( );
        for ( const auto& field : fields )
        {
            if ( !field )
            {
                continue;
            }

            const auto name = field->il2cpp_field_get_name ( );
            if ( !name )
            {
                continue;
            }

            const auto name_str = name->buffer ( );
            if ( std::strcmp ( name_str.c_str ( ), field_name ) == 0 )
            {
                return field;
            }
        }
        return nullptr;
    }
};

class il2cpp_image : public api
{
public:
    auto il2cpp_image_get_class ( std::uint32_t index ) -> il2cpp_class*
    {
        if ( !this->is_valid ( ) )
        {
            return nullptr;
        }

        const auto result = caller->call<std::uint64_t>
            (
                this->function[ "il2cpp_image_get_class" ],
                reinterpret_cast< std::uint64_t >( this ),
                index
            );

        return reinterpret_cast< il2cpp_class* >( result );
    }

    auto il2cpp_image_get_name ( ) -> il2cpp_string*
    {
        if ( !this->is_valid ( ) )
        {
            return nullptr;
        }

        const auto result = caller->call<std::uint64_t>
            (
                this->function[ "il2cpp_image_get_name" ],
                reinterpret_cast< std::uint64_t >( this )
            );

        return reinterpret_cast< il2cpp_string* >( result );
    }

    auto il2cpp_image_get_class_count ( ) -> std::uint32_t
    {
        if ( !this->is_valid ( ) )
        {
            return 0;
        }

        const auto result = caller->call<std::uint32_t>
            (
                this->function[ "il2cpp_image_get_class_count" ],
                reinterpret_cast< std::uint64_t >( this )
            );

        return result;
    }
};

class il2cpp_assembly : public api
{
public:
    auto il2cpp_assembly_get_image ( ) -> il2cpp_image*
    {
        if ( !this->is_valid ( ) )
        {
            return nullptr;
        }

        const auto result = caller->call<std::uint64_t>
            (
                this->function[ "il2cpp_assembly_get_image" ],
                reinterpret_cast< std::uint64_t >( this )
            );

        return reinterpret_cast< il2cpp_image* >( result );
    }
};

class il2cpp_domain : public api
{
public:
    auto il2cpp_domain_get_assemblies ( ) -> std::vector<il2cpp_assembly*>
    {
        if ( !this->is_valid ( ) )
        {
            return {};
        }

        const auto size_ptr = il2cpp_stack::get_unique_stack_ptr ( );

        const auto assemblies_ptr = caller->call<std::uint64_t>
            (
                this->function[ "il2cpp_domain_get_assemblies" ],
                reinterpret_cast< std::uint64_t >( this ),
                size_ptr
            );

        std::size_t count = 0;
        const auto emu = caller->get_emulator ( );
        if ( !emu )
        {
            return {};
        }
        emu->get_memory ( ).read ( size_ptr, &count, sizeof ( std::size_t ) );

        if ( !assemblies_ptr || count == 0 )
        {
            return {};
        }

        constexpr std::size_t max_assemblies = 4096;
        if ( count > max_assemblies )
        {
            count = max_assemblies;
        }

        std::vector<il2cpp_assembly*> assemblies ( count );
        memory->read_memory ( assemblies_ptr, assemblies.data ( ), count * sizeof ( std::uint64_t ) );

        return assemblies;
    }

    auto il2cpp_domain_get ( ) -> il2cpp_domain*
    {
        const auto domain = caller->call<std::uint64_t> ( this->function[ "il2cpp_domain_get" ] );
        if ( !domain )
        {
            return nullptr;
        }

        return reinterpret_cast< il2cpp_domain* >( domain );
    }
};

class il2cpp_handler : public il2cpp_domain, public api
{
public:
    std::unordered_map<std::string, il2cpp_class*> class_cache;
    bool cache_initialized = false;

    auto build_class_cache ( ) -> void
    {
        if ( this->cache_initialized )
        {
            return;
        }

        std::size_t total = 0;
        for ( const auto& image : this->images )
        {
            if ( !image || !image->is_valid ( ) )
            {
                continue;
            }

            total += image->il2cpp_image_get_class_count ( );
        }

        this->class_cache.reserve ( total * 2 );

        for ( const auto& image : this->images )
        {
            if ( !image || !image->is_valid ( ) )
            {
                continue;
            }

            const auto class_count = image->il2cpp_image_get_class_count ( );
            if ( !class_count )
            {
                continue;
            }

            for ( std::uint32_t idx{ 0 }; idx < class_count; ++idx )
            {
                const auto il_class = image->il2cpp_image_get_class ( idx );
                if ( !il_class || !il_class->is_valid ( ) )
                {
                    continue;
                }

                const auto name_ptr = il_class->il2cpp_class_get_name ( );
                if ( !name_ptr || !name_ptr->is_valid ( ) )
                {
                    continue;
                }

                const auto name = name_ptr->buffer ( );
                if ( name.empty ( ) )
                {
                    continue;
                }

                const auto ns_ptr = il_class->il2cpp_class_get_namespace ( );
                const auto ns = ( ns_ptr && ns_ptr->is_valid ( ) ) ? ns_ptr->buffer ( ) : std::string ( );

                if ( !ns.empty ( ) )
                {
                    this->class_cache.try_emplace ( ns + "::" + name, il_class );
                }

                this->class_cache.try_emplace ( name, il_class );
            }
        }

        this->cache_initialized = true;
    }
public:
    template<typename t = il2cpp_class*>
    auto get_class ( const char* name, const char* space = nullptr ) -> t
    {
        if ( !name )
        {
            return nullptr;
        }

        if ( !this->cache_initialized )
        {
            this->build_class_cache ( );
        }

        if ( space && space[ 0 ] != '\0' )
        {
            const auto it = this->class_cache.find ( std::string ( space ) + "::" + name );
            if ( it != this->class_cache.end ( ) )
            {
                return reinterpret_cast< t > ( it->second );
            }
        }

        const auto it = this->class_cache.find ( std::string ( name ) );
        if ( it != this->class_cache.end ( ) )
        {
            return reinterpret_cast< t > ( it->second );
        }

        return nullptr;
    }

    auto initialize ( ) -> bool
    {
        il2cpp_stack::reset ( );

        const auto exports = memory->get_exports ( memory->game_assembly );
        if ( exports.empty ( ) )
        {
            return false;
        }

        for ( const auto& [name, rva] : exports )
        {
            this->function[ name ] = memory->game_assembly + rva;
        }

        const auto domain = this->il2cpp_domain_get ( );
        if ( !domain || !domain->is_valid ( ) )
        {
            return false;
        }

        const auto assemblies = domain->il2cpp_domain_get_assemblies ( );
        if ( assemblies.empty ( ) )
        {
            return false;
        }

        this->images.clear ( );

        for ( const auto assembly : assemblies )
        {
            if ( !assembly || !assembly->is_valid ( ) )
            {
                continue;
            }

            const auto image = assembly->il2cpp_assembly_get_image ( );
            if ( !image || !image->is_valid ( ) )
            {
                continue;
            }

            this->images.emplace_back ( image );
        }

        if ( this->images.empty ( ) )
        {
            return false;
        }

        this->build_class_cache ( );

        return true;
    }
};

const auto il2cpp = new il2cpp_handler ( );