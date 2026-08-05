#ifndef RC_OWCA_SCRIPT_EXEC_BUFFER_H
#define RC_OWCA_SCRIPT_EXEC_BUFFER_H

#include "stdafx.h"
#include "line.h"
#include "owca_code.h"
#include "variable_index.h"
#include "identifier_index.h"
#include <source_location>

#ifdef DEBUG
// #define OWCA_SCRIPT_EXEC_LOG
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
            VariableIndex,
            IdentifierIndex,
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
            case DataKind::VariableIndex: return "VariableIndex";
            case DataKind::IdentifierIndex: return "IdentifierIndex";
            default: return "Unknown";
            }
        }
        enum class ExecuteOp : std::uint8_t {
            ClassCreate,
            ExprCompareEq,
            ExprCompareNotEq,
            ExprCompareLessEq,
            ExprCompareMoreEq,
            ExprCompareLess,
            ExprCompareMore,
            ExprCompareIs,
            ExprConstantStringInterpolated,
            ExprIdentifierFunctionWrite,
            ExprMemberRead,
            ExprMemberWrite,
            ExprOper1BinNeg,
            ExprOper1LogNot,
            ExprOper1Negate,
            ExprRetTrueAndJumpIfTrue,
            ExprRetFalseAndJumpIfFalse,
            ExprMove,

            // order of binary operators is fixed
            ExprOper2First,
            ExprOper2Add = ExprOper2First,
            ExprOper2Sub,
            ExprOper2Mul,
            ExprOper2Div,
            ExprOper2Mod,
            ExprOper2BinOr,
            ExprOper2BinAnd,
            ExprOper2BinXor,
            ExprOper2BinLShift,
            ExprOper2BinRShift,

            // order of binary operators is fixed
            ExprOper2MemberFirst,
            ExprOper2MemberAdd = ExprOper2MemberFirst,
            ExprOper2MemberSub,
            ExprOper2MemberMul,
            ExprOper2MemberDiv,
            ExprOper2MemberMod,
            ExprOper2MemberBinOr,
            ExprOper2MemberBinAnd,
            ExprOper2MemberBinXor,
            ExprOper2MemberBinLShift,
            ExprOper2MemberBinRShift,

            // order of binary operators is fixed
            ExprOper2IndexFirst,
            ExprOper2IndexAdd = ExprOper2IndexFirst,
            ExprOper2IndexSub,
            ExprOper2IndexMul,
            ExprOper2IndexDiv,
            ExprOper2IndexMod,
            ExprOper2IndexBinOr,
            ExprOper2IndexBinAnd,
            ExprOper2IndexBinXor,
            ExprOper2IndexBinLShift,
            ExprOper2IndexBinRShift,

            ExprOper2MakeRange,
            ExprOper2IndexRead,
            ExprOper2IndexWrite,
            ExprOperXCall,
            ExprOperXCallWithMember,
            ExprOperXCreateArray,
            ExprOperXCreateTuple,
            ExprOperXCreateSet,
            ExprOperXCreateMap,
            ExprToString,
            ExprToIterator,
            ExprIteratorNextAndJumpIfCompleted,
            Function,
            If,
            IfAlmostAlwaysTrue,
            IfAlmostAlwaysFalse,
            Return,
            ReturnCloseIterator,
            ReturnValue,
            Throw,
            TryInit,
            TryCompleted,
            TryBlockCompleted,
            TryCatchType,
            TryCatchTypeCompleted,
            WithInit,
            WithCompleted,Yield,
            Jump,
            _Count
        };
        inline std::string_view to_string(ExecuteOp op) {
            switch(op) {
            case ExecuteOp::ClassCreate: return "ClassCreate";
            case ExecuteOp::ExprCompareEq: return "ExprCompareEq";
            case ExecuteOp::ExprCompareNotEq: return "ExprCompareNotEq";
            case ExecuteOp::ExprCompareLessEq: return "ExprCompareLessEq";
            case ExecuteOp::ExprCompareMoreEq: return "ExprCompareMoreEq";
            case ExecuteOp::ExprCompareLess: return "ExprCompareLess";
            case ExecuteOp::ExprCompareMore: return "ExprCompareMore";
            case ExecuteOp::ExprCompareIs: return "ExprCompareIs";
            case ExecuteOp::ExprConstantStringInterpolated: return "ExprConstantStringInterpolated";
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
            case ExecuteOp::ExprOper2MemberAdd: return "ExprOper2MemberAdd";
            case ExecuteOp::ExprOper2MemberSub: return "ExprOper2MemberSub";
            case ExecuteOp::ExprOper2MemberMul: return "ExprOper2MemberMul";
            case ExecuteOp::ExprOper2MemberDiv: return "ExprOper2MemberDiv";
            case ExecuteOp::ExprOper2MemberMod: return "ExprOper2MemberMod";
            case ExecuteOp::ExprOper2MemberBinOr: return "ExprOper2MemberBinOr";
            case ExecuteOp::ExprOper2MemberBinAnd: return "ExprOper2MemberBinAnd";
            case ExecuteOp::ExprOper2MemberBinXor: return "ExprOper2MemberBinXor";
            case ExecuteOp::ExprOper2MemberBinLShift: return "ExprOper2MemberBinLShift";
            case ExecuteOp::ExprOper2MemberBinRShift: return "ExprOper2MemberBinRShift";
            case ExecuteOp::ExprOper2IndexAdd: return "ExprOper2IndexAdd";
            case ExecuteOp::ExprOper2IndexSub: return "ExprOper2IndexSub";
            case ExecuteOp::ExprOper2IndexMul: return "ExprOper2IndexMul";
            case ExecuteOp::ExprOper2IndexDiv: return "ExprOper2IndexDiv";
            case ExecuteOp::ExprOper2IndexMod: return "ExprOper2IndexMod";
            case ExecuteOp::ExprOper2IndexBinOr: return "ExprOper2IndexBinOr";
            case ExecuteOp::ExprOper2IndexBinAnd: return "ExprOper2IndexBinAnd";
            case ExecuteOp::ExprOper2IndexBinXor: return "ExprOper2IndexBinXor";
            case ExecuteOp::ExprOper2IndexBinLShift: return "ExprOper2IndexBinLShift";
            case ExecuteOp::ExprOper2IndexBinRShift: return "ExprOper2IndexBinRShift";
            case ExecuteOp::ExprOper2MakeRange: return "ExprOper2MakeRange";
            case ExecuteOp::ExprOper2IndexRead: return "ExprOper2IndexRead";
            case ExecuteOp::ExprOper2IndexWrite: return "ExprOper2IndexWrite";
            case ExecuteOp::ExprOperXCall: return "ExprOperXCall";
            case ExecuteOp::ExprOperXCallWithMember: return "ExprOperXCallWithMember";
            case ExecuteOp::ExprOperXCreateArray: return "ExprOperXCreateArray";
            case ExecuteOp::ExprOperXCreateTuple: return "ExprOperXCreateTuple";
            case ExecuteOp::ExprOperXCreateSet: return "ExprOperXCreateSet";
            case ExecuteOp::ExprOperXCreateMap: return "ExprOperXCreateMap";
            case ExecuteOp::ExprToString: return "ExprToString";
            case ExecuteOp::ExprToIterator: return "ExprToIterator";
            case ExecuteOp::ExprIteratorNextAndJumpIfCompleted: return "ExprIteratorNextAndJumpIfCompleted";
            case ExecuteOp::ExprMove: return "ExprMove";
            case ExecuteOp::Function: return "Function";
            case ExecuteOp::If: return "If";
            case ExecuteOp::IfAlmostAlwaysTrue: return "IfAlmostAlwaysTrue";
            case ExecuteOp::IfAlmostAlwaysFalse: return "IfAlmostAlwaysFalse";
            case ExecuteOp::Return: return "Return";
            case ExecuteOp::ReturnCloseIterator: return "ReturnCloseIterator";
            case ExecuteOp::ReturnValue: return "ReturnValue";
            case ExecuteOp::Throw: return "Throw";
            case ExecuteOp::TryInit: return "TryInit";
            case ExecuteOp::TryCompleted: return "TryCompleted";
            case ExecuteOp::TryBlockCompleted: return "TryBlockCompleted";
            case ExecuteOp::TryCatchType: return "TryCatchType";
            case ExecuteOp::TryCatchTypeCompleted: return "TryCatchTypeCompleted";
            case ExecuteOp::WithInit: return "WithInit";
            case ExecuteOp::WithCompleted: return "WithCompleted";
            case ExecuteOp::Yield: return "Yield";
            case ExecuteOp::Jump: return "Jump";
            default: return "Unknown";
            }
        }
        using DataKindsType = std::unordered_map<const unsigned char *, DataKind>;

        class CodePosition {
            const unsigned char *pos = nullptr;
#ifdef DEBUG
            static const DataKindsType &empty() {
                static const DataKindsType empty_instance;
                return empty_instance;
            }
            const DataKindsType *data_kinds;
#endif

            size_t decode_size(std::source_location sl) {
#ifdef DEBUG
                ensure_data_kind(DataKind::Size, sl);
#endif
                std::uint32_t size;
                std::memcpy(&size, pos, sizeof(size));
                pos += sizeof(size);
                return size;
            }
#ifdef DEBUG
            void ensure_data_kind(DataKind expected, std::source_location sl) {
                if (data_kinds && !data_kinds->empty()) {
                    auto it = data_kinds->find(pos);
                    if (it == data_kinds->end() || it->second != expected) {
                        const unsigned char *start = pos;
                        for(auto &[p, k] : *data_kinds) {
                            if (p < start) start = p;
                        }
                        auto msg = std::format("{}:{}: Data kind mismatch at position {} ({}): expected {}, got {}", sl.file_name(), sl.line(), (pos - start), (void*)pos, to_string(expected), it == data_kinds->end() ? "Unknown" : to_string(it->second));
                        std::cout << msg << std::endl;
                        throw std::runtime_error(msg);
                    }
                }
            }
#endif
        public:

#ifdef DEBUG
            CodePosition(const unsigned char *pos, const DataKindsType &data_kinds) : pos(pos), data_kinds(&data_kinds) {}
            CodePosition(const CodePosition &cp, const unsigned char *pos) : pos(pos), data_kinds(cp.data_kinds) {}
            explicit CodePosition() : data_kinds(&empty()) {}
#else
            explicit CodePosition(const unsigned char *pos) : pos(pos) {}
            CodePosition(const CodePosition &, const unsigned char *pos) : pos(pos) {}
            explicit CodePosition() = default;
#endif

            template <typename T> T decode(std::source_location sl = std::source_location::current()) requires(std::is_same_v<T, std::string_view>) {
                auto size = decode_size(sl);
                auto ptr = pos;
#ifdef DEBUG
                if (size > 0) ensure_data_kind(DataKind::Blob, sl);
#endif
                pos += size;
                return std::string_view((const char*)ptr, size);
            }
            template <typename T> T decode(std::source_location sl = std::source_location::current()) requires(std::is_enum_v<T>) {
                static_assert(sizeof(T) <= sizeof(std::uint64_t), "Enum type too large to decode");
#ifdef DEBUG
                ensure_data_kind(std::is_same_v<T, ExecuteOp> ? DataKind::Op : DataKind::Enum, sl);
#endif
                T t;
                std::memcpy(&t, pos, sizeof(T));
                pos += sizeof(T);
                return static_cast<T>(t);
            }
            template <typename T> T decode(std::source_location sl = std::source_location::current()) requires(std::is_same_v<T, VariableIndex>) {
#ifdef DEBUG
                ensure_data_kind(DataKind::VariableIndex, sl);
#endif
                std::uint32_t t;
                std::memcpy(&t, pos, sizeof(std::uint32_t));
                pos += sizeof(std::uint32_t);
                return VariableIndex{ static_cast<VariableIndexKind>(t >> 30), t & 0x3fffffff };
            }
            template <typename T> T decode(std::source_location sl = std::source_location::current()) requires(std::is_same_v<T, IdentifierIndex>) {
#ifdef DEBUG
                ensure_data_kind(DataKind::IdentifierIndex, sl);
#endif
                std::uint32_t t;
                std::memcpy(&t, pos, sizeof(std::uint32_t));
                pos += sizeof(std::uint32_t);
                return IdentifierIndex(t);
            }
            template <typename T> T decode(std::source_location sl = std::source_location::current()) requires(std::is_integral_v<T> && !std::is_enum_v<T>) {
                static_assert(sizeof(T) <= sizeof(std::uint64_t), "Integral type too large to decode");
#ifdef DEBUG
                if constexpr (std::is_same_v<T, bool>) {
                    ensure_data_kind(DataKind::Bool, sl);
                } else if constexpr (sizeof(T) == 1) {
                    ensure_data_kind(DataKind::Int8, sl);
                } else if constexpr (sizeof(T) == 2) {
                    ensure_data_kind(DataKind::Int16, sl);
                } else if constexpr (sizeof(T) == 4) {
                    ensure_data_kind(DataKind::Int32, sl);
                } else if constexpr (sizeof(T) == 8) {
                    ensure_data_kind(DataKind::Int64, sl);
                } else {
                    static_assert(sizeof(T) == 0, "Unsupported integral type");
                }
#endif
                T t;
                std::memcpy(&t, pos, sizeof(T));
                pos += sizeof(T);
                return static_cast<T>(t);
            }
            template <typename T> T decode(std::source_location sl = std::source_location::current()) requires(std::is_floating_point_v<T> && !std::is_enum_v<T>) {
                static_assert(sizeof(T) <= sizeof(std::uint64_t), "Floating point type too large to decode");
#ifdef DEBUG
                if constexpr (sizeof(T) == 4) {
                    ensure_data_kind(DataKind::Float32, sl);
                } else if constexpr (sizeof(T) == 8) {
                    ensure_data_kind(DataKind::Float64, sl);
                } else {
                    static_assert(sizeof(T) == 0, "Unsupported floating point type");
                }
#endif
                T t;
                std::memcpy(&t, pos, sizeof(T));
                pos += sizeof(T);
                return static_cast<T>(t);
            }
            CodePosition decode_jump(std::source_location sl = std::source_location::current()) {
#ifdef DEBUG
                ensure_data_kind(DataKind::JumpOffset, sl);
#endif
                std::int32_t offset;
                std::memcpy(&offset, pos, sizeof(offset));
                pos += sizeof(offset);
                return CodePosition{ *this, pos + offset };
            }

            auto value() const { return pos; }
            CodePosition operator + (std::int32_t offset) const {
                return CodePosition(*this, pos + offset);
            }
            CodePosition operator - (std::int32_t offset) const {
                return CodePosition(*this, pos - offset);
            }
            CodePosition operator += (std::int32_t offset) {
                pos += offset;
                return *this;
            }
            CodePosition operator -= (std::int32_t offset) {
                pos -= offset;
                return *this;
            }
        };

        class ExecuteBufferWriter {
            using WriteDataKindsType = std::unordered_map<size_t, DataKind>;

            std::vector<unsigned char> buffer;
            WriteDataKindsType data_kinds;
            std::vector<LineEntry> lines;
            std::vector<std::pair<std::string, std::uint32_t>> identifiers;

            template <typename T> std::uint32_t prepare(Line line, const T *data, size_t sz, DataKind kind, std::source_location sl) {
                auto size = sizeof(T) * sz;
                auto current_size = buffer.size();
                auto padding = 0u;
                buffer.resize(current_size + padding + size);
                data_kinds[current_size + padding] = kind;
#ifdef OWCA_SCRIPT_EXEC_LOG
                if (kind == DataKind::Op) {
                    std::cout << sl.file_name() << ":" << sl.line() << " Writing data of kind " << to_string(kind) << " at line " << line.line << " position " << (current_size + padding) << " oper " << to_string((ExecuteOp)buffer[current_size + padding]) << std::endl;
                }
                else {
                    std::cout << sl.file_name() << ":" << sl.line() << " Writing data of kind " << to_string(kind) << " at line " << line.line << " position " << (current_size + padding) << std::endl;
                }
#endif
                return current_size + padding;
            }
            void append_impl_vec(Line line, const char *data, size_t sz, std::source_location sl) {
                if (sz == 0) return;
                auto pos = prepare(line, data, sz, DataKind::Blob, sl);
                std::memcpy(buffer.data() + pos, data, sz);
            }
            template <typename T> void append_impl(Line line, T value, DataKind kind, std::source_location sl) {
                handle_line(line);
                auto pos = prepare(line, &value, 1, kind, sl);
                std::memcpy(buffer.data() + pos, &value, sizeof(T));
            }
            void handle_line(Line line) {
                if (lines.empty() || lines.back().line != line.line) {
                    lines.emplace_back(LineEntry{static_cast<std::uint32_t>(buffer.size()), line.line});
                }
            }
            void append_size(Line line, size_t size, std::source_location sl) {
                append_impl(line, (std::uint32_t)size, DataKind::Size, sl);
            }
        public:
            ExecuteBufferWriter() = default;

            void finalize() {
                std::sort(identifiers.begin(), identifiers.end(), [](const auto &a, const auto &b) { return a.first < b.first; });
                auto identifier_count = append_placeholder<std::uint32_t>(Line{ 0 });
                auto ii = 0;
                for(auto i = 0u; i < identifiers.size(); ) {
                    append(Line{ 0 }, identifiers[i].first);
                    auto entries_count = append_placeholder<std::uint32_t>(Line{ 0 });
                    auto first = i;
                    for(; i < identifiers.size() && identifiers[first].first == identifiers[i].first; ++i) {
                        append(Line{ 0 }, identifiers[i].second);
                    }
                    update_placeholder(entries_count, (std::uint32_t)(i - first));
                    ++ii;
                }
                update_placeholder(identifier_count, ii);
            }

            Line current_line() const {
                if (lines.empty()) return Line{ 0 };
                return Line{ lines.back().line };
            }
            auto take() && {
                DataKindsType data_kinds_converted;
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
            template <typename T> void append(Line line, T value, std::source_location sl = std::source_location::current()) requires(std::is_enum_v<T>) {
                append_impl(line, value, std::is_same_v<T, ExecuteOp> ? DataKind::Op : DataKind::Enum, sl);
            }
            template <typename T> void append(Line line, T value, std::source_location sl = std::source_location::current()) requires(std::is_integral_v<T> && !std::is_enum_v<T>) {
                DataKind kind;
                if constexpr (std::is_same_v<T, bool>) kind = DataKind::Bool;
                else if constexpr (std::is_same_v<T, std::int8_t> || std::is_same_v<T, std::uint8_t>) kind = DataKind::Int8;
                else if constexpr (std::is_same_v<T, std::int16_t> || std::is_same_v<T, std::uint16_t>) kind = DataKind::Int16;
                else if constexpr (std::is_same_v<T, std::int32_t> || std::is_same_v<T, std::uint32_t>) kind = DataKind::Int32;
                else if constexpr (std::is_same_v<T, std::int64_t> || std::is_same_v<T, std::uint64_t>) kind = DataKind::Int64;
                else {
                    static_assert(sizeof(T) == 0, "Unsupported integral type");
                }
                append_impl(line, value, kind, sl);
            }
            template <typename T> void append(Line line, T value, std::source_location sl = std::source_location::current()) requires(std::is_floating_point_v<T>) {
                DataKind kind;
                if constexpr (std::is_same_v<T, float>) kind = DataKind::Float32;
                else if constexpr (std::is_same_v<T, double>) kind = DataKind::Float64;
                else {
                    static_assert(sizeof(T) == 0, "Unsupported floating point type");
                }
                append_impl(line, value, kind, sl);
            }
            void append(Line line, const char *str, std::source_location sl = std::source_location::current()) {
                auto sz = strlen(str);
                append_size(line, sz, sl);
                append_impl_vec(line, str, sz, sl);
            }
            void append_identifier(Line line, std::string_view str, std::source_location sl = std::source_location::current()) {
                auto p = buffer.size();
                append_impl(line, (std::uint32_t)0, DataKind::IdentifierIndex, sl);
                identifiers.push_back({ std::string{ str }, p });
            }
            void append(Line line, std::string_view str, std::source_location sl = std::source_location::current()) {
                append_size(line, str.size(), sl);
                append_impl_vec(line, str.data(), str.size(), sl);
            }
            void append(Line line, const std::string &str, std::source_location sl = std::source_location::current()) {
                append_size(line, str.size(), sl);
                append_impl_vec(line, str.data(), str.size(), sl);
            }
            void append(Line line, VariableIndex value, std::source_location sl = std::source_location::current()) {
                append_impl(line, value.value(), DataKind::VariableIndex, sl);
            }
            void append(Line line, IdentifierIndex value, std::source_location sl = std::source_location::current()) {
                append_impl(line, value.value(), DataKind::IdentifierIndex, sl);
            }
            static void set_identifier_index(unsigned char *pos, IdentifierIndex value) {
                auto v = value.value();
                std::memcpy(pos, &v, sizeof(v));
            }
            template <typename T> void append(Line line, const T &t) requires (!std::is_enum_v<T> && !std::is_integral_v<T> && !std::is_floating_point_v<T> && !std::is_same_v<T, std::string_view> && !Span<T> && !Vector<T>) {
                serialize_object(*this, line, t);
            }
            std::uint32_t position() const { return buffer.size(); }

            template <typename T> struct Placeholder {
                const std::uint32_t pos;
            };

            template <typename T> Placeholder<T> append_placeholder(Line line, std::source_location sl = std::source_location::current()) {
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
                auto pos = prepare(line, (T*)nullptr, 1, kind, sl);
                return Placeholder<T>{ pos };
            }
            template <typename T> void update_placeholder(const Placeholder<T> &placeholder, std::int32_t value) {
                assert(placeholder.pos + sizeof(T) <= buffer.size());
                std::memcpy(buffer.data() + placeholder.pos, &value, sizeof(T));
            }

            struct JumpPlaceholder {
                const std::uint32_t pos;
            };

            JumpPlaceholder append_jump_placeholder(Line line, std::source_location sl = std::source_location::current()) {
                DataKind kind = DataKind::JumpOffset;
                handle_line(line);
                auto pos = prepare(line, (std::int32_t*)nullptr, 1, kind, sl);
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
