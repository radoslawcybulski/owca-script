#ifndef RC_OWCA_SCRIPT_OBJECT_H
#define RC_OWCA_SCRIPT_OBJECT_H

#include "stdafx.h"
#include "allocation_base.h"
#include "identifier_index.h"
#include "owca_code.h"
#include "owca_value.h"
#include "line.h"

namespace OwcaScript {
	class OwcaVM;
	class OwcaClass;
	class OwcaFunctions;
	class NativeClassInterface;

	namespace Internal {
		struct RuntimeFunction;
		struct RuntimeFunctions;
		struct Object;

		template <typename V> struct ValuesMap {
			struct ArrayStorage {
				static constexpr const size_t S = 16;
				std::array<IdentifierIndex, S> keys;
				std::array<V, S> values;
				size_t count = 0;
			};
			std::variant<ArrayStorage, std::unordered_map<IdentifierIndex, V>> values;

			template <typename F> void for_each(const F &f) {
				visit_variant(values, [&](ArrayStorage &arr) {
					for (size_t i = 0; i < arr.count; ++i) {
						f(arr.keys[i], arr.values[i]);
					}
				}, [&](std::unordered_map<IdentifierIndex, V> &map) {
					for (auto &it : map) {
						f(it.first, it.second);
					}
				});
			}
			template <typename F> void for_each(const F &f) const {
				visit_variant(values, [&](const ArrayStorage &arr) {
					for (size_t i = 0; i < arr.count; ++i) {
						f(arr.keys[i], arr.values[i]);
					}
				}, [&](const std::unordered_map<IdentifierIndex, V> &map) {
					for (auto &it : map) {
						f(it.first, it.second);
					}
				});
			}

			void set(IdentifierIndex key, V value) {
				visit_variant(values, [&](ArrayStorage &arr) {
					for (size_t i = 0; i < arr.count; ++i) {
						if (arr.keys[i] == key) {
							arr.values[i] = std::move(value);
							return;
						}
					}
					if (arr.count < ArrayStorage::S) {
						arr.keys[arr.count] = key;
						arr.values[arr.count] = std::move(value);
						++arr.count;
						return;
					}
					std::unordered_map<IdentifierIndex, V> map;
					for (size_t i = 0; i < arr.count; ++i) {
						map[arr.keys[i]] = std::move(arr.values[i]);
					}
					map[key] = std::move(value);
					values = std::move(map);
				}, [&](std::unordered_map<IdentifierIndex, V> &map) {
					map[key] = std::move(value);
				});
			}
			std::pair<V*, bool> emplace(IdentifierIndex key, V value) {
				return visit_variant(values, [&](ArrayStorage &arr) {
					for (size_t i = 0; i < arr.count; ++i) {
						if (arr.keys[i] == key) {
							return std::make_pair(&arr.values[i], false);
						}
					}
					if (arr.count < ArrayStorage::S) {
						arr.keys[arr.count] = key;
						arr.values[arr.count] = std::move(value);
						++arr.count;
						return std::make_pair(&arr.values[arr.count - 1], true);
					}
					std::unordered_map<IdentifierIndex, V> map;
					for (size_t i = 0; i < arr.count; ++i) {
						map[arr.keys[i]] = std::move(arr.values[i]);
					}
					auto &r = map[key];
					r = std::move(value);
					values = std::move(map);
					return std::make_pair(&r, true);
				}, [&](std::unordered_map<IdentifierIndex, V> &map) {
					auto it = map.insert({ key, std::move(value) });
					return std::make_pair(&it.first->second, it.second);
				});
			}
			V *find(IdentifierIndex key) {
				return visit_variant(values, [&](ArrayStorage &arr) -> V* {
					for (size_t i = 0; i < arr.count; ++i) {
						if (arr.keys[i] == key) {
							return &arr.values[i];
						}
					}
					return nullptr;
				}, [&](std::unordered_map<IdentifierIndex, V> &map) -> V* {
					auto it = map.find(key);
					if (it == map.end()) return nullptr;
					return &it->second;
				});
			}
		};
		struct Class : public AllocationBase {
			static constexpr const Kind object_kind = Kind::Class;

			ValuesMap<std::variant<Class*, RuntimeFunctions*>> values;
			const std::string_view name, full_name;
			OwcaCode code;
			Line fileline;
			std::unordered_set<Class*> all_base_classes;
			std::vector<Class*> base_classes;
			std::vector<Class*> lookup_order;
			std::vector<RuntimeFunction*> runtime_functions;
			std::vector<IdentifierIndex> runtime_variables;
			std::vector<std::tuple<UserClassTokenPtr, Class*, size_t, size_t>> native_storage_pointers;
			std::shared_ptr<NativeClassInterface> native;
			std::optional<UserClassTokenPtr> native_token;
			size_t native_storage = 0;
			size_t native_storage_total = 0;
			std::function<OwcaValue()> allocator_override;
			bool reload_self = false;
			bool all_variables = false;

			std::string_view type() const override { return "Class"; }
			std::string to_string() const override;
			void gc_mark(GenerationGC generation_gc) const override;

			void initialize_set_native_class_info(UserClassTokenPtr token, size_t sz);
			void initialize_add_base_class(OwcaClass b);
			void initialize_add_function(OwcaFunctions f);
			void initialize_add_variable(IdentifierIndex name);
			void initialize_set_all_variables();
			void finalize_initializing();
			char* native_storage_ptr(Object *) const;
			const char* native_storage_ptr(const Object *) const;

			Class(Line line, std::string_view type, std::string_view full_name, OwcaCode code);

            OwcaValue bound_function_self_object() override;
		};

		struct Object : public AllocationBase {
			static constexpr const Kind object_kind = Kind::User;

			Class* type_;
			ValuesMap<OwcaValue> values;

			Object(Class* type);
			~Object();

			std::string_view type() const override;
			std::string to_string() const override;
			void gc_mark(GenerationGC generation_gc) const override;
			std::span<char> native_storage_raw(UserClassTokenPtr token) {
				for(auto &p : type_->native_storage_pointers) {
					if (std::get<0>(p) == token) {
						return { type_->native_storage_ptr(this) + std::get<2>(p), std::get<3>(p) };
					}
				}
				return { (char*)nullptr, 0 };
			}

            OwcaValue bound_function_self_object() override;
		};

		void gc_mark_value(GenerationGC ggc, const AllocationBase* ptr);
	}
}

#endif
