#include "stdafx.h"
#include "impl_base.h"
#include "vm.h"
#include "owca_vm.h"

namespace OwcaScript::Internal {
    bool ImplBase::Comparer::debug_break(bool v) {
        if (!v) {
            std::cout << "comparison failed\n";
        }
        return v;
    }
    bool ImplBase::Comparer::compare(const ImplExpr *l, const ImplExpr *r) {
        if (!l && !r) return true;
        if (!l || !r) return debug_break(false);
        return l->compare(r);
    }
    bool ImplBase::Comparer::compare(const ImplStat *l, const ImplStat *r) {
        if (!l && !r) return true;
        if (!l || !r) return debug_break(false);
        return l->compare(r);
    }

    enum class Serializer::SerializationOperation : unsigned char {
        Noop,
        Null,
        Int8,
        Int16,
        Int32,
        Int64,
        Float32,
        Float64,
        True,
        False,
        Blob,
        Span,
        Pair,
        Tuple,
        Enum,
        String,
        ImplStat,
        ImplExpr,
    };
    static std::string_view to_string(Serializer::SerializationOperation s) {
        switch(s) {
        case Serializer::SerializationOperation::Noop: return "Noop";
        case Serializer::SerializationOperation::Null: return "Null";
        case Serializer::SerializationOperation::Int8: return "Int8";
        case Serializer::SerializationOperation::Int16: return "Int16";
        case Serializer::SerializationOperation::Int32: return "Int32";
        case Serializer::SerializationOperation::Int64: return "Int64";
        case Serializer::SerializationOperation::Float32: return "Float32";
        case Serializer::SerializationOperation::Float64: return "Float64";
        case Serializer::SerializationOperation::True: return "True";
        case Serializer::SerializationOperation::False: return "False";
        case Serializer::SerializationOperation::Blob: return "Blob";
        case Serializer::SerializationOperation::Span: return "Span";
        case Serializer::SerializationOperation::Pair: return "Pair";
        case Serializer::SerializationOperation::Tuple: return "Tuple";
        case Serializer::SerializationOperation::Enum: return "Enum";
        case Serializer::SerializationOperation::String: return "String";
        case Serializer::SerializationOperation::ImplStat: return "ImplStat";
        case Serializer::SerializationOperation::ImplExpr: return "ImplExpr";
        }
        return "";
    }

    Serializer::Serializer(size_t size) {
        output_storage.reserve(1024 * 64);

        assert(magic.size() == 12);
        write(magic.data(), magic.size());
        std::uint32_t size2 = (std::uint32_t)size;
        if (size2 != size)
            throw OwcaVM::SerializationFailed{ std::format("can't serialize code of size {} - size doesn't fit in uint32", size) };
        write(&size2, sizeof(size2));
    }

    void Serializer::write(SerializationOperation s)
    {
        write(&s, sizeof(s));
    }
    void Serializer::write(const void *ptr, size_t sz) {
        auto s = output_storage.size();
        output_storage.resize(s + sz);
        std::memcpy(output_storage.data() + s, ptr, sz);
    }

