#include "owca-script/identifier_index.h"
#include "stdafx.h"
#include "namespace.h"
#include "vm.h"
#include <stdexcept>

namespace OwcaScript::Internal {
	std::string_view Namespace::type() const {
		return "namespace";
	}
	std::string Namespace::to_string() const {
		return std::format("namespace {}", code.filename());
	}
	void Namespace::gc_mark(GenerationGC generation_gc) const {
        gc_mark_value(generation_gc, globals);
        gc_mark_value(generation_gc, code);
    }

    OwcaValue Namespace::member(std::string_view key) const {
        return member(current_vm().get_identifier_index(key));
    }
    std::optional<OwcaValue> Namespace::try_member(std::string_view key) const {
        return try_member(current_vm().get_identifier_index(key));
    }
    void Namespace::set_member(std::string_view key, OwcaValue val) {
        set_member(current_vm().get_identifier_index(key), std::move(val));
    }
    bool Namespace::try_set_member(std::string_view key, OwcaValue val) {
        return try_set_member(current_vm().get_identifier_index(key), std::move(val));
    }

    OwcaValue Namespace::member(IdentifierIndex key) const {
        auto val = try_member(key);
        if (!val) {
            throw std::runtime_error{ std::format("namespace {} has no member named {}", code.filename(), current_vm().get_identifier_name(key)) };
        }
        return *val;
    }

    std::optional<OwcaValue> Namespace::try_member(IdentifierIndex key) const {
        auto it = code.identifier_to_global_index().find(key);
        if (it == code.identifier_to_global_index().end()) {
            return std::nullopt;
        }
        return globals[it->second];
    }
    void Namespace::set_member(IdentifierIndex key, OwcaValue val) {
        auto it = code.identifier_to_global_index().find(key);
        if (it == code.identifier_to_global_index().end()) {
            throw std::runtime_error{ std::format("namespace {} has no member named {}", code.filename(), current_vm().get_identifier_name(key)) };
        }
        globals[it->second] = val;
    }
    bool Namespace::try_set_member(IdentifierIndex key, OwcaValue val) {
        auto it = code.identifier_to_global_index().find(key);
        if (it == code.identifier_to_global_index().end()) {
            return false;
        }
        globals[it->second] = val;
        return true;
    }

    OwcaValue Namespace::bound_function_self_object() {
        return OwcaNamespace{ this };
    }
}
