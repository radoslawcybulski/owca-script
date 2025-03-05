#include "stdafx.h"
#include "ast_try.h"
#include "vm.h"
#include "owca_value.h"

namespace OwcaScript::Internal {
	class ImplTry : public ImplStat {
	public:
        using ImplStat::ImplStat;

		#define FIELDS(Q) \
			Q(body, ImplStat*) \
            Q(catches, std::span<std::tuple<unsigned int, std::span<ImplExpr*>, ImplStat*>>)

        IMPL_DEFINE_STAT(Kind::Try)

        struct ExceptionHandlerInfo {
            ImplStat *catch_body = nullptr;
            unsigned int ident_index = 0;
            OwcaValue oe;
        };
        std::optional<ExceptionHandlerInfo> find_handler(OwcaVM vm, OwcaException oe) const {
            auto oe_type = oe.type();

            for(auto [ ident_index, exception_types, catch_body ] : catches) {
                for(auto et : exception_types) {
                    auto v = et->execute_expression(vm);
                    auto oe2 = v.as_class(vm);
                    if (oe_type.has_base_class(oe2))
                        return ExceptionHandlerInfo{ catch_body, ident_index, oe };
                }
            }
            return std::nullopt;
        }

		void execute_statement_impl(OwcaVM vm) const override {
            std::optional<OwcaException> oe_value;
            std::optional<ExceptionHandlerInfo> handler_info;
            try {
                body->execute_statement(vm);
                return;
            }
            catch(OwcaException oe) {
                auto sentinel = VM::ExceptionHandlingSentinel{ VM::get(vm), oe };
                handler_info = find_handler(vm, oe);
                if (!handler_info) throw;
                oe_value = oe;
            }
            auto sentinel = VM::ExceptionHandlingSentinel{ VM::get(vm), *oe_value };
            if (handler_info->ident_index != std::numeric_limits<unsigned int>::max()) {
                VM::get(vm).set_identifier(handler_info->ident_index, handler_info->oe, false);
            }
            handler_info->catch_body->execute_statement(vm);
}
		Task execute_generator_statement_impl(OwcaVM vm, State &st) const override {
            VM::get(vm).update_execution_line(line);
            auto pp = VM::AllocatedObjectsPointer{ VM::get(vm) };

            std::optional<OwcaException> oe_value;
            std::optional<ExceptionHandlerInfo> handler_info;
            try {
                co_await body->execute_generator_statement(vm, st);
                co_return;
            }
            catch(OwcaException oe) {
                auto sentinel = VM::ExceptionHandlingSentinel{ VM::get(vm), oe };
                handler_info = find_handler(vm, oe);
                if (!handler_info) throw;
                oe_value = oe;
            }
            auto sentinel = VM::ExceptionHandlingSentinel{ VM::get(vm), *oe_value };
            if (handler_info->ident_index != std::numeric_limits<unsigned int>::max()) {
                VM::get(vm).set_identifier(handler_info->ident_index, handler_info->oe, false);
            }
            co_await handler_info->catch_body->execute_generator_statement(vm, st);
            co_return;
		}
		size_t calculate_generator_allocation_size() const override {
            auto sz = body->calculate_generator_allocation_size();
            for(auto [ ident_index, exception_types, catch_body ] : catches) {
                sz = std::max(sz, catch_body->calculate_generator_allocation_size());
            }
			return sz + calculate_generator_object_size_for_this();
		}
	};

	void AstTry::calculate_size(CodeBufferSizeCalculator &ei) const
	{
		ei.code_buffer.preallocate<ImplTry>(line);
        body->calculate_size(ei);
        for(auto &q : catches) {
            for(auto &w : std::get<2>(q))
                w->calculate_size(ei);
            ei.code_buffer.preallocate_array<ImplExpr*>(std::get<2>(q).size());
            std::get<3>(q)->calculate_size(ei);
        }
        ei.code_buffer.preallocate_array<std::tuple<unsigned int, std::span<ImplExpr*>, ImplStat*>>(catches.size());
	}
	ImplStat* AstTry::emit(EmitInfo& ei) {
		auto ret = ei.code_buffer.preallocate<ImplTry>(line);
		auto b = body->emit(ei);
        
        auto catches_dest = ei.code_buffer.preallocate_array<std::tuple<unsigned int, std::span<ImplExpr*>, ImplStat*>>(catches.size());
        for(auto i = 0u; i < catches.size(); ++i) {
            auto types_dest = ei.code_buffer.preallocate_array<ImplExpr*>(std::get<2>(catches[i]).size());
            for(auto j = 0u; j < std::get<2>(catches[i]).size(); ++j) {
                types_dest[j] = std::get<2>(catches[i])[j]->emit(ei);
            }
            auto body_dest = std::get<3>(catches[i])->emit(ei);
            auto index = std::get<1>(catches[i]);
            catches_dest[i] = { index, types_dest, body_dest };
        }

        ret->init(b, catches_dest);
		return ret;
	}

	void AstTry::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstTry::visit_children(AstVisitor& vis) {
        body->visit(vis);
        for(auto &c : catches) {
            for(auto &e : std::get<2>(c)) {
                e->visit(vis);
            }
            std::get<3>(c)->visit(vis);
        }
	}
	void AstTry::initialize_serialization_functions(std::span<std::function<ImplStat*(Deserializer&, Line)>> functions)
	{
		functions[(size_t)ImplStat::Kind::Try] = [](Deserializer &ser, Line line) { return ser.allocate_object<ImplTry>(line); };
	}
}