    Deserializer::Deserializer(std::string_view fname, std::span<unsigned char> data, std::span<std::function<ImplStat*(Deserializer&, Line)>> stat_constructors, std::span<std::function<ImplExpr*(Deserializer&, Line)>> expr_constructors) :
                data(data), stat_constructors(stat_constructors), expr_constructors(expr_constructors), fname(fname) {
        if (data.size() < 16)
            throw OwcaVM::SerializationFailed{ std::format("{}: data too short ({}), can't read header", fname, data.size()) };
        if (std::string_view{ (char*)data.data(), 12 } != Serializer::magic)
            throw OwcaVM::SerializationFailed{ std::format("{}: magic doesn't match", fname) };
        std::uint32_t val;
        std::memcpy(&val, data.data() + 12, 4);
        position = 16;
        loaded_data.reserve(val);
    }
    std::vector<char> Deserializer::take_loaded_data()
    {
        //assert(loaded_data.size() == loaded_data_size);
        return std::move(loaded_data);
    }
    Deserializer::SerializationOperation Deserializer::peek() {
        if (position >= data.size()) throw OwcaVM::SerializationFailed{ std::format("{}: can't load a byte at position {} (size is only {})", fname, position, data.size()) };
            return (SerializationOperation)data[position];
    }
    bool Deserializer::try_require(SerializationOperation s) {
        if ((SerializationOperation)peek() != s) return false;
        require(s);
        return true;
    }
    void Deserializer::require(SerializationOperation s) {
        SerializationOperation t;
        assert(sizeof(t) == 1);
        auto pos = position;
        read(&t, sizeof(t));
        if (s != t) throw OwcaVM::SerializationFailed{ std::format("{}: data corruption at position {}, expected {}, got {}", fname, position, to_string(s), to_string(t)) };
    }
    void Deserializer::read(void *dst, size_t s) {
        if (position + s > data.size()) throw OwcaVM::SerializationFailed{ std::format("{}: can't load {} bytes at position {} (size is only {})", fname, s, position, data.size()) };
        std::memcpy(dst, data.data() + position, s);
        position += s;
    }
    void *Deserializer::allocate_raw(size_t size, size_t align) {
        assert((align & (align - 1)) == 0);
        auto p = loaded_data.size();
        p = (p + align - 1) & ~(align - 1);
        auto p2 = p + size;
        if (p2 > loaded_data.capacity()) throw OwcaVM::SerializationFailed{ std::format("{}: data corruption at output position {}, tried to allocate more (total {}) than expected ({})", fname, loaded_data.size(), p2, loaded_data.capacity()) };
        //std::cout << "Allocate " << p << " " << size << "\n";
        loaded_data.resize(p2);
        return loaded_data.data() + p;
    }
    ImplStat *Deserializer::allocate_stat(unsigned int kind, Line line) {
        if (kind >= stat_constructors.size()) throw OwcaVM::SerializationFailed{ std::format("{}: data corruption - invalid stat type ({})", fname, kind) };
        return stat_constructors[kind](*this, line);
    }
    ImplExpr *Deserializer::allocate_expr(unsigned int kind, Line line) {
        if (kind >= expr_constructors.size()) throw OwcaVM::SerializationFailed{ std::format("{}: data corruption - invalid expr type ({})", fname, kind) };
        return expr_constructors[kind](*this, line);
    }

    void Serializer::serialize_array_header() { write(SerializationOperation::Span); }
    void Serializer::serialize_pair_header() { write(SerializationOperation::Pair); }
    void Serializer::serialize_tuple_header() { write(SerializationOperation::Tuple); }
    void Serializer::serialize_enum_header() { write(SerializationOperation::Enum); }
    void Serializer::serialize_impl(std::uint64_t v) {
        if (v <= std::numeric_limits<std::uint8_t>::max()) {
            write(SerializationOperation::Int8);
            auto tmp = (std::uint8_t)v;
            write(&tmp, sizeof(tmp));
        }
        else if (v <= std::numeric_limits<std::uint16_t>::max()) {
            write(SerializationOperation::Int16);
            auto tmp = (std::uint16_t)v;
            write(&tmp, sizeof(tmp));
        }
        else if (v <= std::numeric_limits<std::uint32_t>::max()) {
            write(SerializationOperation::Int32);
            auto tmp = (std::uint32_t)v;
            write(&tmp, sizeof(tmp));
        }
        else {
            write(SerializationOperation::Int64);
            auto tmp = (std::uint64_t)v;
            write(&tmp, sizeof(tmp));
        }
    }
    void Serializer::serialize_impl(std::int64_t v) {
        if (v >= std::numeric_limits<std::int8_t>::min() && v <= std::numeric_limits<std::int8_t>::max()) {
            write(SerializationOperation::Int8);
            auto tmp = (std::uint8_t)v;
            write(&tmp, sizeof(tmp));
        }
        else if (v >= std::numeric_limits<std::int16_t>::min() && v <= std::numeric_limits<std::int16_t>::max()) {
            write(SerializationOperation::Int16);
            auto tmp = (std::uint16_t)v;
            write(&tmp, sizeof(tmp));
        }
        else if (v >= std::numeric_limits<std::int32_t>::min() && v <= std::numeric_limits<std::int32_t>::max()) {
            write(SerializationOperation::Int32);
            auto tmp = (std::uint32_t)v;
            write(&tmp, sizeof(tmp));
        }
        else {
            write(SerializationOperation::Int64);
            auto tmp = (std::uint64_t)v;
            write(&tmp, sizeof(tmp));
        }
    }
    void Serializer::serialize(float v) {
        write(SerializationOperation::Int32);
        write(&v, sizeof(v));
    }
    void Serializer::serialize(double v) {
        write(SerializationOperation::Int64);
        write(&v, sizeof(v));
    }
    void Serializer::serialize(bool v) {
        write(v ? SerializationOperation::True : SerializationOperation::False);
    }
    void Serializer::serialize(std::string_view v) {
        write(SerializationOperation::String);
        serialize(v.size());
        write(v.data(), v.size());
    }
    void Serializer::serialize(const ImplExpr *v) {
        if (!v) write(SerializationOperation::Null);
        else serialize(*v);
    }
    void Serializer::serialize(const ImplStat *v) {
        if (!v) write(SerializationOperation::Null);
        else serialize(*v);
    }
    void Serializer::serialize(const ImplExpr &v) {
        write(SerializationOperation::ImplExpr);
        serialize(v.kind());
        serialize(v.line.line);
        v.store_members(*this);
    }
    void Serializer::serialize(const ImplStat &v) {
        write(SerializationOperation::ImplStat);
        serialize(v.kind());
        serialize(v.line.line);
        v.store_members(*this);
    }


