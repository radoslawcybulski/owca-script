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

		std::string_view name;
		std::span<ImplExpr*> base_classes;
		std::span<ImplExpr*> members;

		void init(std::string_view name, std::span<ImplExpr*> base_classes, std::span<ImplExpr*> members)
		{
			this->name = name;
			this->base_classes = base_classes;
			this->members = members;
		}

		OwcaValue execute(OwcaVM &vm) const override
		{
			auto code = VM::get(vm).currently_running_code();
			assert(code);

			auto cls = VM::get(vm).allocate<Class>(0, line);
			cls->type_ = name;
			cls->code = std::move(code);
			cls->base_classes.reserve(base_classes.size());
			for (auto b : base_classes) {
				auto res = b->execute(vm);
				auto c = VM::get(vm).ensure_is_class(res);
				cls->base_classes.push_back(c);
			}

			std::function<void(Class*)> fill_lookup_order = [&](Class* c) {
				cls->lookup_order.push_back(c);
				for (auto q : cls->base_classes)
					fill_lookup_order(q);
			};
			fill_lookup_order(cls);

			size_t offset = 0;
			for (auto q : cls->base_classes) {
				for (auto it : q->native_storage_pointers) {
					cls->native_storage_pointers.insert({ it.first, { offset, it.second.second } });
					offset += it.second.second;
					offset = (offset + 15) & ~15;
				}
			}
			cls->native_storage_total = offset;

			for (auto m : members) {
				auto f = m->execute(vm);
				auto fnc = f.as_functions(vm);
				assert(fnc.functions->functions.size() == 1);
				for (auto it2 : fnc.functions->functions) {
					cls->runtime_functions.push_back(it2.second);
				}
			}

			for (auto i = cls->lookup_order.size(); i > 0; --i) {
				for (auto f : cls->lookup_order[i - 1]->runtime_functions) {
					auto name = f->name;

					auto it = cls->values.insert({ std::string{ name }, {} });
					if (it.second || it.first->second.kind() != OwcaValueKind::Functions) {
						auto rf = VM::get(vm).allocate<RuntimeFunctions>(0, name);
						it.first->second = OwcaFunctions{ rf };
					}
					auto dst_fnc = it.first->second.as_functions(vm);
					dst_fnc.functions->functions[f->param_count] = f;
				}
			}

			return OwcaClass{ cls };
		}
	};

	class ImplExprNativeClass : public ImplExprScriptClass {
	public:
		using ImplExprScriptClass::ImplExprScriptClass;

		OwcaClass::NativeClassInterface *native;

		void init(std::string_view name, std::span<ImplExpr*> base_classes, std::span<ImplExpr*> members, OwcaClass::NativeClassInterface *native)
		{
			ImplExprScriptClass::init(name, base_classes, members);
			this->native = native;
		}

		OwcaValue execute(OwcaVM &vm) const override
		{
			auto res = ImplExprScriptClass::execute(vm);
			auto cls = VM::get(vm).ensure_is_class(res);
			auto size = native->native_storage_size();
			cls->native_storage_pointers[cls] = { cls->native_storage_total, size };
			cls->native_storage_total = (cls->native_storage_total + 15) & ~15;

			return res;
		}
	};

	void AstClass::calculate_size(CodeBufferSizeCalculator &ei) const
	{
		if (native) ei.code_buffer.preallocate<ImplExprNativeClass>(line);
		else ei.code_buffer.preallocate<ImplExprScriptClass>(line);

		ei.code_buffer.allocate(name_);

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
		
		auto bc = ei.code_buffer.preallocate_array<ImplExpr*>(base_classes.size());
		for (auto i = 0u; i < base_classes.size(); ++i) {
			bc[i] = base_classes[i]->emit(ei);
		}
		auto fncs = ei.code_buffer.preallocate_array<ImplExpr*>(members.size());
		for (auto i = 0u; i < members.size(); ++i) {
			fncs[i] = members[i]->emit(ei);
		}

		if (native) {
			auto ni = native.get();
			ei.code_buffer.register_native_interface(std::move(native));
			ret2->init(nm, bc, fncs, ni);
			return ret2;
		}
		else {
			ret1->init(nm, bc, fncs);
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