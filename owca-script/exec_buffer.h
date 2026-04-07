#ifndef RC_OWCA_SCRIPT_EXEC_BUFFER_H
#define RC_OWCA_SCRIPT_EXEC_BUFFER_H

#include "stdafx.h"
#include "line.h"

namespace OwcaScript {
	namespace Internal {
        template <typename T> struct IsSpan {
            static constexpr bool value = false;
        };
        template <typename T> struct IsSpan<std::span<T>> {
            static constexpr bool value = true;
            using type = T;
        };
        template <typename T> concept Span = IsSpan<T>::value;
        template <typename T> struct IsVector {
            static constexpr bool value = false;
        };
        template <typename T> struct IsVector<std::vector<T>> {
            static constexpr bool value = true;
            using type = T;
        };
        template <typename T> concept Vector = IsVector<T>::value;

        enum class ExecuteOp : std::uint8_t {
            Class,
            ExprPopAndIgnore,
            ExprCompareEq,
            ExprCompareNotEq,
            ExprCompareLessEq,
            ExprCompareMoreEq,
            ExprCompareLess,
            ExprCompareMore,
            ExprCompareIs,
            ExprConstantEmpty,
            ExprConstantBool,
            ExprConstantFloat,
            ExprConstantString,
            ExprConstantStringInterpolated,
            ExprIdentifierRead,
            ExprIdentifierWrite,
            ExprInterpretedString,
            ExprMemberRead,
            ExprMemberWrite,
            ExprOper1BinNeg,
            ExprOper1LogNot,
            ExprOper1Negate,
            ExprOper2LogOr,
            ExprOper2LogAnd,
            ExprOper2BinOr,
            ExprOper2BinAnd,
            ExprOper2BinXor,
            ExprOper2BinLShift,
            ExprOper2BinRShift,
            ExprOper2Add,
            ExprOper2Sub,
            ExprOper2Mul,
            ExprOper2Div,
            ExprOper2Mod,
            ExprOper2MakeRange,
            ExprOper2IndexRead,
            ExprOper2IndexWrite,
            ExprOperXCall,
            ExprOperXCreateArray,
            ExprOperXCreateTuple,
            ExprOperXCreateSet,
            ExprOperXCreateMap,
            ForInit,
            ForNext,
            ForCondition,
            ForCompleted,
            Function,
            If,
            LoopControlBreak,
            LoopControlContinue,
            Return,
            ReturnValue,
            Throw,
            Try,
            TryCompleted,
            TryCatchTypeDone,
            WhileInit,
            WhileNext,
            WhileCondition,
            WhileCompleted,
            WithInit,
            WithCompleted,
            Yield,
            Jump,
        };

        class ExecuteBufferReader {
        public:
            struct LineEntry {
                std::uint32_t code_pos;
                std::uint32_t line;

                bool operator <(const LineEntry &other) const {
                    return code_pos < other.code_pos;
                }
            };
            enum class DataKind {
                Other,
                Op,
                Enum,
                Bool,
                Int8,
                Int16,
                Int32,
                Int64,
                Size,
                Float32,
                Float64,
                Blob,
            };
            using Op = ExecuteOp;

            ExecuteBufferReader(std::span<const unsigned char> code, std::span<const DataKind> data_kinds, std::span<const LineEntry> lines) : code(code), data_kinds(data_kinds), lines(lines), pos(0) {}