    void Deserializer::deserialize_array_header() {
        require(SerializationOperation::Span);
    }
    void Deserializer::deserialize_pair_header() {
        require(SerializationOperation::Pair);
    }
    void Deserializer::deserialize_tuple_header() {
        require(SerializationOperation::Tuple);
    }
    void Deserializer::deserialize_enum_header() {
        require(SerializationOperation::Enum);
    }
    void Deserializer::deserialize_impl(std::uint8_t &v) {
        require(SerializationOperation::Int8);
        read(&v, sizeof(v));
    }
    void Deserializer::deserialize_impl(std::uint16_t &v) {
        require(SerializationOperation::Int16);
        read(&v, sizeof(v));
    }
    void Deserializer::deserialize_impl(std::uint32_t &v) {
        require(SerializationOperation::Int32);
        read(&v, sizeof(v));
    }
    void Deserializer::deserialize_impl(std::uint64_t &v) {
        require(SerializationOperation::Int64);
        read(&v, sizeof(v));
    }

    void Deserializer::deserialize(std::uint8_t &v) {
        deserialize_impl(v);
    }
    void Deserializer::deserialize(std::uint16_t &v) {
        auto p = peek();
        if (p == SerializationOperation::Int8) {
            std::uint8_t t;
            deserialize_impl(t);
            v = t;
        }
        else {
            deserialize_impl(v);
        }
    }
    void Deserializer::deserialize(std::uint32_t &v) {
        auto p = peek();
        if (p == SerializationOperation::Int8) {
            std::uint8_t t;
            deserialize_impl(t);
            v = t;
        }
        else if (p == SerializationOperation::Int16) {
            std::uint16_t t;
            deserialize_impl(t);
            v = t;
        }
        else {
            deserialize_impl(v);
        }
    }
    void Deserializer::deserialize(std::uint64_t &v) {
        auto p = peek();
        if (p == SerializationOperation::Int8) {
            std::uint8_t t;
            deserialize_impl(t);
            v = t;
        }
        else if (p == SerializationOperation::Int16) {
            std::uint16_t t;
            deserialize_impl(t);
            v = t;
        }
        else if (p == SerializationOperation::Int32) {
            std::uint32_t t;
            deserialize_impl(t);
            v = t;
        }
        else {
            deserialize_impl(v);
        }
    }
    void Deserializer::deserialize(std::int8_t &v) {
        std::uint8_t t;
        deserialize_impl(t);
        v = (std::int8_t)t;
    }
    void Deserializer::deserialize(std::int16_t &v) {
        auto p = peek();
        if (p == SerializationOperation::Int8) {
            std::uint8_t t;
            deserialize_impl(t);
            v = (std::int8_t)t;
        }
        else {
            std::uint16_t t;
            deserialize_impl(t);
            v = (std::int16_t)t;
        }
    }
    void Deserializer::deserialize(std::int32_t &v) {
        auto p = peek();
        if (p == SerializationOperation::Int8) {
            std::uint8_t t;
            deserialize_impl(t);
            v = (std::int8_t)t;
        }
        else if (p == SerializationOperation::Int16) {
            std::uint16_t t;
            deserialize_impl(t);
            v = (std::int16_t)t;
        }
        else {
            std::uint32_t t;
            deserialize_impl(t);
            v = (std::int32_t)t;
        }
    }
    void Deserializer::deserialize(std::int64_t &v) {
        auto p = peek();
        if (p == SerializationOperation::Int8) {
            std::uint8_t t;
            deserialize_impl(t);
            v = (std::int8_t)t;
        }
        else if (p == SerializationOperation::Int16) {
            std::uint16_t t;
            deserialize_impl(t);
            v = (std::int16_t)t;
        }
        else if (p == SerializationOperation::Int32) {
            std::uint32_t t;
            deserialize_impl(t);
            v = (std::int32_t)t;
        }
        else {
            std::uint64_t t;
            deserialize_impl(t);
            v = (std::int64_t)t;
        }
    }
    void Deserializer::deserialize(float &v) {
        require(SerializationOperation::Float32);
        read(&v, sizeof(v));
    }
    void Deserializer::deserialize(double &v) {
        require(SerializationOperation::Float64);
        read(&v, sizeof(v));
    }
    void Deserializer::deserialize(bool &v) {
        if (peek() == SerializationOperation::True) {
            require(SerializationOperation::True);
            v = true;
        }
        else {
            require(SerializationOperation::False);
            v = false;
        }
    }
    void Deserializer::deserialize(std::string_view &v) {
        require(SerializationOperation::String);
        size_t size;
        deserialize(size);
        if (size == 0) {
            v = {};
        }
        else {
            auto p = allocate_raw(size + 1, 1);
            read(p, size);
            ((char*)p)[size] = 0;
            v = { (char*)p, size };
        }
    }
    void Deserializer::deserialize(ImplExpr *&v) {
        if (try_require(SerializationOperation::Null)) {
            v = nullptr;
            return;
        }
        require(SerializationOperation::ImplExpr);
        ImplExpr::Kind kind;
        unsigned int line;

        deserialize(kind);
        deserialize(line);

        auto p = allocate_expr((unsigned int)kind, Line{ line });
        p->load_members(*this);
        v = p;
    }
    void Deserializer::deserialize(ImplStat *&v) {
        if (try_require(SerializationOperation::Null)) {
            v = nullptr;
            return;
        }
        require(SerializationOperation::ImplStat);
        ImplStat::Kind kind;
        unsigned int line;

        deserialize(kind);
        deserialize(line);

        auto p = allocate_stat((unsigned int)kind, Line{ line });
        p->load_members(*this);
        v = p;
    }

