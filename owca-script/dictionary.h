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

			std::tuple<size_t, size_t, bool> find_place(OwcaVM &, const OwcaValue &key, size_t hash) const;
			std::tuple<size_t, size_t, bool> find_place(OwcaVM &, const OwcaValue &key) const;

			void try_rehash();
			OwcaValue &item(OwcaVM &, const OwcaValue &key);
			const OwcaValue &read(OwcaVM &, const OwcaValue &key);
			void write(OwcaVM &, const OwcaValue &key, OwcaValue value);
			OwcaValue pop(OwcaVM &, const OwcaValue &key);
			std::optional<std::pair<OwcaValue, OwcaValue>> find(OwcaVM &, const OwcaValue &key) const;

			struct Iterator {
				unsigned int pos = 0;
			};
			std::optional<std::pair<OwcaValue, OwcaValue>> read_and_update_iterator(Iterator&) const;

			std::string_view type() const;
			std::string to_string() const;
			void gc_mark(VM& vm, GenerationGC generation_gc);
		};

		struct DictionaryShared : public AllocationBase {
			Dictionary dict;

			std::string_view type() const override {
				return dict.type();
			}
			std::string to_string() const override {
				return dict.to_string();
			}
			void gc_mark(VM& vm, GenerationGC generation_gc) override {
				return dict.gc_mark(vm, generation_gc);
			}
		};
	}
}

#endif
