#ifndef RC_OWCA_SCRIPT_DICTIONARY_H
#define RC_OWCA_SCRIPT_DICTIONARY_H

#include "stdafx.h"
#include "allocation_base.h"
#include "owca_value.h"

namespace OwcaScript {
	class OwcaVM;

	namespace Internal {
		struct DictionaryShared;

		struct Dictionary {
			std::vector<std::tuple<size_t, OwcaValue, OwcaValue>> values;
			size_t elements = 0;
			size_t mask = 0;
			size_t version = 0;
			OwcaVM vm;
			const bool is_map = true;

			Dictionary(OwcaVM vm, bool is_map) : vm(vm), is_map(is_map) {}

			std::tuple<size_t, size_t, bool> find_place(OwcaValue key, size_t hash) const;
			std::tuple<size_t, size_t, bool> find_place(OwcaValue key) const;

			void try_rehash();
			OwcaValue &item(OwcaValue key);
			OwcaValue read(OwcaValue key);
			void write(OwcaValue key, OwcaValue value);
			void delete_pos(size_t pos, bool rehash = true);
			OwcaValue pop(OwcaValue key, std::optional<OwcaValue> default_value = std::nullopt);
			std::optional<std::pair<const OwcaValue*, OwcaValue*>> find(OwcaValue key);
			std::optional<std::pair<const OwcaValue*, const OwcaValue*>> find(OwcaValue key) const;
			OwcaValue get_or_default(OwcaValue key, OwcaValue default_value);
			OwcaValue set_default(OwcaValue key, OwcaValue default_value);
			Generator iter_keys() const;
			Generator iter_values() const;
			Generator iter_items() const;
			void clone_to(Dictionary &) const;
			
			void union_with(Dictionary &other);
			void intersection_with(Dictionary &other);
			void difference_with(Dictionary &other);

			size_t next(size_t pos = (size_t)-1) const;
			std::pair<const OwcaValue *, OwcaValue *> read(size_t pos);
			std::pair<const OwcaValue *, const OwcaValue *> read(size_t pos) const;

			std::string_view type() const;
			std::string to_string() const;
			
			friend void gc_mark_value(OwcaVM vm, GenerationGC gc, const Dictionary &);
		};

		struct DictionaryShared : public AllocationBase {
			static constexpr const Kind object_kind = Kind::Map;

			Dictionary dict;

			DictionaryShared(OwcaVM vm) : dict(vm, true) {}
			
			DictionaryShared *clone() const;
			std::string_view type() const override {
				return dict.type();
			}
			std::string to_string() const override {
				return dict.to_string();
			}
			void gc_mark(OwcaVM vm, GenerationGC generation_gc) const override {
				gc_mark_value(vm, generation_gc, dict);
			}
		};

		struct SetShared : public AllocationBase {
			static constexpr const Kind object_kind = Kind::Set;

			Dictionary dict;

			SetShared(OwcaVM vm) : dict(vm, false) {}
			
			SetShared *clone() const;
			std::string_view type() const override {
				return dict.type();
			}
			std::string to_string() const override {
				return dict.to_string();
			}
			void gc_mark(OwcaVM vm, GenerationGC generation_gc) const override {
				gc_mark_value(vm, generation_gc, dict);
			}
		};
	}
}

#endif