    ImplStat::Task::Yield::Yield(OwcaVM vm, State &st, OwcaValue val) : st(st)
    {
        VM::get(vm).set_yield_value(val);
    }

    bool ImplStat::Task::Yield::await_suspend(std::coroutine_handle<promise_type> h) noexcept
    {
        if constexpr (debug_print) std::cout << "Awaiter::await_suspend(" << (void*)this << ") storing to " << ((char*)st.top() - st.storage.data()) <<"\n";
        st.top()->h = h;
        return true;
    }

    ImplStat::Task *ImplStat::State::top() {
        assert(storage.size() >= sizeof(Task));
        return (Task*)((char*)storage.data() + storage.size() - sizeof(Task));
    }

    void ImplStat::execute_statement(OwcaVM vm) const
    {
        VM::get(vm).update_execution_line(line);
        auto pp = VM::AllocatedObjectsPointer{ VM::get(vm) };
        return execute_statement_impl(vm);
    }

    ImplStat::Task ImplStat::execute_generator_statement(OwcaVM vm, State &st) const
    {
        return execute_generator_statement_impl(vm, st);
    }

    OwcaValue ImplExpr::execute_expression(OwcaVM vm) const
    {
        VM::get(vm).update_execution_line(line);
        return execute_expression_impl(vm);
    }

    size_t ImplStat::calculate_generator_object_size_for_this() const
    {
        size_t sz;
        {
            auto state = State{ 4096 };
            auto t = execute_generator_statement((Internal::VM*)nullptr, state);
            sz = state.storage.size();
        }
        return sz;
    }
}
