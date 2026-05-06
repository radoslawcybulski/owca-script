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
	void Namespace::gc_mark(OwcaVM vm, GenerationGC generation_gc) const {
        gc_mark_value(vm, generation_gc, globals);
    }
    OwcaValue Namespace::member(std::string_view key) const {
        auto val = try_member(key);
        if (!val) {
            throw std::runtime_error{ std::format("namespace {} has no member named {}", code.filename(), key) };
        }
        return *val;
    }

    std::optional<OwcaValue> Namespace::try_member(std::string_view key) const {
        auto it = identifier_to_global_index.find(key);
        if (it == identifier_to_global_index.end()) {
            return std::nullopt;
        }
        return globals[it->second];
    }
    void Namespace::set_member(std::string_view key, OwcaValue val) {
        auto it = identifier_to_global_index.find(key);
        if (it == identifier_to_global_index.end()) {
            throw std::runtime_error{ std::format("namespace {} has no member named {}", code.filename(), key) };
        }
        globals[it->second] = val;
    }
    bool Namespace::try_set_member(std::string_view key, OwcaValue val) {
        auto it = identifier_to_global_index.find(key);
        if (it == identifier_to_global_index.end()) {
            return false;
        }
        globals[it->second] = val;
        return true;
    }
}