            template <typename T> T decode() requires(std::is_same_v<T, std::string_view>) {
                auto size = decode_size();
                ensure_data_kind(DataKind::Blob, pos);
                auto p = align_pos(alignof(char), sizeof(char) * size);
                return std::string_view((const char*)(code.data() + p), size);
            }
            template <typename T> T decode() requires(std::is_enum_v<T>) {
                static_assert(sizeof(T) <= sizeof(std::uint64_t), "Enum type too large to decode");
                auto p = align_pos(alignof(std::underlying_type_t<T>), sizeof(std::underlying_type_t<T>));
                ensure_data_kind(std::is_same_v<T, Op> ? DataKind::Op : DataKind::Enum, p);
                T t;
                std::memcpy(&t, code.data() + p, sizeof(T));
                return static_cast<T>(t);
            }
            template <typename T> T decode() requires(std::is_integral_v<T> && !std::is_enum_v<T>) {
                static_assert(sizeof(T) <= sizeof(std::uint64_t), "Integral type too large to decode");
                auto p = align_pos(alignof(T), sizeof(T));
                if (std::is_same_v<T, bool>) {
                    ensure_data_kind(DataKind::Bool, p);
                } else if (sizeof(T) == 1) {
                    ensure_data_kind(DataKind::Int8, p);
                } else if (sizeof(T) == 2) {
                    ensure_data_kind(DataKind::Int16, p);
                } else if (sizeof(T) == 4) {
                    ensure_data_kind(DataKind::Int32, p);
                } else if (sizeof(T) == 8) {
                    ensure_data_kind(DataKind::Int64, p);
                }
                T t;
                std::memcpy(&t, code.data() + p, sizeof(T));
                return static_cast<T>(t);
            }
            template <typename T> T decode() requires(std::is_floating_point_v<T> && !std::is_enum_v<T>) {
                static_assert(sizeof(T) <= sizeof(std::uint64_t), "Floating point type too large to decode");
                auto p = align_pos(alignof(T), sizeof(T));
                if (sizeof(T) == 4) {
                    ensure_data_kind(DataKind::Float32, p);
                } else if (sizeof(T) == 8) {
                    ensure_data_kind(DataKind::Float64, p);
                }
                T t;
                std::memcpy(&t, code.data() + p, sizeof(T));
                return static_cast<T>(t);
            }
            template <typename T> T decode() requires(Span<T>) {
                auto sz = decode<std::uint32_t>();
                auto p = align_pos(alignof(typename IsSpan<T>::type), sizeof(typename IsSpan<T>::type) * sz);
                ensure_data_kind(DataKind::Blob, p);
                return T((typename IsSpan<T>::type*)(code.data() + p), sz);
            }
            template <typename T> T decode() {
                return T{ *this };
            }
        private:
            size_t decode_size() {
                auto p = align_pos(alignof(std::uint32_t), sizeof(std::uint32_t));
                ensure_data_kind(DataKind::Size, p);
                std::uint32_t size;
                std::memcpy(&size, code.data() + p, sizeof(size));
                return size;
            }
            void ensure_data_kind(DataKind expected, size_t p) {
                if (!data_kinds.empty() && data_kinds[p] != expected) {
                    throw std::runtime_error(std::format("Expected data kind {} at position {}, but got {}", static_cast<int>(expected), p, static_cast<int>(data_kinds[p])));
                }
            }
            size_t align_pos(size_t align, size_t size) {
                pos += (align - (pos % align)) % align;
                auto p = pos;
                pos += size;
                return p;
            }
            std::span<const unsigned char> code;
            std::span<const DataKind> data_kinds;
            std::span<const LineEntry> lines;
            size_t pos;
        };

        class ExecuteBufferWriter {
            std::vector<unsigned char> buffer;
            std::vector<ExecuteBufferReader::DataKind> data_kinds;
            std::vector<ExecuteBufferReader::LineEntry> lines;

            template <typename T> std::uint32_t prepare(const T *data, size_t sz, ExecuteBufferReader::DataKind kind) {
                auto align = alignof(T);
                auto size = sizeof(T) * sz;
                auto current_size = buffer.size();
                auto padding = (align - (current_size % align)) % align;
                buffer.resize(current_size + padding + size);
                data_kinds.resize(buffer.size(), ExecuteBufferReader::DataKind::Other);
                data_kinds[current_size + padding] = kind;
                return current_size + padding;
            }
            template <typename T> void append_impl_vec(const T *data, size_t sz) {
                auto pos = prepare(data, sz, ExecuteBufferReader::DataKind::Blob);
                std::memcpy(buffer.data() + pos, data, sizeof(T) * sz);
            }
            template <typename T> void append_impl(T value, ExecuteBufferReader::DataKind kind) {
                auto pos = prepare(&value, 1, kind);
                std::memcpy(buffer.data() + pos, &value, sizeof(T));
            }
            void handle_line(Line line) {
                if (lines.empty() || lines.back().line != line.line) {
                    lines.emplace_back(ExecuteBufferReader::LineEntry{static_cast<std::uint32_t>(buffer.size()), line.line});
                }
            }
        public:
            ExecuteBufferWriter() = default;

