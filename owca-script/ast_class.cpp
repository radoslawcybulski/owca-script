#include "stdafx.h"
#include "ast_class.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"
#include "object.h"
#include "runtime_function.h"

namespace OwcaScript::Internal {
	AstClass::AstClass(Line line, std::string_view name, std::string full_name, std::vector<std::unique_ptr<AstExpr>> base_classes, std::vector<std::unique_ptr<AstFunction>> members, std::vector<std::string> variable_names, bool all_variable_names, bool native) : 
		AstExpr(line), base_classes_(std::move(base_classes)), members_(std::move(members)), variable_names_(std::move(variable_names)), name_(std::string{name}), full_name_(std::move(full_name)), all_variable_names_(all_variable_names), native_(native) {
			assert(this->variable_names_.empty() || native_);
		}

	// class ImplExprScriptClass : public ImplExpr {
	// public:
	// 	using ImplExpr::ImplExpr;

	// 	#define FIELDS(Q) \
	// 		Q(name, std::string_view) \
	// 		Q(full_name, std::string_view) \
	// 		Q(base_classes, std::span<ImplExpr*>) \
	// 		Q(members, std::span<ImplExpr*>) \
	// 		Q(native_variable_names, std::span<std::string_view>) \
	// 		Q(all_variables, bool)

	// 	IMPL_DEFINE_EXPR(Kind::ScriptClass)

	// 	OwcaValue execute_expression_impl(OwcaVM vm) const override
	// 	{
	// 		auto code = VM::get(vm).currently_running_code();
	// 		assert(code);

	// 		auto cls = VM::get(vm).allocate<Class>(0, line, name, full_name, std::move(code), base_classes.size());
	// 		auto guard = VM::get(vm).set_currently_building_class(ClassToken{ cls });
	// 		for (auto b : base_classes) {
	// 			auto res = b->execute_expression(vm);
	// 			cls->initialize_add_base_class(vm, res);
	// 		}

	// 		for (auto m : members) {
	// 			auto f = m->execute_expression(vm);
	// 			cls->initialize_add_function(vm, f);
	// 		}
	// 		if (all_variables) {
	// 			cls->initialize_set_all_variables();
	// 		}
	// 		else {
	// 			for(auto i = 0u; i < native_variable_names.size(); ++i) {
	// 				cls->initialize_add_variable(native_variable_names[i]);
	// 			}
	// 		}

	// 		cls->finalize_initializing(vm);

	// 		return OwcaClass{ cls };
	// 	}
	// };

	// class ImplExprNativeClass : public ImplExprScriptClass {
	// public:
	// 	using ImplExprScriptClass::ImplExprScriptClass;

	// 	Kind kind() const override { return Kind::NativeClass; }
	// 	OwcaValue execute_expression_impl(OwcaVM vm) const override
	// 	{
	// 		auto res = ImplExprScriptClass::execute_expression_impl(vm);
	// 		auto cls = res.as_class(vm).internal_value();
	// 		auto native = cls->code->native_code_provider();
	// 		if (native) {
	// 			if (auto impl = native->native_class(full_name, ClassToken{ cls })) {
	// 				auto size = impl->native_storage_size();
	// 				cls->native_storage_pointers[cls] = { cls->native_storage_total, size };
	// 				cls->native_storage_total = (cls->native_storage_total + size + 15) & ~15;
	// 				cls->native = std::move(impl);
	// 			}
	// 		}
	// 		if (!cls->native) {
	// 			VM::get(vm).throw_missing_native(std::format("missing native class {}", full_name));
	// 		}

	// 		return res;
	// 	}
	// };

	void AstClass::emit(EmitInfo& ei) {
		for(auto &q : members_)
			q->emit(ei);
		for(auto &q : base_classes_)
			q->emit(ei);
		ei.code_writer.append(line, ExecuteOp::Class);
		ei.code_writer.append(line, native_);
		ei.code_writer.append(line, name_);
		ei.code_writer.append(line, full_name_);
		ei.code_writer.append(line, (std::uint32_t)base_classes_.size());
		ei.code_writer.append(line, (std::uint32_t)members_.size());
		ei.code_writer.append(line, all_variable_names_);
		ei.code_writer.append(line, (std::uint32_t)variable_names_.size());
		for (auto &q : variable_names_)
			ei.code_writer.append(line, q);
	}

	void AstClass::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstClass::visit_children(AstVisitor& vis) {
		for(auto &q : base_classes_)
			q->visit(vis);
		for(auto &q : members_)
			q->visit(vis);
	}
}