#include "stdafx.h"
#include "ast_expr_interpreted_string.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"
#include "string.h"

namespace OwcaScript::Internal {
	class ImplExprInterpretedString : public ImplExpr {
	public:
		using ImplExpr::ImplExpr;

		#define FIELDS(Q) \
			Q(values, std::span<ImplExpr*>) \
			Q(sizes, std::span<std::uint32_t>) \
            Q(strings, std::string_view)

		IMPL_DEFINE_EXPR(Kind::InterpretedString)

		OwcaValue execute_expression_impl(OwcaVM vm) const override {
            std::vector<std::pair<OwcaValue, std::string>> evaluated_values;
            size_t evaluated_values_size = 0;

            evaluated_values.reserve(values.size());
            for(size_t i = 0u; i < values.size(); ++i) {
                evaluated_values.push_back({ values[i]->execute_expression(vm), "" });

                if (evaluated_values.back().first.kind() != OwcaValueKind::String) {
                    evaluated_values.back().second = evaluated_values.back().first.to_string();
                    evaluated_values_size += evaluated_values.back().second.size();
                }
                else {
                    evaluated_values_size += evaluated_values.back().first.as_string(vm).size();
                }
            }

            auto new_size = strings.size() + evaluated_values_size;
            if (new_size >= (size_t(1) << 31)) {
                VM::get(vm).throw_string_too_large(new_size);
            }
            auto str_obj = VM::get(vm).allocate<String>(new_size, (std::uint32_t)new_size);
            auto str = str_obj->pointer();
            size_t strings_ptr = 0;
            for(auto j = 0u; j < sizes.size(); ++j) {
                if (j > 0) {
                    auto &ev = evaluated_values[j - 1];
                    if (ev.first.kind() == OwcaValueKind::String) {
                        std::memcpy(str, ev.first.as_string(vm).text().data(), ev.first.as_string(vm).size());
                        str += ev.first.as_string(vm).size();
                    }
                    else {
                        std::memcpy(str, ev.second.data(), ev.second.size());
                        str += ev.second.size();
                    }
                }
                std::memcpy(str, strings.data() + strings_ptr, sizes[j]);
                str += sizes[j];
                strings_ptr += sizes[j];
            }
            assert(strings_ptr == strings.size());
            return OwcaValue{ OwcaString{ std::move(str_obj) } };
		}
	};
	void AstExprInterpretedString::calculate_size(CodeBufferSizeCalculator& ei) const
	{
        ei.code_buffer.preallocate<ImplExprInterpretedString>(line);
		ei.code_buffer.preallocate_array<ImplExpr*>(evals.size());
		for (auto i = 0u; i < evals.size(); ++i) {
			evals[i]->calculate_size(ei);
		}
        ei.code_buffer.preallocate_array<std::uint32_t>(sizes.size());
        ei.code_buffer.allocate(strings);
    }

	ImplExpr* AstExprInterpretedString::emit(EmitInfo& ei) {
		auto ret = ei.code_buffer.preallocate<ImplExprInterpretedString>(line);
		auto tmp = ei.code_buffer.preallocate_array<ImplExpr*>(evals.size());
		for (auto i = 0u; i < evals.size(); ++i) {
			tmp[i] = evals[i]->emit(ei);
		}
        auto tmp_sizes = ei.code_buffer.preallocate_array<std::uint32_t>(sizes.size());
        for (auto i = 0u; i < sizes.size(); ++i) {
            tmp_sizes[i] = sizes[i];
        }
        auto strings_val = ei.code_buffer.allocate(strings);
		ret->init(tmp, tmp_sizes, strings_val);
		return ret;
	}
	void AstExprInterpretedString::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprInterpretedString::visit_children(AstVisitor& vis) {
        for(auto &e : evals) {
            e->visit(vis);
        }
	}
	void AstExprInterpretedString::initialize_serialization_functions(std::span<std::function<ImplExpr*(Deserializer&, Line)>> functions)
	{
		functions[(size_t)ImplExpr::Kind::InterpretedString] = [](Deserializer &ser, Line line) { return ser.allocate_object<ImplExprInterpretedString>(line); };
	}
}