            Line current_line() const {
                if (lines.empty()) return Line{ 0 };
                return Line{ lines.back().line };
            }
            auto take() && {
                return std::make_tuple(std::move(buffer), std::move(data_kinds), std::move(lines));
            }
            template <typename T> void append(Line line, T value) requires(std::is_enum_v<T>) {
                handle_line(line);
                append_impl(value, std::is_same_v<T, ExecuteBufferReader::Op> ? ExecuteBufferReader::DataKind::Op : ExecuteBufferReader::DataKind::Enum);
            }
            template <typename T> void append(Line line, T value) requires(std::is_integral_v<T> && !std::is_enum_v<T>) {
                handle_line(line);
                ExecuteBufferReader::DataKind kind;
                if constexpr (std::is_same_v<T, bool>) kind = ExecuteBufferReader::DataKind::Bool;
                else if constexpr (std::is_same_v<T, std::int8_t> || std::is_same_v<T, std::uint8_t>) kind = ExecuteBufferReader::DataKind::Int8;
                else if constexpr (std::is_same_v<T, std::int16_t> || std::is_same_v<T, std::uint16_t>) kind = ExecuteBufferReader::DataKind::Int16;
                else if constexpr (std::is_same_v<T, std::int32_t> || std::is_same_v<T, std::uint32_t>) kind = ExecuteBufferReader::DataKind::Int32;
                else if constexpr (std::is_same_v<T, std::int64_t> || std::is_same_v<T, std::uint64_t>) kind = ExecuteBufferReader::DataKind::Int64;
                else {
                    static_assert(sizeof(T) == 0, "Unsupported integral type");
                }
                append_impl(value, kind);
            }
            template <typename T> void append(Line line, T value) requires(std::is_floating_point_v<T>) {
                handle_line(line);
                ExecuteBufferReader::DataKind kind;
                if constexpr (std::is_same_v<T, float>) kind = ExecuteBufferReader::DataKind::Float32;
                else if constexpr (std::is_same_v<T, double>) kind = ExecuteBufferReader::DataKind::Float64;
                else {
                    static_assert(sizeof(T) == 0, "Unsupported floating point type");
                }
                append_impl(value, kind);
            }
            void append(Line line, const char *str) {
                auto sz = strlen(str);
                append(line, (std::uint32_t)sz);
                append_impl_vec(str, sz);
            }
            void append(Line line, std::string_view str) {
                append(line, (std::uint32_t)str.size());
                append_impl_vec(str.data(), str.size());
            }
            void append(Line line, const std::string &str) {
                append(line, (std::uint32_t)str.size());
                append_impl_vec(str.data(), str.size());
            }
            template <typename T> void append(Line line, const std::vector<T> &vec) {
                append(line, (std::uint32_t)vec.size());
                for(auto &v : vec) {
                    append(line, v);
                }
            }
            template <typename T> void append(Line line, const T &t) requires (!std::is_enum_v<T> && !std::is_integral_v<T> && !std::is_floating_point_v<T> && !std::is_same_v<T, std::string_view> && !Span<T> && !Vector<T>) {
                serialize_object(*this, line, t);
            }

            std::uint32_t position() const { return buffer.size(); }

            template <typename T> struct Placeholder {
                const std::uint32_t pos;
            };

            template <typename T> Placeholder<T> append_placeholder(Line line) {
                ExecuteBufferReader::DataKind kind;
                if constexpr (std::is_same_v<T, bool>) kind = ExecuteBufferReader::DataKind::Bool;
                else if constexpr (std::is_same_v<T, std::int8_t> || std::is_same_v<T, std::uint8_t>) kind = ExecuteBufferReader::DataKind::Int8;
                else if constexpr (std::is_same_v<T, std::int16_t> || std::is_same_v<T, std::uint16_t>) kind = ExecuteBufferReader::DataKind::Int16;
                else if constexpr (std::is_same_v<T, std::int32_t> || std::is_same_v<T, std::uint32_t>) kind = ExecuteBufferReader::DataKind::Int32;
                else if constexpr (std::is_same_v<T, std::int64_t> || std::is_same_v<T, std::uint64_t>) kind = ExecuteBufferReader::DataKind::Int64;
                else {
                    static_assert(sizeof(T) == 0, "Unsupported integral type");
                }
                handle_line(line);
                auto pos = prepare((T*)nullptr, 1, kind);
                return Placeholder<T>{ pos };
            }
            template <typename T> void update_placeholder(const Placeholder<T> &placeholder, T value) {
                assert(placeholder.pos + sizeof(T) <= buffer.size());
                std::memcpy(buffer.data() + placeholder.pos, &value, sizeof(T));
            }
        };

	}
}

#endif
