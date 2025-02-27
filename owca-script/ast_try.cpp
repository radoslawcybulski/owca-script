#include "stdafx.h"
#include "ast_try.h"
#include "vm.h"
#include "owca_value.h"

namespace OwcaScript::Internal {
	class ImplTry : public ImplStat {
	public:
		using ImplStat::ImplStat;

        ImplStat* body;
        std::span<std::tuple<unsigned int, std::span<ImplExpr*>, ImplStat*>> catches;

		void init(ImplStat* body, std::span<std::tuple<unsigned int, std::span<ImplExpr*>, ImplStat*>> catches) {
            this->body = body;
            this->catches = catches;
		}

		void execute_statement_impl(OwcaVM vm) const override {
            try {
                body->execute_statement(vm);
                return;
            }
            catch(OwcaException oe) {
                auto sentinel = VM::ExceptionHandlingSentinel{ VM::get(vm), oe };
                auto oe_type = oe.type();

                for(auto [ ident_index, exception_types, catch_body ] : catches) {
                    bool found = false;

                    for(auto et : exception_types) {
                        auto v = et->execute_expression(vm);
                        auto oe2 = v.as_class(vm);
                        if (oe_type.has_base_class(oe2)) {
                            found = true;
                            break;
                        }
                    }

                    if (found) {
                        if (ident_index != std::numeric_limits<unsigned int>::max()) {
                            VM::get(vm).set_identifier(ident_index, oe, false);
                        }
                        catch_body->execute_statement(vm);
                        break;
                    }
                }
            }
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
}