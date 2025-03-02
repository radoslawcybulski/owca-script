#ifndef RC_OWCA_SCRIPT_DICTIONARY_H
#define RC_OWCA_SCRIPT_DICTIONARY_H

#include "stdafx.h"
#include "allocation_base.h"

namespace OwcaScript {
	class OwcaVM;

	namespace Internal {
		struct Dictionary {
			std::vector<std::tuple<size_t, OwcaValue, OwcaValue>> values;
			size_t elements = 0;
			size_t mask = 0;
			OwcaVM vm;
			const bool is_map = true;

			Dictionary(OwcaVM vm, bool is_map) : vm(vm), is_map(is_map) {}

			std::tuple<size_t, size_t, bool> find_place(OwcaValue key, size_t hash) const;
			std::tuple<size_t, size_t, bool> find_place(OwcaValue key) const;

			void try_rehash();
			OwcaValue &item(OwcaValue key);
			OwcaValue read(OwcaValue key);
			void write(OwcaValue key, OwcaValue value);
			OwcaValue pop(OwcaValue key);
			std::optional<std::pair<const OwcaValue*, OwcaValue*>> find(OwcaValue key);
			std::optional<std::pair<const OwcaValue*, const OwcaValue*>> find(OwcaValue key) const;

			size_t next(size_t pos = (size_t)-1) const;
			std::pair<const OwcaValue *, OwcaValue *> read(size_t pos);
			std::pair<const OwcaValue *, const OwcaValue *> read(size_t pos) const;

			std::string_view type() const;
			std::string to_string() const;
			void gc_mark(OwcaVM vm, GenerationGC generation_gc);
		};

		struct DictionaryShared : public AllocationBase {
			static constexpr const Kind object_kind = Kind::Map;

			Dictionary dict;

			DictionaryShared(OwcaVM vm, bool is_map) : dict(vm, is_map) {}
			
			std::string_view type() const override {
				return dict.type();
			}
			std::string to_string() const override {
				return dict.to_string();
			}
			void gc_mark(OwcaVM vm, GenerationGC generation_gc) override {
				return dict.gc_mark(vm, generation_gc);
			}
		};
	}
}

#endif
