#include "stdafx.h"
#include "ast_class.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"
#include "object.h"
#include "runtime_function.h"

namespace OwcaScript::Internal {
	class ImplExprScriptClass : public ImplExpr {
	public:
		using ImplExpr::ImplExpr;

		std::string_view name, full_name;
		std::span<ImplExpr*> base_classes;
		std::span<ImplExpr*> members;

		void init(std::string_view name, std::string_view full_name, std::span<ImplExpr*> base_classes, std::span<ImplExpr*> members)
		{
			this->name = name;
			this->full_name = full_name;
			this->base_classes = base_classes;
			this->members = members;
		}

		OwcaValue execute_impl(OwcaVM vm) const override
		{
			auto code = VM::get(vm).currently_running_code();
			assert(code);

			auto cls = VM::get(vm).allocate<Class>(0, line, name, full_name, std::move(code), base_classes.size());
			for (auto b : base_classes) {
				auto res = b->execute(vm);
				cls->initialize_add_base_class(vm, res);
			}

			for (auto m : members) {
				auto f = m->execute(vm);
				cls->initialize_add_function(vm, f);
			}

			cls->finalize_initializing(vm);

			return OwcaClass{ cls };
		}
	};

	class ImplExprNativeClass : public ImplExprScriptClass {
	public:
		using ImplExprScriptClass::ImplExprScriptClass;

		void init(std::string_view name, std::string_view full_name, std::span<ImplExpr*> base_classes, std::span<ImplExpr*> members)
		{
			ImplExprScriptClass::init(name, full_name, base_classes, members);
		}

		OwcaValue execute_impl(OwcaVM vm) const override
		{
			auto res = ImplExprScriptClass::execute_impl(vm);
			auto cls = VM::get(vm).ensure_is_class(res);
			auto native = cls->code->native_code_provider();
			if (native) {
				if (auto impl = native->native_class(full_name, ClassToken{ cls })) {
					auto size = impl->native_storage_size();
					cls->native_storage_pointers[cls] = { cls->native_storage_total, size };
					cls->native_storage_total = (cls->native_storage_total + size + 15) & ~15;
					cls->native = std::move(impl);
				}
			}
			if (!cls->native) {
				VM::get(vm).throw_missing_native(std::format("missing native function {}", full_name));
			}

			return res;
		}
	};

	void AstClass::calculate_size(CodeBufferSizeCalculator &ei) const
	{
		if (native) ei.code_buffer.preallocate<ImplExprNativeClass>(line);
		else ei.code_buffer.preallocate<ImplExprScriptClass>(line);

		ei.code_buffer.allocate(name_);
		ei.code_buffer.allocate(full_name_);

		ei.code_buffer.preallocate_array<ImplExpr*>(base_classes.size());
		for (auto i = 0u; i < base_classes.size(); ++i) {
			base_classes[i]->calculate_size(ei);
		}
		ei.code_buffer.preallocate_array<ImplExpr*>(members.size());
		for (auto i = 0u; i < members.size(); ++i) {
			members[i]->calculate_size(ei);
		}
	}

	ImplExpr* AstClass::emit(EmitInfo& ei) {
		ImplExprScriptClass* ret1 = nullptr;
		ImplExprNativeClass* ret2 = nullptr;
		
		if (native) ret2 = ei.code_buffer.preallocate<ImplExprNativeClass>(line);
		else ret1 = ei.code_buffer.preallocate<ImplExprScriptClass>(line);

		auto nm = ei.code_buffer.allocate(name_);
		auto fm = ei.code_buffer.allocate(full_name_);
		
		auto bc = ei.code_buffer.preallocate_array<ImplExpr*>(base_classes.size());
		for (auto i = 0u; i < base_classes.size(); ++i) {
			bc[i] = base_classes[i]->emit(ei);
		}
		auto fncs = ei.code_buffer.preallocate_array<ImplExpr*>(members.size());
		for (auto i = 0u; i < members.size(); ++i) {
			fncs[i] = members[i]->emit(ei);
		}

		if (native) {
			ret2->init(nm, fm, bc, fncs);
			return ret2;
		}
		else {
			ret1->init(nm, fm, bc, fncs);
			return ret1;
		}
	}

	void AstClass::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstClass::visit_children(AstVisitor& vis) {
		for(auto &q : base_classes)
			q->visit(vis);
		for(auto &q : members)
			q->visit(vis);
	}
}