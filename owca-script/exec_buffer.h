#ifndef RC_OWCA_SCRIPT_EXEC_BUFFER_H
#define RC_OWCA_SCRIPT_EXEC_BUFFER_H

#include "stdafx.h"
#include "line.h"
#include "owca_code.h"

//#define OWCA_SCRIPT_EXEC_LOG

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

        enum class DataKind : std::uint8_t{
            Unfilled,
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
        inline std::string_view to_string(DataKind kind) {
            switch(kind) {
            case DataKind::Unfilled: return "Unfilled";
            case DataKind::Other: return "Other";
            case DataKind::Op: return "Op";
            case DataKind::Enum: return "Enum";
            case DataKind::Bool: return "Bool";
            case DataKind::Int8: return "Int8";
            case DataKind::Int16: return "Int16";
            case DataKind::Int32: return "Int32";
            case DataKind::Int64: return "Int64";
            case DataKind::Size: return "Size";
            case DataKind::Float32: return "Float32";
            case DataKind::Float64: return "Float64";
            case DataKind::Blob: return "Blob";
            default: return "Unknown";
            }
        }
        enum class ExecuteOp : std::uint8_t {
            ClassInit,
            ClassCreate,
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
            ExprIdentifierFunctionWrite,
            ExprMemberRead,
            ExprMemberWrite,
            ExprOper1BinNeg,
            ExprOper1LogNot,
            ExprOper1Negate,
            ExprRetTrueAndJumpIfTrue,
            ExprRetFalseAndJumpIfFalse,
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
            ExprToString,
            ExprToIterator,
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
            TryInit,
            TryCompleted,
            TryBlockCompleted,
            TryCatchType,
            TryCatchTypeCompleted,
            WhileInit,
            WhileCondition,
            WhileNext,
            WhileCompleted,
            WithInit,
            WithInitPrepare,
            WithCompleted,
            Yield,
            Jump,
        };
        inline std::string_view to_string(ExecuteOp op) {
            switch(op) {
            case ExecuteOp::ClassInit: return "ClassInit";
            case ExecuteOp::ClassCreate: return "ClassCreate";
            case ExecuteOp::ExprPopAndIgnore: return "ExprPopAndIgnore";
            case ExecuteOp::ExprCompareEq: return "ExprCompareEq";
            case ExecuteOp::ExprCompareNotEq: return "ExprCompareNotEq";
            case ExecuteOp::ExprCompareLessEq: return "ExprCompareLessEq";
            case ExecuteOp::ExprCompareMoreEq: return "ExprCompareMoreEq";
            case ExecuteOp::ExprCompareLess: return "ExprCompareLess";
            case ExecuteOp::ExprCompareMore: return "ExprCompareMore";
            case ExecuteOp::ExprCompareIs: return "ExprCompareIs";
            case ExecuteOp::ExprConstantEmpty: return "ExprConstantEmpty";
            case ExecuteOp::ExprConstantBool: return "ExprConstantBool";
            case ExecuteOp::ExprConstantFloat: return "ExprConstantFloat";
            case ExecuteOp::ExprConstantString: return "ExprConstantString";
            case ExecuteOp::ExprConstantStringInterpolated: return "ExprConstantStringInterpolated";
            case ExecuteOp::ExprIdentifierRead: return "ExprIdentifierRead";
            case ExecuteOp::ExprIdentifierWrite: return "ExprIdentifierWrite";
            case ExecuteOp::ExprIdentifierFunctionWrite: return "ExprIdentifierFunctionWrite";
            case ExecuteOp::ExprMemberRead: return "ExprMemberRead";
            case ExecuteOp::ExprMemberWrite: return "ExprMemberWrite";
            case ExecuteOp::ExprOper1BinNeg: return "ExprOper1BinNeg";
            case ExecuteOp::ExprOper1LogNot: return "ExprOper1LogNot";
            case ExecuteOp::ExprOper1Negate: return "ExprOper1Negate";
            case ExecuteOp::ExprRetTrueAndJumpIfTrue: return "ExprRetTrueAndJumpIfTrue";
            case ExecuteOp::ExprRetFalseAndJumpIfFalse: return "ExprRetFalseAndJumpIfFalse";
            case ExecuteOp::ExprOper2BinOr: return "ExprOper2BinOr";
            case ExecuteOp::ExprOper2BinAnd: return "ExprOper2BinAnd";
            case ExecuteOp::ExprOper2BinXor: return "ExprOper2BinXor";
            case ExecuteOp::ExprOper2BinLShift: return "ExprOper2BinLShift";
            case ExecuteOp::ExprOper2BinRShift: return "ExprOper2BinRShift";
            case ExecuteOp::ExprOper2Add: return "ExprOper2Add";
            case ExecuteOp::ExprOper2Sub: return "ExprOper2Sub";
            case ExecuteOp::ExprOper2Mul: return "ExprOper2Mul";
            case ExecuteOp::ExprOper2Div: return "ExprOper2Div";
            case ExecuteOp::ExprOper2Mod: return "ExprOper2Mod";
            case ExecuteOp::ExprOper2MakeRange: return "ExprOper2MakeRange";
            case ExecuteOp::ExprOper2IndexRead: return "ExprOper2IndexRead";
            case ExecuteOp::ExprOper2IndexWrite: return "ExprOper2IndexWrite";
            case ExecuteOp::ExprOperXCall: return "ExprOperXCall";
            case ExecuteOp::ExprOperXCreateArray: return "ExprOperXCreateArray";
            case ExecuteOp::ExprOperXCreateTuple: return "ExprOperXCreateTuple";
            case ExecuteOp::ExprOperXCreateSet: return "ExprOperXCreateSet";
            case ExecuteOp::ExprOperXCreateMap: return "ExprOperXCreateMap";
            case ExecuteOp::ExprToString: return "ExprToString";
            case ExecuteOp::ExprToIterator: return "ExprToIterator";
            case ExecuteOp::ForInit: return "ForInit";
            case ExecuteOp::ForNext: return "ForNext";
            case ExecuteOp::ForCondition: return "ForCondition";
            case ExecuteOp::ForCompleted: return "ForCompleted";
            case ExecuteOp::Function: return "Function";
            case ExecuteOp::If: return "If";
            case ExecuteOp::LoopControlBreak: return "LoopControlBreak";
            case ExecuteOp::LoopControlContinue: return "LoopControlContinue";
            case ExecuteOp::Return: return "Return";
            case ExecuteOp::ReturnValue: return "ReturnValue";
            case ExecuteOp::Throw: return "Throw";
            case ExecuteOp::TryInit: return "TryInit";
            case ExecuteOp::TryCompleted: return "TryCompleted";
            case ExecuteOp::TryBlockCompleted: return "TryBlockCompleted";
            case ExecuteOp::TryCatchType: return "TryCatchType";
            case ExecuteOp::TryCatchTypeCompleted: return "TryCatchTypeCompleted";
            case ExecuteOp::WhileInit: return "WhileInit";
            case ExecuteOp::WhileCondition: return "WhileCondition";
            case ExecuteOp::WhileNext: return "WhileNext";
            case ExecuteOp::WhileCompleted: return "WhileCompleted";
            case ExecuteOp::WithInit: return "WithInit";
            case ExecuteOp::WithInitPrepare: return "WithInitPrepare";
            case ExecuteOp::WithCompleted: return "WithCompleted";
            case ExecuteOp::Yield: return "Yield";
            case ExecuteOp::Jump: return "Jump";
            default: return "Unknown";
            }
        }
        struct LineEntry {
            std::uint32_t code_pos;
            std::uint32_t line;

            bool operator <(const LineEntry &other) const {
                return code_pos < other.code_pos;
            }
        };

        class ExecuteBufferReader {
        public:
            using Op = ExecuteOp;

            ExecuteBufferReader(OwcaCode code, size_t pos);

            std::uint32_t position() const { return pos; }
            void jump(std::uint32_t new_pos) { pos = new_pos; }
            Line line_from_code_pos(std::uint32_t pos) const;
            const auto &code() const { return code_object_; }

            template <typename T> T decode() requires(std::is_same_v<T, std::string_view>) {
                auto size = decode_size();
                if (size > 0) ensure_data_kind(DataKind::Blob, pos);
                auto p = align_pos(alignof(char), sizeof(char) * size);
                return std::string_view((const char*)(code_.data() + p), size);
            }
            template <typename T> T decode() requires(std::is_enum_v<T>) {
                static_assert(sizeof(T) <= sizeof(std::uint64_t), "Enum type too large to decode");
                auto p = align_pos(alignof(std::underlying_type_t<T>), sizeof(std::underlying_type_t<T>));
                ensure_data_kind(std::is_same_v<T, Op> ? DataKind::Op : DataKind::Enum, p);
                T t;
                std::memcpy(&t, code_.data() + p, sizeof(T));
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
                std::memcpy(&t, code_.data() + p, sizeof(T));
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
                std::memcpy(&t, code_.data() + p, sizeof(T));
                return static_cast<T>(t);
            }
            template <typename T> void decode_span_helper(const auto &cb) {
                auto sz = decode_size();
                for(auto i = 0u; i < sz; ++i) {
                    auto v = decode<T>();
                    cb(i, sz, v);
                }
            }
            template <typename T> T decode() {
                return T{ *this };
            }
        private:
            size_t decode_size() {
                auto p = align_pos(alignof(std::uint32_t), sizeof(std::uint32_t));
                ensure_data_kind(DataKind::Size, p);
                std::uint32_t size;
                std::memcpy(&size, code_.data() + p, sizeof(size));
                return size;
            }
            void ensure_data_kind(DataKind expected, size_t p) {
                if (!data_kinds.empty() && data_kinds[p] != expected) {
                    auto msg = std::format("Data kind mismatch at position {}: expected {}, got {}", p, to_string(expected), to_string(data_kinds[p]));
                    std::cout << msg << std::endl;
                    throw std::runtime_error(msg);
                }
            }
            size_t align_pos(size_t align, size_t size) {
                pos += (align - (pos % align)) % align;
                auto p = pos;
                pos += size;
                return p;
            }
            OwcaCode code_object_;
            std::span<const unsigned char> code_;
            std::span<const DataKind> data_kinds;
            std::span<const LineEntry> lines;
            size_t pos;
        };

        class ExecuteBufferWriter {
            std::vector<unsigned char> buffer;
            std::vector<DataKind> data_kinds;
            std::vector<LineEntry> lines;

            template <typename T> std::uint32_t prepare(const T *data, size_t sz, DataKind kind) {
                auto align = alignof(T);
                auto size = sizeof(T) * sz;
                auto current_size = buffer.size();
                auto padding = (align - (current_size % align)) % align;
                buffer.resize(current_size + padding + size);
                data_kinds.resize(buffer.size(), DataKind::Unfilled);
                data_kinds[current_size + padding] = kind;
                return current_size + padding;
            }
            template <typename T> void append_impl_vec(const T *data, size_t sz) {
                if (sz == 0) return;
                auto pos = prepare(data, sz, DataKind::Blob);
                std::memcpy(buffer.data() + pos, data, sizeof(T) * sz);
            }
            template <typename T> void append_impl(Line line, T value, DataKind kind) {
                handle_line(line);
                auto pos = prepare(&value, 1, kind);
                std::memcpy(buffer.data() + pos, &value, sizeof(T));
#ifdef OWCA_SCRIPT_EXEC_LOG                
                if (kind == DataKind::Op) {
                    std::cout << "Writing data of kind " << to_string(kind) << " at line " << line.line << " position " << (pos) << " oper " << to_string((ExecuteOp)buffer[pos]) << std::endl;
                }
                else {
                    std::cout << "Writing data of kind " << to_string(kind) << " at line " << line.line << " position " << (pos) << std::endl;
                }
#endif
            }
            void handle_line(Line line) {
                if (lines.empty() || lines.back().line != line.line) {
                    lines.emplace_back(LineEntry{static_cast<std::uint32_t>(buffer.size()), line.line});
                }
            }
            void append_size(Line line, size_t size) {
                append_impl(line, (std::uint32_t)size, DataKind::Size);
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
                append_impl(line, value, std::is_same_v<T, ExecuteBufferReader::Op> ? DataKind::Op : DataKind::Enum);
            }
            template <typename T> void append(Line line, T value) requires(std::is_integral_v<T> && !std::is_enum_v<T>) {
                DataKind kind;
                if constexpr (std::is_same_v<T, bool>) kind = DataKind::Bool;
                else if constexpr (std::is_same_v<T, std::int8_t> || std::is_same_v<T, std::uint8_t>) kind = DataKind::Int8;
                else if constexpr (std::is_same_v<T, std::int16_t> || std::is_same_v<T, std::uint16_t>) kind = DataKind::Int16;
                else if constexpr (std::is_same_v<T, std::int32_t> || std::is_same_v<T, std::uint32_t>) kind = DataKind::Int32;
                else if constexpr (std::is_same_v<T, std::int64_t> || std::is_same_v<T, std::uint64_t>) kind = DataKind::Int64;
                else {
                    static_assert(sizeof(T) == 0, "Unsupported integral type");
                }
                append_impl(line, value, kind);
            }
            template <typename T> void append(Line line, T value) requires(std::is_floating_point_v<T>) {
                DataKind kind;
                if constexpr (std::is_same_v<T, float>) kind = DataKind::Float32;
                else if constexpr (std::is_same_v<T, double>) kind = DataKind::Float64;
                else {
                    static_assert(sizeof(T) == 0, "Unsupported floating point type");
                }
                append_impl(line, value, kind);
            }
            void append(Line line, const char *str) {
                auto sz = strlen(str);
                append_size(line, sz);
                append_impl_vec(str, sz);
            }
            void append(Line line, std::string_view str) {
                append_size(line, str.size());
                append_impl_vec(str.data(), str.size());
            }
            void append(Line line, const std::string &str) {
                append_size(line, str.size());
                append_impl_vec(str.data(), str.size());
            }
            template <typename T> void append(Line line, const T &t) requires (!std::is_enum_v<T> && !std::is_integral_v<T> && !std::is_floating_point_v<T> && !std::is_same_v<T, std::string_view> && !Span<T> && !Vector<T>) {
                serialize_object(*this, line, t);
            }
            template <typename T> void append_span_helper(Line line, std::span<T> vec) {
                append_size(line, vec.size());
                for (const auto &item : vec) {
                    append(line, item);
                }
            }
            template <typename T> void append_span_helper(Line line, const std::vector<T> &vec) {
                append_span_helper(line, std::span<const T>(vec.data(), vec.size()));
            }
            std::uint32_t position() const { return buffer.size(); }

            template <typename T> struct Placeholder {
                const std::uint32_t pos;
            };

            template <typename T> Placeholder<T> append_placeholder(Line line) {
                DataKind kind;
                if constexpr (std::is_same_v<T, bool>) kind = DataKind::Bool;
                else if constexpr (std::is_same_v<T, std::int8_t> || std::is_same_v<T, std::uint8_t>) kind = DataKind::Int8;
                else if constexpr (std::is_same_v<T, std::int16_t> || std::is_same_v<T, std::uint16_t>) kind = DataKind::Int16;
                else if constexpr (std::is_same_v<T, std::int32_t> || std::is_same_v<T, std::uint32_t>) kind = DataKind::Int32;
                else if constexpr (std::is_same_v<T, std::int64_t> || std::is_same_v<T, std::uint64_t>) kind = DataKind::Int64;
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
