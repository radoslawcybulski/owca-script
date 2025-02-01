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

			Dictionary(OwcaVM vm) : vm(vm) {}

			std::tuple<size_t, size_t, bool> find_place(const OwcaValue &key, size_t hash) const;
			std::tuple<size_t, size_t, bool> find_place(const OwcaValue &key) const;

			void try_rehash();
			OwcaValue &item(const OwcaValue &key);
			const OwcaValue &read(const OwcaValue &key);
			void write(const OwcaValue &key, OwcaValue value);
			OwcaValue pop(const OwcaValue &key);
			std::optional<std::pair<const OwcaValue*, OwcaValue*>> find(const OwcaValue &key);
			std::optional<std::pair<const OwcaValue*, const OwcaValue*>> find(const OwcaValue &key) const;

			size_t next(size_t pos = (size_t)-1) const;
			std::pair<const OwcaValue *, OwcaValue *> read(size_t pos);
			std::pair<const OwcaValue *, const OwcaValue *> read(size_t pos) const;

			std::string_view type() const;
			std::string to_string() const;
			void gc_mark(VM& vm, GenerationGC generation_gc);
		};

		struct DictionaryShared : public AllocationBase {
			Dictionary dict;

			DictionaryShared(OwcaVM vm) : dict(vm) {}
			
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
