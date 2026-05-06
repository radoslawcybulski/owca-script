#ifndef RC_OWCA_SCRIPT_EXEC_BUFFER_H
#define RC_OWCA_SCRIPT_EXEC_BUFFER_H

#include "stdafx.h"
#include "line.h"
#include "owca_code.h"
#include <unordered_map>

#ifdef DEBUG
//#define OWCA_SCRIPT_EXEC_LOG
#endif

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
            JumpOffset,
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
            case DataKind::JumpOffset: return "JumpOffset";
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
            ExprGlobalRead,
            ExprGlobalWrite,
            ExprGlobalFunctionWrite,
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
            ReturnCloseIterator,
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
            WithCompleted,
            Yield,
            Jump,
            _Count
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
            case ExecuteOp::ExprGlobalRead: return "ExprGlobalRead";
            case ExecuteOp::ExprGlobalWrite: return "ExprGlobalWrite";
            case ExecuteOp::ExprGlobalFunctionWrite: return "ExprGlobalFunctionWrite";
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
            case ExecuteOp::ReturnCloseIterator: return "ReturnCloseIterator";
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

        class CodePosition {
            const unsigned char *pos = nullptr;
        public:

            CodePosition() = default;
            explicit CodePosition(const unsigned char *pos) : pos(pos) {}

            auto value() const { return pos; }
            CodePosition operator + (std::int32_t offset) const {
                return CodePosition(pos + offset);
            }
            CodePosition operator - (std::int32_t offset) const {
                return CodePosition(pos - offset);
            }
        };
        class StartOfCode {
        public:
            const unsigned char *operator +(CodePosition pos) const {
                return pos.value();
            }
        };
        class ExecuteBufferReader {
        public:
            using Position = CodePosition;
            using DataKindsType = std::unordered_map<const unsigned char *, DataKind>;
            using Op = ExecuteOp;

            template <typename T> static T decode(StartOfCode code, Position &pos, const DataKindsType & data_kinds) requires(std::is_same_v<T, std::string_view>) {
                auto size = decode_size(code, pos, data_kinds);
                auto p = align_pos(pos, alignof(char), sizeof(char) * size);
#ifdef DEBUG                
                if (size > 0) ensure_data_kind(data_kinds, DataKind::Blob, p);
#endif
                return std::string_view((const char*)(code + p), size);
            }
            template <typename T> static T decode(StartOfCode code, Position &pos, const DataKindsType & data_kinds) requires(std::is_enum_v<T>) {
                static_assert(sizeof(T) <= sizeof(std::uint64_t), "Enum type too large to decode");
                auto p = align_pos(pos, alignof(std::underlying_type_t<T>), sizeof(std::underlying_type_t<T>));
#ifdef DEBUG                
                ensure_data_kind(data_kinds, std::is_same_v<T, Op> ? DataKind::Op : DataKind::Enum, p);
#endif
                T t;
                std::memcpy(&t, code + p, sizeof(T));
                return static_cast<T>(t);
            }
            template <typename T> static T decode(StartOfCode code, Position &pos, const DataKindsType & data_kinds) requires(std::is_integral_v<T> && !std::is_enum_v<T>) {
                static_assert(sizeof(T) <= sizeof(std::uint64_t), "Integral type too large to decode");
                auto p = align_pos(pos, alignof(T), sizeof(T));
#ifdef DEBUG
                if constexpr (std::is_same_v<T, bool>) {
                    ensure_data_kind(data_kinds, DataKind::Bool, p);
                } else if constexpr (sizeof(T) == 1) {
                    ensure_data_kind(data_kinds, DataKind::Int8, p);
                } else if constexpr (sizeof(T) == 2) {
                    ensure_data_kind(data_kinds, DataKind::Int16, p);
                } else if constexpr (sizeof(T) == 4) {
                    ensure_data_kind(data_kinds, DataKind::Int32, p);
                } else if constexpr (sizeof(T) == 8) {
                    ensure_data_kind(data_kinds, DataKind::Int64, p);
                } else {
                    static_assert(sizeof(T) == 0, "Unsupported integral type");
                }
#endif
                T t;
                std::memcpy(&t, code + p, sizeof(T));
                return static_cast<T>(t);
            }
            template <typename T> static T decode(StartOfCode code, Position &pos, const DataKindsType & data_kinds) requires(std::is_floating_point_v<T> && !std::is_enum_v<T>) {
                static_assert(sizeof(T) <= sizeof(std::uint64_t), "Floating point type too large to decode");
                auto p = align_pos(pos, alignof(T), sizeof(T));
#ifdef DEBUG
                if constexpr (sizeof(T) == 4) {
                    ensure_data_kind(data_kinds, DataKind::Float32, p);
                } else if constexpr (sizeof(T) == 8) {
                    ensure_data_kind(data_kinds, DataKind::Float64, p);
                } else {
                    static_assert(sizeof(T) == 0, "Unsupported floating point type");
                }
#endif
                T t;
                std::memcpy(&t, code + p, sizeof(T));
                return static_cast<T>(t);
            }
            static Position decode_jump(StartOfCode code, Position &pos, const DataKindsType & data_kinds) {
                auto p = align_pos(pos, alignof(std::int32_t), sizeof(std::int32_t));
#ifdef DEBUG
                ensure_data_kind(data_kinds, DataKind::JumpOffset, p);
#endif
                std::int32_t offset;
                std::memcpy(&offset, code + p, sizeof(offset));
                return pos + offset;
            }
        private:
            static size_t decode_size(StartOfCode code, Position &pos, const DataKindsType & data_kinds) {
                auto p = align_pos(pos, alignof(std::uint32_t), sizeof(std::uint32_t));
#ifdef DEBUG
                ensure_data_kind(data_kinds, DataKind::Size, p);
#endif
                std::uint32_t size;
                std::memcpy(&size, code + p, sizeof(size));
                return size;
            }
#ifdef DEBUG
            static void ensure_data_kind(const DataKindsType & data_kinds, DataKind expected, Position p) {
                if (!data_kinds.empty()) {
                    auto it = data_kinds.find(p.value());
                    if (it == data_kinds.end() || it->second != expected) {
                        auto msg = std::format("Data kind mismatch at position {}: expected {}, got {}", (void*)p.value(), to_string(expected), it == data_kinds.end() ? "Unknown" : to_string(it->second));
                        std::cout << msg << std::endl;
                        throw std::runtime_error(msg);
                    }
                }
            }
#endif
            static Position align_pos(Position &pos, size_t align, size_t size) {
                auto p = pos;
                pos = pos + size;
                return p;
            }
        };

        class ExecuteBufferWriter {
            std::vector<unsigned char> buffer;
            using DataKindsType = std::unordered_map<size_t, DataKind>;
            DataKindsType data_kinds;
            std::vector<LineEntry> lines;

            template <typename T> std::uint32_t prepare(const T *data, size_t sz, DataKind kind) {
                auto align = alignof(T);
                auto size = sizeof(T) * sz;
                auto current_size = buffer.size();
                auto padding = 0u;
                buffer.resize(current_size + padding + size);
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
                ExecuteBufferReader::DataKindsType data_kinds_converted;
#ifdef DEBUG
                for(auto &entry : data_kinds) {
                    data_kinds_converted[buffer.data() + entry.first] = entry.second;
                }
#endif
                return std::make_tuple(std::move(buffer), std::move(data_kinds_converted), std::move(lines));
            }
            void append_jump_position(Line line, std::uint32_t target_pos) {
                auto jump_pos = append_jump_placeholder(line);
                update_jump_placeholder(jump_pos, target_pos);
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
            template <typename T> void update_placeholder(const Placeholder<T> &placeholder, std::int32_t value) {
                assert(placeholder.pos + sizeof(T) <= buffer.size());
                std::memcpy(buffer.data() + placeholder.pos, &value, sizeof(T));
            }

            struct JumpPlaceholder {
                const std::uint32_t pos;
            };

            JumpPlaceholder append_jump_placeholder(Line line) {
                DataKind kind = DataKind::JumpOffset;
                handle_line(line);
                auto pos = prepare((std::int32_t*)nullptr, 1, kind);
                return JumpPlaceholder{ pos };
            }
            void update_jump_placeholder(const JumpPlaceholder &placeholder, std::int32_t value) {
                assert(placeholder.pos + sizeof(std::int32_t) <= buffer.size());
                auto src = placeholder.pos + sizeof(std::int32_t);
                auto offset = (std::int32_t)((std::int64_t)value - (std::int64_t)src);
                std::memcpy(buffer.data() + placeholder.pos, &offset, sizeof(std::int32_t));
            }
        };

	}
}

#endif
