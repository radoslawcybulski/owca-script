#include "stdafx.h"
#include "ast_try.h"
#include "vm.h"
#include "owca_value.h"

namespace OwcaScript::Internal {
// 	class ImplTry : public ImplStat {
// 	public:
//         using ImplStat::ImplStat;

// 		#define FIELDS(Q) \
// 			Q(body, ImplStat*) \
//             Q(catches, std::span<std::tuple<unsigned int, std::span<ImplExpr*>, ImplStat*>>)

//         IMPL_DEFINE_STAT(Kind::Try)

//         struct ExceptionHandlerInfo {
//             ImplStat *catch_body = nullptr;
//             unsigned int ident_index = 0;
//             OwcaValue oe;
//         };
//         std::optional<ExceptionHandlerInfo> find_handler(OwcaVM vm, OwcaException oe) const {
//             auto oe_type = oe.type();

//             for(auto [ ident_index, exception_types, catch_body ] : catches) {
//                 for(auto et : exception_types) {
//                     auto v = et->execute_expression(vm);
//                     auto oe2 = v.as_class(vm);
//                     if (oe_type.has_base_class(oe2))
//                         return ExceptionHandlerInfo{ catch_body, ident_index, oe };
//                 }
//             }
//             return std::nullopt;
//         }

// 		void execute_statement_impl(OwcaVM vm) const override {
//             std::optional<OwcaException> oe_value;
//             std::optional<ExceptionHandlerInfo> handler_info;
//             try {
//                 body->execute_statement(vm);
//                 return;
//             }
//             catch(OwcaException oe) {
//                 auto sentinel = VM::ExceptionHandlingSentinel{ VM::get(vm), oe };
//                 handler_info = find_handler(vm, oe);
//                 if (!handler_info) throw;
//                 oe_value = oe;
//             }
//             auto sentinel = VM::ExceptionHandlingSentinel{ VM::get(vm), *oe_value };
//             if (handler_info->ident_index != std::numeric_limits<unsigned int>::max()) {
//                 VM::get(vm).set_identifier(handler_info->ident_index, handler_info->oe, false);
//             }
//             handler_info->catch_body->execute_statement(vm);
// }
// 		Task execute_generator_statement_impl(OwcaVM vm, State &st) const override {
//             VM::get(vm).update_execution_line(line);
//             auto pp = VM::AllocatedObjectsPointer{ VM::get(vm) };

//             std::optional<OwcaException> oe_value;
//             std::optional<ExceptionHandlerInfo> handler_info;
//             try {
//                 co_await body->execute_generator_statement(vm, st);
//                 co_return;
//             }
//             catch(OwcaException oe) {
//                 auto sentinel = VM::ExceptionHandlingSentinel{ VM::get(vm), oe };
//                 handler_info = find_handler(vm, oe);
//                 if (!handler_info) throw;
//                 oe_value = oe;
//             }
//             auto sentinel = VM::ExceptionHandlingSentinel{ VM::get(vm), *oe_value };
//             if (handler_info->ident_index != std::numeric_limits<unsigned int>::max()) {
//                 VM::get(vm).set_identifier(handler_info->ident_index, handler_info->oe, false);
//             }
//             co_await handler_info->catch_body->execute_generator_statement(vm, st);
//             co_return;
// 		}
// 		size_t calculate_generator_allocation_size() const override {
//             auto sz = body->calculate_generator_allocation_size();
//             for(auto [ ident_index, exception_types, catch_body ] : catches) {
//                 sz = std::max(sz, catch_body->calculate_generator_allocation_size());
//             }
// 			return sz + calculate_generator_object_size_for_this();
// 		}
// 	};

	void AstTry::emit(EmitInfo& ei) {
        auto start = ei.code_writer.position();
        ei.code_writer.append(line, ExecuteOp::TryInit);
        auto body_start = ei.code_writer.append_placeholder<std::uint32_t>(line);
        auto body_end = ei.code_writer.append_placeholder<std::uint32_t>(line);
        std::vector<ExecuteBufferWriter::Placeholder<std::uint32_t>> fallback_jumps;
        for(auto &c : catches_) {
            for(auto &q : std::get<2>(c)) {
                q->emit(ei);
            }
            auto &line = std::get<3>(c)->line;
            ei.code_writer.append(line, ExecuteOp::TryCatchType);
            ei.code_writer.append(line, (std::uint32_t)std::get<2>(c).size());
            ei.code_writer.append(line, std::get<1>(c));
            auto skip = ei.code_writer.append_placeholder<std::uint32_t>(line);
            std::get<3>(c)->emit(ei);
            ei.code_writer.append(line, ExecuteOp::TryBlockCompleted);
            fallback_jumps.push_back(ei.code_writer.append_placeholder<std::uint32_t>(line));
            ei.code_writer.update_placeholder(skip, ei.code_writer.position());
        }
        ei.code_writer.append(line, ExecuteOp::TryCatchTypeCompleted);
        ei.code_writer.update_placeholder(body_start, ei.code_writer.position());
        body_->emit(ei);
        ei.code_writer.update_placeholder(body_end, ei.code_writer.position());
        for(auto &j : fallback_jumps) {
            ei.code_writer.update_placeholder(j, ei.code_writer.position());
        }
        ei.code_writer.append(line, ExecuteOp::TryCompleted);
	}

	void AstTry::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstTry::visit_children(AstVisitor& vis) {
        body_->visit(vis);
        for(auto &c : catches_) {
            for(auto &e : std::get<2>(c)) {
                e->visit(vis);
            }
            std::get<3>(c)->visit(vis);
        }
	}
}