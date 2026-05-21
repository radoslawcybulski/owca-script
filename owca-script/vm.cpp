#include "stdafx.h"
#include "owca-script/owca_tuple.h"
#include "vm.h"
#include "owca_value.h"
#include "runtime_function.h"
#include "owca_code.h"
#include "owca_vm.h"
#include "owca_value.h"
#include "owca_namespace.h"
#include "dictionary.h"
#include "array.h"
#include "tuple.h"
#include "object.h"
#include "owca_iterator.h"
#include "string.h"
#include "owca_variable.h"
#include "exception.h"
#include "owca_exception.h"
#include "iterator.h"
#include "range.h"
#include "executor.h"
#include "namespace.h"

namespace OwcaScript::Internal {
	VM::VM() : root_allocated_memory(this) {
		root_allocated_memory.prev = root_allocated_memory.next = &root_allocated_memory;
		executor = std::make_unique<Executor>(this);
		initialize_builtins();
		auto vm = OwcaVM{ this };
		empty_tuple = create_tuple(std::vector<OwcaValue>{}).internal_value();
		empty_string = allocate<String>(0, 0u);
	}
	VM::~VM() {
		empty_tuple = nullptr;
		empty_string = nullptr;
		executor.reset();
		AllocationBase *valid = &root_allocated_memory;
		while (valid->next != &root_allocated_memory) {
			auto obj = valid->next;
			valid->next = obj->next;
			obj->next->prev = valid;
			obj->~AllocationBase();
			std::free(obj);
		}
	}

	struct VM::BuiltinProvider : public NativeCodeProvider {
		static OwcaValue range_init1(const OwcaVM &vm, OwcaRange self, Number upper) {
			self.internal_object()->from = 0;
			self.internal_object()->to = upper;
			self.internal_object()->step = 1;
			return self;
		}
		static OwcaValue range_init2(const OwcaVM &vm, OwcaRange self, Number lower, Number upper) {
			self.internal_object()->from = lower;
			self.internal_object()->to = upper;
			self.internal_object()->step = 1;
			return self;
		}
		static OwcaValue range_init3(const OwcaVM &vm, OwcaRange self, Number lower, Number upper, Number step) {
			self.internal_object()->from = lower;
			self.internal_object()->to = upper;
			self.internal_object()->step = step;
			if (step == 0) {
				VM::get(vm).throw_range_step_is_zero();
			}
			return self;
		}
		static OwcaValue range_lower(const OwcaVM &vm, OwcaRange r) {
			return r.lower();
		}
		static OwcaValue range_upper(const OwcaVM &vm, OwcaRange r) {
			return r.upper();
		}
		static OwcaValue range_size(const OwcaVM &vm, OwcaRange r) {
			return r.size();
		}
		static OwcaValue range_step(const OwcaVM &vm, OwcaRange r) {
			return r.step();
		}
		static Generator range_iter(const OwcaVM &vm, OwcaRange o) {
			return o.internal_object()->iter(vm);
		}
		static OwcaValue iterator_completed(const OwcaVM &vm, OwcaIterator oi) {
			return oi.completed();
		}
		static OwcaValue iterator_next(const OwcaVM &vm, OwcaIterator oi) {
			return VM::get(vm).resume_generator(oi).value_or(OwcaEmpty{});
		}
		static OwcaValue bool_init(const OwcaVM &vm, OwcaValue, OwcaValue r) {
			return VM::get(vm).calculate_if_true(r);
		}
		static OwcaValue float_init(const OwcaVM &vm, OwcaValue, OwcaValue r) {
			return r.visit(
				[&](bool value) -> Number {
					return value ? 1.0f : 0.0f;
				},
				[&](Number value) -> Number {
					return value;
				},
				[&](OwcaString source) -> Number {
					auto text = source.text();
					unsigned int base = 0;

					if (text.size() > 1) {
						if (text.substr(0, 2) == "0x" || text.substr(0, 2) == "0X") {
							base = 16;
							text = text.substr(2);
						}
						else if (text.substr(0, 2) == "0b" || text.substr(0, 2) == "0B") {
							base = 2;
							text = text.substr(2);
						}
						else if (text.substr(0, 2) == "0o" || text.substr(0, 2) == "0O") {
							base = 8;
							text = text.substr(2);
						}
					}
			
					if (base > 0) {
						long long int value = 0;
						auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value, base);
			
						if (ec == std::errc() && ptr == text.data() + text.size()) {
							return (Number)value;
						}
						else if (ec == std::errc() || ec == std::errc::invalid_argument) {
							VM::get(vm).throw_overflow(std::format("`{}` is not a valid number", source.text()));
						}
						else if (ec == std::errc::result_out_of_range) {
							VM::get(vm).throw_overflow(std::format("`{}` doesn't fit in range of allowed values ({} -> {}) for used integer (long long int) type",
								source.text(), std::numeric_limits<long long int>::min(), std::numeric_limits<long long int>::max()));
						}
						else {
							assert(false);
						}
					}
					else {
						Number value = 0;
						auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
			
						if (ec == std::errc() && ptr == text.data() + text.size()) {
							return value;
						}
						else if (ec == std::errc() || ec == std::errc::invalid_argument) {
							VM::get(vm).throw_overflow(std::format("`{}` is not a valid number", source.text()));
						}
						else if (ec == std::errc::result_out_of_range) {
							VM::get(vm).throw_overflow(std::format("`{}` doesn't fit in range of allowed values for given OwcaFloatInternal type", source.text()));
						}
						else {
							assert(false);
						}
					}
					return {};
				},
				[&](const auto &) -> Number {
					VM::get(vm).throw_cant_convert_to_float(r.type());
				}
			);
		}
		static OwcaValue string_init(const OwcaVM &vm, OwcaValue, OwcaValue r) {
			return r.visit(
				[&](OwcaEmpty o) -> OwcaValue { return vm.create_string("nul"); },
				[&](OwcaCompleted o) -> OwcaValue { return vm.create_string("completed"); },
				[&](OwcaRange o) -> OwcaValue { return vm.create_string(o.to_string()); },
				[&](Number o) -> OwcaValue { return vm.create_string(std::format("{}", o)); },
				[&](bool o) -> OwcaValue { return vm.create_string(o ? "true" : "false"); },
				[&](OwcaString o) -> OwcaValue { return o; },
				[&](OwcaFunctions s) -> OwcaValue { return vm.create_string(std::format("function {}", s.internal_value()->full_name)); },
				[&](OwcaMap s) -> OwcaValue { return vm.create_string(s.to_string()); },
				[&](OwcaClass s) -> OwcaValue { return vm.create_string(s.to_string()); },
				[&](OwcaObject s) -> OwcaValue { return vm.create_string(s.to_string()); },
				[&](OwcaArray s) -> OwcaValue { return vm.create_string(s.to_string()); },
				[&](OwcaTuple s) -> OwcaValue { return vm.create_string(s.to_string()); },
				[&](OwcaSet s) -> OwcaValue { return vm.create_string(s.to_string()); },
				[&](OwcaException s) -> OwcaValue { return vm.create_string(s.to_string()); },
				[&](OwcaIterator s) -> OwcaValue { return vm.create_string("iterator"); },
				[&](OwcaNamespace s) -> OwcaValue { return vm.create_string(s.to_string()); }
				);
		}
		static OwcaValue string_size(const OwcaVM &vm, OwcaString r) {
			return r.internal_value()->size();
		}
		static Generator string_iter(const OwcaVM &vm, OwcaString o) {
			for(auto i = 0u; i < o.size(); ++i) {
				co_yield o.substr(i, 1);
			}
		}
		static OwcaValue function_bind(const OwcaVM &vm, OwcaFunctions r, OwcaValue bind) {
			return r.bind(bind);
		}
		static OwcaValue function_bound_value(const OwcaVM &vm, OwcaFunctions r) {
			return r.self().value_or(OwcaValue{});
		}
		static OwcaValue map_has_key(const OwcaVM &vm, OwcaMap self, OwcaValue key) {
			return self.has_key(key);
		}
		static OwcaValue map_pop(const OwcaVM &vm, OwcaMap self, OwcaValue key) {
			return self.pop(key);
		}
		static OwcaValue map_pop_or_default_2(const OwcaVM &vm, OwcaMap self, OwcaValue key) {
			return self.pop_or_default(key, {});
		}
		static OwcaValue map_pop_or_default_3(const OwcaVM &vm, OwcaMap self, OwcaValue key, OwcaValue default_value) {
			return self.pop_or_default(key, default_value);
		}
		static OwcaValue map_get_or_default(const OwcaVM &vm, OwcaMap self, OwcaValue key, OwcaValue default_value) {
			return self.get_or_default(key, default_value);
		}
		static OwcaValue map_set_default(const OwcaVM &vm, OwcaMap self, OwcaValue key, OwcaValue default_value) {
			return self.set_default(key, default_value);
		}		
		static Generator map_keys(const OwcaVM &vm, OwcaMap self) {
			return self.keys();
		}
		static Generator map_values(const OwcaVM &vm, OwcaMap self) {
			return self.values();
		}
		static Generator map_items(const OwcaVM &vm, OwcaMap self) {
			return self.items();
		}
		static OwcaValue map_init(const OwcaVM &vm, OwcaMap self, OwcaValue r) {
			r.visit(
				[&](OwcaArray o) {
					for(auto v : o.internal_value()->values) {
						auto pair = Internal::VM::get(vm).unpack_two_elements_or_raise(v);
						self[pair.first] = pair.second;
					}
				},
				[&](OwcaTuple o) {
					for(auto v : o.internal_value()->values) {
						auto pair = Internal::VM::get(vm).unpack_two_elements_or_raise(v);
						self[pair.first] = pair.second;
					}
				},
				[&](OwcaMap o) {
					o.internal_value()->dict.clone_to(self.internal_value()->dict);
				},
				[&](OwcaIterator o) {
					for(auto v : o) {
						auto pair = Internal::VM::get(vm).unpack_two_elements_or_raise(v);
						self[pair.first] = pair.second;
					}
				},
				[&](OwcaObject o) {
					auto iter = VM::get(vm).create_iterator(o);
					while(auto v = iter.next()) {
						auto pair = Internal::VM::get(vm).unpack_two_elements_or_raise(*v);
						self[pair.first] = pair.second;
					}
				},
				[&](const auto &) {
					VM::get(vm).throw_wrong_type(std::format("can't create a map from {}", r.type()));
				}
			);
			return {};
		}
		static OwcaValue map_size(const OwcaVM &vm, OwcaMap r) {
			return r.size();
		}
		static Generator map_iter(const OwcaVM &vm, OwcaMap o) {
			for(auto v : o) {
				co_yield v.first;
			}
		}
		static OwcaValue set_init(const OwcaVM &vm, OwcaSet self, OwcaValue r) {
			r.visit(
				[&](OwcaArray o) {
					for(auto v : o) {
						self.add(v);
					}
				},
				[&](OwcaTuple o) {
					for(auto v : o) {
						self.add(v);
					}
				},
				[&](OwcaMap o) {
					for(auto val : o) {
						self.add(val.first);
					}
				},
				[&](OwcaSet o) {
					o.internal_value()->dict.clone_to(self.internal_value()->dict);
				},
				[&](OwcaString o) {
					for(auto i = 0u; i < o.size(); ++i) {
						self.add(o.substr(i, i + 1));
					}
				},
				[&](OwcaObject o) {
					auto iter = VM::get(vm).create_iterator(o);
					while(auto v = iter.next()) {
						self.add(*v);
					}
				},
				[&](OwcaIterator o) {
					for(auto v : o) {
						self.add(v);
					}
				},
				[&](const auto &) {
					VM::get(vm).throw_wrong_type(std::format("can't create a set from {}", r.type()));
				}
			);
			return {};
		}
		static OwcaValue set_union_with(const OwcaVM &vm, OwcaSet self, OwcaSet other) {
			return self | other;
		}
		static OwcaValue set_intersection_with(const OwcaVM &vm, OwcaSet self, OwcaSet other) {
			return self & other;
		}
		static OwcaValue set_difference_with(const OwcaVM &vm, OwcaSet self, OwcaSet other) {
			return self - other;
		}
		static OwcaValue set_add(const OwcaVM &vm, OwcaSet self, OwcaValue v) {
			self.add(v);
			return {};
		}
		static OwcaValue set_remove(const OwcaVM &vm, OwcaSet self, OwcaValue v) {
			self.remove(v);
			return {};
		}
		static OwcaValue set_size(const OwcaVM &vm, const OwcaSet &r) {
			return r.size();
		}
		static Generator set_iter(const OwcaVM &vm, OwcaSet o) {
			for(auto v : o) {
				co_yield v;
			}
		}
		static OwcaValue class_name(const OwcaVM &vm, OwcaClass r) {
			return vm.create_string(r.internal_value()->name);
		}
		static OwcaValue class_full_name(const OwcaVM &vm, OwcaClass r) {
			return vm.create_string(r.internal_value()->full_name);
		}
		static OwcaValue array_init(const OwcaVM &vm, OwcaArray self, OwcaValue r) {
			r.visit(
				[&](OwcaArray o) {
					self.internal_value()->values = o.internal_value()->values;
				},
				[&](OwcaTuple o) {
					self.internal_value()->values = { o.internal_value()->values.begin(), o.internal_value()->values.end() };
				},
				[&](OwcaMap o) {
					for(const auto &val : o) {
						self.internal_value()->values.push_back(val.first);
					}
				},
				[&](OwcaSet o) {
					for(const auto &val : o) {
						self.internal_value()->values.push_back(val);
					} 
				},
				[&](OwcaString o) {
					for(auto i = 0u; i < o.size(); ++i) {
						self.internal_value()->values.push_back(vm.create_string(o.text().substr(i, 1)));
					}
				},
				[&](OwcaObject o) {
					auto iter = VM::get(vm).create_iterator(o);
					while(auto v = iter.next()) {
						self.internal_value()->values.push_back(*v);
					}
				},
				[&](OwcaIterator o) {
					for(auto v : o) {
						self.internal_value()->values.push_back(v);
					}
				},
				[&](const auto &) {
					VM::get(vm).throw_wrong_type(std::format("can't create an array from {}", r.type()));
				}
			);
			return {};
		}
		static OwcaValue array_size(const OwcaVM &vm, OwcaArray self) {
			return self.internal_value()->values.size();
		}
		static Generator array_iter(const OwcaVM &vm, OwcaArray o) {
			for(auto i = 0u; i < o.size(); ++i) {
				co_yield o[i];
			}
		}
		
		static OwcaValue array_push_back(const OwcaVM &vm, OwcaArray self, OwcaValue v) {
			self.push_back(v);
			return {};
		}
		static OwcaValue array_push_front(const OwcaVM &vm, OwcaArray self, OwcaValue v) {
			self.push_front(v);
			return {};
		}
		static OwcaValue array_pop_back(const OwcaVM &vm, OwcaArray self) {
			return self.pop_back();
		}
		static OwcaValue array_pop_front(const OwcaVM &vm, OwcaArray self) {
			return self.pop_front();
		}
		static OwcaValue array_sort(const OwcaVM &vm, OwcaArray self) {
			auto values = self.internal_value()->values;
			std::sort(values.begin(), values.end(), [&](auto a, auto b) {
				return VM::get(vm).compare_values_less(a, b);
			});
			return OwcaArray{ VM::get(vm).allocate<Array>(0, std::move(values)) };
		}
		static OwcaValue tuple_init(const OwcaVM &vm, OwcaTuple self, OwcaValue r) {
			r.visit(
				[&](OwcaArray o) {
					self.internal_value()->values = { o.internal_value()->values.begin(), o.internal_value()->values.end() };
				},
				[&](OwcaTuple o) {
					self.internal_value()->values = o.internal_value()->values;
				},
				[&](OwcaMap o) {
					for(const auto &val : o) {
						self.internal_value()->values.push_back(val.first);
					} 
				},
				[&](OwcaSet o) {
					for(const auto &val : o) {
						self.internal_value()->values.push_back(val);
					}
				},
				[&](OwcaString o) {
					self.internal_value()->values.reserve(o.size());
					for(auto i = 0u; i < o.size(); ++i) {
						self.internal_value()->values.push_back(vm.create_string(o.text().substr(i, 1)));
					}
				},
				[&](OwcaObject o) {
					auto iter = VM::get(vm).create_iterator(o);
					while(auto v = iter.next()) {
						self.internal_value()->values.push_back(*v);
					}
				},
				[&](OwcaIterator o) {
					for(auto v : o) {
						self.internal_value()->values.push_back(v);
					}
				},
				[&](const auto &) {
					VM::get(vm).throw_wrong_type(std::format("can't create an array from {}", r.type()));
				}
			);
			return {};
		}
		static OwcaValue tuple_size(const OwcaVM &vm, OwcaTuple self) {
			return self.internal_value()->values.size();
		}
		static Generator tuple_iter(const OwcaVM &vm, OwcaTuple o) {
			for(auto i = 0u; i < o.size(); ++i) {
				co_yield o[i];
			}
		}
		static OwcaValue tuple_sort(const OwcaVM &vm, OwcaTuple self) {
			auto values = self.internal_value()->values;
			std::sort(values.begin(), values.end(), [&](auto a, auto b) {
				return VM::get(vm).compare_values_less(a, b);
			});
			return OwcaTuple{ VM::get(vm).allocate<Tuple>(0, std::move(values)) };
		}
		static OwcaValue exception_init(const OwcaVM &vm, OwcaException self, OwcaString msg) {
			VM::get(vm).initialize_exception_object(*self.internal_value());
			self.internal_value()->message = std::string{ msg.text() };
			return {};
		}
		static OwcaValue exception_count(const OwcaVM &vm, OwcaException self) {
			return self.count();
		}
		static OwcaValue exception_message(const OwcaVM &vm, OwcaException self) {
			return vm.create_string(self.message());
		}
		static OwcaValue exception_line(const OwcaVM &vm, OwcaException self, Number index) {
			assert(self.count() > 0);
			auto ind = std::floor(index);
			if (ind < 0 || ind >= self.count())
				VM::get(vm).throw_cant_call(std::format("frame index {} is out of range (0..{})", ind, self.count() - 1));
			return self.frame(ind).line;
			
		}
		static OwcaValue exception_filename(const OwcaVM &vm, OwcaException self, Number index) {
			assert(self.count() > 0);
			auto ind = std::floor(index);
			if (ind < 0 || ind >= self.count())
				VM::get(vm).throw_cant_call(std::format("frame index {} is out of range (0..{})", ind, self.count() - 1));
			return vm.create_string(self.frame(ind).filename);
		}
		static OwcaValue exception_function(const OwcaVM &vm, OwcaException self, Number index) {
			assert(self.count() > 0);
			auto ind = std::floor(index);
			if (ind < 0 || ind >= self.count())
				VM::get(vm).throw_cant_call(std::format("frame index {} is out of range (0..{})", ind, self.count() - 1));
			return vm.create_string(self.frame(ind).function);
		}

		static OwcaValue hash(const OwcaVM &vm, OwcaValue r) {
			return VM::get(vm).calculate_hash(r);
		}
		static OwcaValue print(const OwcaVM &vm, OwcaValue r) {
			std::cout << r.to_string() << "\n";
			return {};
		}
		static OwcaValue time(const OwcaVM &vm) {
			return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() / 1'000'000'000.0;
		}

		std::optional<Function> native_function(std::string_view full_name, std::optional<ClassToken> cls, FunctionToken token, std::span<const std::string_view> param_names) const override {
			if (full_name == "Range.__init__") {
				if (param_names.size() == 2) return adapt(range_init1);
				if (param_names.size() == 3) return adapt(range_init2);
				if (param_names.size() == 4) return adapt(range_init3);
			}
			if (full_name == "Range.lower") return adapt(range_lower);
			if (full_name == "Range.upper") return adapt(range_upper);
			if (full_name == "Range.step") return adapt(range_step);
			if (full_name == "Range.size") return adapt(range_size);
			if (full_name == "Iterator.completed") return adapt(iterator_completed);
			if (full_name == "Iterator.next") return adapt(iterator_next);
			if (full_name == "Bool.__init__") return adapt(bool_init);
			if (full_name == "Float.__init__") return adapt(float_init);
			if (full_name == "String.__init__") return adapt(string_init);
			if (full_name == "String.size") return adapt(string_size);
			if (full_name == "bound_value") return adapt(function_bound_value);
			if (full_name == "Function.bind") return adapt(function_bind);
			if (full_name == "Map.__init__") return adapt(map_init);
			if (full_name == "Map.size") return adapt(map_size);
			if (full_name == "Map.has_key") return adapt(map_has_key);
			if (full_name == "Map.pop") return adapt(map_pop);
			if (full_name == "Map.pop_or_default" && param_names.size() == 2) return adapt(map_pop_or_default_2);
			if (full_name == "Map.pop_or_default" && param_names.size() == 3) return adapt(map_pop_or_default_3);
			if (full_name == "Map.get_or_default") return adapt(map_get_or_default);
			if (full_name == "Map.set_default") return adapt(map_set_default);
			if (full_name == "Set.__init__") return adapt(set_init);
			if (full_name == "Set.size") return adapt(set_size);
			if (full_name == "Set.add") return adapt(set_add);
			if (full_name == "Set.remove") return adapt(set_remove);
			if (full_name == "Set.union_with") return adapt(set_union_with);
			if (full_name == "Set.intersection_with") return adapt(set_intersection_with);
			if (full_name == "Set.difference_with") return adapt(set_difference_with);
			if (full_name == "Class.name") return adapt(class_name);
			if (full_name == "Class.full_name") return adapt(class_full_name);
			if (full_name == "Array.__init__") return adapt(array_init);
			if (full_name == "Array.size") return adapt(array_size);
			if (full_name == "Array.sort") return adapt(array_sort);
			if (full_name == "Array.push_back") return adapt(array_push_back);
			if (full_name == "Array.push_front") return adapt(array_push_front);
			if (full_name == "Array.pop_back") return adapt(array_pop_back);
			if (full_name == "Array.pop_front") return adapt(array_pop_front);
			if (full_name == "Tuple.__init__") return adapt(tuple_init);
			if (full_name == "Tuple.size") return adapt(tuple_size);
			if (full_name == "Tuple.sort") return adapt(tuple_sort);
			if (full_name == "hash") return adapt(hash);
			if (full_name == "print") return adapt(print);
			if (full_name == "time") return adapt(time);
			if (full_name == "Exception.__init__") return adapt(exception_init);
			if (full_name == "Exception.count") return adapt(exception_count);
			if (full_name == "Exception.message") return adapt(exception_message);
			if (full_name == "Exception.line") return adapt(exception_line);
			if (full_name == "Exception.filename") return adapt(exception_filename);
			if (full_name == "Exception.function") return adapt(exception_function);
			return std::nullopt;
		}
		std::optional<GeneratorFunction> native_generator(std::string_view full_name, std::optional<ClassToken> cls, FunctionToken token, std::span<const std::string_view> param_names) const override {
			if (full_name == "Range.__iter__") return adapt(range_iter);
			if (full_name == "String.__iter__") return adapt(string_iter);
			if (full_name == "Map.__iter__") return adapt(map_iter);
			if (full_name == "Map.items") return adapt(map_items);
			if (full_name == "Map.keys") return adapt(map_keys);
			if (full_name == "Map.values") return adapt(map_values);
			if (full_name == "Set.__iter__") return adapt(set_iter);
			if (full_name == "Array.__iter__") return adapt(array_iter);
			if (full_name == "Tuple.__iter__") return adapt(tuple_iter);
			return std::nullopt;
		}
		std::shared_ptr<NativeClassInterface> native_class(std::string_view full_name, ClassToken token) const override {
			if (full_name == "Exception") return std::make_shared<NativeClassInterfaceSimpleImplementation<Exception>>();
			return nullptr;
		}
	};
	void VM::initialize_builtins()
	{
		auto code = std::string{ R"(
class Nul {
}
class Namespace {
}
class Iterator {
	function native completed(self);
	function native next(self);
}
class Range {
	function native __init__(self, upper);
	function native __init__(self, lower, upper);
	function native __init__(self, lower, upper, step);
	function native lower(self);
	function native upper(self);
	function native step(self);
	function native size(self);
	function native generator __iter__(self);
}
class Bool {
	function native __init__(self, value);
}
class Float {
	function native __init__(self, value);
}
class String {
	function native __init__(self, value);
	function native size(self);
	function native generator __iter__(self);
}
class Function {
	function native bind(self, value);
}
function native bound_value(func);
class Map {
	function native __init__(self, value);
	function native size(self);
	function native generator __iter__(self);
	function native has_key(self, key);
	function native pop(self, key);
	function native pop_or_default(self, key);
	function native pop_or_default(self, key, default);
	function native get_or_default(self, key, default);
	function native set_default(self, key, default);
	function native generator items(self);
	function native generator keys(self);
	function native generator values(self);
}
class Set {
	function native __init__(self, value);
	function native union_with(self, other);
	function native intersection_with(self, other);
	function native difference_with(self, other);
	function native add(self, value);
	function native remove(self, value);
	function native size(self);
	function native generator __iter__(self);
}
class Class {
	function native name(self);
	function native full_name(self);
}
class Array {
	function native __init__(self, value);
	function native size(self);
	function native sort(self);
	function native generator __iter__(self);
	function native push_back(self, v);
	function native push_front(self, v);
	function native pop_back(self);
	function native pop_front(self);
}
class Tuple {
	function native __init__(self, value);
	function native size(self);
	function native sort(self);
	function native generator __iter__(self);
}
class native Exception {
	function native __init__(self, message);
	function native count(self);
	function native message(self);
	function native line(self, index);
	function native filename(self, index);
	function native function(self, index);
}
class MathException(Exception) {}
class InvalidOperationException(Exception) {}

function native hash(value);
function native print(msg);
function native time();
)" };
		auto vm = OwcaVM{ this };
		auto code_compiled = vm.compile(builtin_filename, std::move(code), std::make_shared<BuiltinProvider>());
		auto builtins = executor->execute_code_block(code_compiled);

		auto read = [&](OwcaValue val) -> Class*{
			return val.as_class(vm).internal_value();
		};
		for(auto [key, value] : builtins) {
			builtin_identifiers.push_back(key);
			if (key == "Nul") {
				c_nul = read(value);
				c_nul->allocator_override = []() -> OwcaValue { return OwcaEmpty{}; };
			}
			else if (key == "Completed") {
				c_completed = read(value);
				c_completed->allocator_override = []() -> OwcaValue { return OwcaCompleted{}; };
			}
			else if (key == "Range") {
				c_range = read(value);
				c_range->allocator_override = [&]() -> OwcaValue {
					return create_range();
				};
			}
			else if (key == "Bool") {
				c_bool = read(value);
				c_bool->allocator_override = []() -> OwcaValue { return false; };
				c_bool->reload_self = true;
			}
			else if (key == "Float") {
				c_float = read(value);
				c_float->allocator_override = []() -> OwcaValue { return 0; };
				c_float->reload_self = true;
			}
			else if (key == "String") {
				c_string = read(value);
				c_string->allocator_override = [&]() -> OwcaValue { return OwcaString{ empty_string }; };
				c_string->reload_self = true;
			}
			else if (key == "Function") {
				c_function = read(value);
				c_function->allocator_override = [&]() -> OwcaValue {
					throw_not_callable(c_function->full_name);
				};
			}
			else if (key == "Iterator") {
				c_iterator = read(value);
				c_iterator->allocator_override = [&]() -> OwcaValue {
					throw_not_callable(c_iterator->full_name);
				};
			}
			else if (key == "Map") {
				c_map = read(value);
				c_map->allocator_override = [&]() -> OwcaValue {
					return create_map();
				};
			}
			else if (key == "Class") {
				c_class = read(value);
				c_class->allocator_override = [&]() -> OwcaValue {
					throw_not_callable(c_class->full_name);
				};
			}
			else if (key == "Array") {
				c_array = read(value);
				c_array->allocator_override = [&]() -> OwcaValue {
					return create_array({});
				};
			}
			else if (key == "Tuple") {
				c_tuple = read(value);
				c_tuple->allocator_override = [&]() -> OwcaValue {
					return create_tuple(std::vector<OwcaValue>{});
				};
			}
			else if (key == "Set") {
				c_set = read(value);
				c_set->allocator_override = [&]() -> OwcaValue {
					return create_set({});
				};
			}
			else if (key == "Exception") {
				c_exception = read(value);
			}
			else if (key == "MathException") {
				c_math_exception = read(value);
			}
			else if (key == "InvalidOperationException") {
				c_invalid_operation_exception = read(value);
			}
		}
	}
	void VM::initialize_exception_object(Exception &exc)
	{
		for(auto &s : executor->stacktrace) {
			auto rf = s.runtime_function;
			exc.frames.push_back({ .code = rf->code });
			exc.frames.back().line = rf->line(s.code_position);
			exc.frames.back().function = rf->full_name;
		}
	}
	Exception *VM::is_exception(OwcaObject obj) const
	{
		return obj.internal_value()->native_storage<Exception>(ClassToken{ c_exception });
	}	
	VM& VM::get(const OwcaVM &v)
	{
		return *v.vm;
	}

	void VM::throw_exception(Class *exc, std::string_view msg)
	{
		auto tmp = std::vector<OwcaValue>{};
		tmp.push_back(create_string_from_view(msg));
		auto res = execute_call(OwcaClass{ exc }, tmp);
		throw res.as_exception(this);
	}

	void VM::throw_dictionary_changed(bool is_dict)
	{
		executor->throw_dictionary_changed(is_dict);
	}
	void VM::throw_not_implemented(std::string_view msg)
	{
		executor->throw_not_implemented(msg);
	}
	void VM::throw_range_step_is_zero()
	{
		executor->throw_range_step_is_zero();
	}
	void VM::throw_division_by_zero()
	{
		executor->throw_division_by_zero();
	}

	void VM::throw_mod_division_by_zero()
	{
		executor->throw_mod_division_by_zero();
	}

	void VM::throw_cant_convert_to_float_message(std::string_view msg)
	{
		executor->throw_cant_convert_to_float_message(msg);
	}

	void VM::throw_cant_convert_to_float(std::string_view type)
	{
		executor->throw_cant_convert_to_float_message(std::format("can't convert value of type `{}` to floating point", type));
	}

	void VM::throw_cant_convert_to_integer(Number val)
	{
		executor->throw_cant_convert_to_integer(val);
	}

	void VM::throw_cant_convert_to_integer(std::string_view type)
	{
		executor->throw_cant_convert_to_integer(type);
	}

	void VM::throw_not_a_number(std::string_view type)
	{
		executor->throw_not_a_number(type);
	}

	void VM::throw_overflow(std::string_view msg)
	{
		executor->throw_overflow(msg);
	}

	void VM::throw_range_step_must_be_one_in_left_side_of_write_assign()
	{
		executor->throw_range_step_must_be_one_in_left_side_of_write_assign();
	}

	void VM::throw_cant_compare(CompareKind kind, std::string_view left, std::string_view right)
	{
		executor->throw_cant_compare(kind, left, right);
	}

	void VM::throw_string_too_large(size_t size)
	{
		executor->throw_string_too_large(size);
	}
	void VM::throw_index_out_of_range(std::string msg)
	{
		executor->throw_index_out_of_range(msg);
	}

	void VM::throw_value_not_indexable(std::string_view type, std::string_view key_type)
	{
		executor->throw_value_not_indexable(type, key_type);
	}

	void VM::throw_missing_member(std::string_view type, std::string_view ident)
	{
		executor->throw_missing_member(type, ident);
	}

	void VM::throw_cant_call(std::string_view msg)
	{
		executor->throw_cant_call(msg);
	}

	void VM::throw_not_callable(std::string_view type)
	{
		executor->throw_not_callable(type);
	}
	
	void VM::throw_not_callable_wrong_number_of_params(std::string_view type, unsigned int params)
	{
		executor->throw_not_callable_wrong_number_of_params(type, params);
	}

	void VM::throw_wrong_type(std::string_view type, std::string_view expected)
	{
		executor->throw_wrong_type(type, expected);
	}

	void VM::throw_wrong_type(std::string_view msg)
	{
		executor->throw_wrong_type(msg);
	}

	void VM::throw_unsupported_operation_2(std::string_view oper, std::string_view left, std::string_view right)
	{
		executor->throw_unsupported_operation_2(oper, left, right);
	}

	void VM::throw_invalid_operand_for_mul_string(std::string_view type, std::string_view val)
	{
		executor->throw_invalid_operand_for_mul_string(type, val);
	}

	void VM::throw_missing_key(std::string_view key)
	{
		executor->throw_missing_key(key);
	}

	void VM::throw_not_hashable(std::string_view type)
	{
		executor->throw_not_hashable(type);
	}

	void VM::throw_value_cant_have_fields(std::string_view type)
	{
		executor->throw_value_cant_have_fields(type);
	}

	void VM::throw_missing_native(std::string_view msg)
	{
		executor->throw_missing_native(msg);
	}

	void VM::throw_not_iterable(std::string_view msg)
	{
		executor->throw_not_iterable(msg);
	}

	void VM::throw_readonly(std::string_view msg)
	{
		executor->throw_readonly(msg);
	}

	void VM::throw_cant_return_value_from_generator()
	{
		executor->throw_cant_return_value_from_generator();
	}

	void VM::throw_container_is_empty()
	{
		executor->throw_container_is_empty();
	}
	std::pair<OwcaValue, OwcaValue> VM::unpack_two_elements_or_raise(OwcaValue val) {
		return val.visit(
			[&](OwcaArray o) {
				if (o.internal_value()->values.size() != 2) {
					throw_exception(c_invalid_operation_exception, std::format("expected container with two elements, got {}", o.internal_value()->values.size()));
				}
				return std::make_pair(o[0], o[1]);
			},
			[&](OwcaTuple o) {
				if (o.internal_value()->values.size() != 2) {
					throw_exception(c_invalid_operation_exception, std::format("expected container with two elements, got {}", o.internal_value()->values.size()));
				}
				return std::make_pair(o[0], o[1]);
			},
			[&](OwcaIterator o) {
				std::array<OwcaValue, 2> values;
				size_t count = 0;
				for(auto v : o) {
					if (count >= 2) {
						throw_exception(c_invalid_operation_exception, "expected container with two elements, got more");
					}
					values[count++] = v;
				}
				if (count < 2) {
					throw_exception(c_invalid_operation_exception, std::format("expected container with two elements, got {}", count));
				}
				return std::make_pair(values[0], values[1]);
			},
			[&](const auto &) -> std::pair<OwcaValue, OwcaValue> {
				throw_exception(c_invalid_operation_exception, std::format("expected container with two elements, got {}", val.type()));
			}
		);
	}
	OwcaValue VM::member(OwcaValue val, std::string_view key)
	{
		auto v = try_member(val, key);
		if (!v) {
			throw_missing_member(val.type(), key);
		}
		return *v;
	}

	std::optional<OwcaValue> VM::try_member(OwcaValue val, std::string_view key)
	{
		auto vm = OwcaVM{ this };
		OwcaValue tmp;
		auto read_member = [&](Class *cls) -> OwcaValue * {
			auto it = cls->values.find(key);
			if (it == cls->values.end()) return nullptr;
			return visit_variant(it->second,
				[&](RuntimeFunctions *rf) -> OwcaValue * {
					tmp = OwcaFunctions{ rf };
					return &tmp;
				},
				[&](Class* var) -> OwcaValue * { return nullptr; }
				);
		};
		bool bind_if_needed = true;
		auto read_from_class = [&](Class *cls, Internal::Object *obj) -> OwcaValue* {
			auto it2 = cls->values.find(key);
			if (it2 != cls->values.end()) {
				return visit_variant(it2->second,
					[&](RuntimeFunctions *rf) -> OwcaValue * {
						tmp = OwcaFunctions{ rf };
						return &tmp;
					},
					[&](Class* var) -> OwcaValue * {
						assert(obj);
						auto native_storage = obj->native_storage_raw(ClassToken{ var });
						auto got = var->native->get_member(this, key, native_storage, tmp);
						return got ? &tmp : nullptr;
					});
			}
			return nullptr;
		};
		auto read_from_object = [&](Internal::Object *obj) -> OwcaValue* {
			auto it = obj->values.find(key);
			if (it != obj->values.end()) {
				bind_if_needed = false;
				return &it->second;
			}

			return read_from_class(obj->type_, obj);
		};
		auto read_from_nspace = [&](Internal::Namespace *obj) -> OwcaValue* {
			auto it = obj->identifier_to_global_index.find(key);
			if (it != obj->identifier_to_global_index.end()) {
				bind_if_needed = false;
				return &obj->globals[it->second];
			}

			return read_from_class(c_namespace, nullptr);
		};
		auto v = val.visit(
			[&](OwcaEmpty o) -> OwcaValue* { return read_member(c_nul); },
			[&](OwcaCompleted o) -> OwcaValue* { return read_member(c_completed); },
			[&](OwcaRange o) -> OwcaValue* { return read_member(c_range); },
			[&](Number o) -> OwcaValue* { return read_member(c_float); },
			[&](bool o) -> OwcaValue* { return read_member(c_bool); },
			[&](OwcaString o) -> OwcaValue* { return read_member(c_string); },
			[&](OwcaFunctions o) -> OwcaValue* { return read_member(c_function); },
			[&](OwcaMap o) -> OwcaValue* { return read_member(c_map); },
			[&](OwcaClass o) -> OwcaValue* { return read_member(c_class); },
			[&](OwcaObject o) -> OwcaValue* { return read_from_object(o.internal_value()); },
			[&](OwcaArray o) -> OwcaValue* { return read_member(c_array); },
			[&](OwcaTuple o) -> OwcaValue* { return read_member(c_tuple); },
			[&](OwcaSet o) -> OwcaValue* { return read_member(c_set); },
			[&](OwcaException o) -> OwcaValue* { return read_from_object(o.internal_owner()); },
			[&](OwcaIterator o) -> OwcaValue* { return read_member(c_iterator); },
			[&](OwcaNamespace o) -> OwcaValue* { return read_from_nspace(o.internal_value()); }
		);

		if (v && v->kind() == OwcaValueKind::Functions && bind_if_needed) {
			auto f = v->as_functions(vm);
			if (!f.internal_self_object())
				return f.bind(val);
		}

		if (!v) return std::nullopt;

		return *v;
	}

	void VM::member(OwcaValue val, std::string_view key, OwcaValue value)
	{
		val.visit(
			[&](OwcaObject o) {
				auto it2 = o.internal_value()->type_->values.find(key);
				if (it2 != o.internal_value()->type_->values.end()) {
					auto succ = visit_variant(it2->second,
						[&](RuntimeFunctions *rf) -> bool { return false; },
						[&](Class* var) -> bool {
							auto native_storage = o.internal_value()->native_storage_raw(ClassToken{ var });
							return var->native->set_member(this, key, native_storage, value);
						});
					if (succ) return;
				}
				o.internal_value()->values[std::string{ key }] = value;
			},
			[&](OwcaNamespace o) {
				o.member(key, value);
			},
			[&](const auto &) {
				throw_value_cant_have_fields(val.type());
			}
		);
	}

	OwcaNamespace VM::execute_code_block(const OwcaCode &oc)
	{
		return executor->execute_code_block(oc);
	}

	std::optional<OwcaValue> VM::resume_generator(OwcaIterator oi)
	{
		return executor->continue_iterator(oi);
	}

	OwcaValue VM::execute_call(OwcaValue func, std::span<OwcaValue> arguments)
	{
		return executor->execute_call(func, arguments);
	}
	OwcaArray VM::create_array(std::deque<OwcaValue> arguments)
	{
		auto t = allocate<Array>(0, std::move(arguments));
		return OwcaArray{ t };
	}
	OwcaArray VM::create_array(OwcaArray left, OwcaArray right)
	{
		std::deque<OwcaValue> arguments;
		arguments.insert(arguments.end(), left.internal_value()->values.begin(), left.internal_value()->values.end());
		arguments.insert(arguments.end(), right.internal_value()->values.begin(), right.internal_value()->values.end());
		return create_array(std::move(arguments));
	}
	OwcaArray VM::create_array(OwcaArray left, Number right)
	{
		if (right < 0) throw_invalid_operand_for_mul_string("an array", std::to_string(right));
		std::deque<OwcaValue> arguments;
		for (Number i = 0; i < right; ++i) {
			arguments.insert(arguments.end(), left.internal_value()->values.begin(), left.internal_value()->values.end());
		}
		return create_array(std::move(arguments));
	}
	OwcaTuple VM::create_tuple(std::pair<OwcaValue, OwcaValue> arguments)
	{
		auto t = allocate<Tuple>(0, std::vector<OwcaValue>{ arguments.first, arguments.second });
		return OwcaTuple{ t };
	}
	OwcaTuple VM::create_tuple(std::vector<OwcaValue> arguments)
	{
		if (arguments.empty() && empty_tuple != nullptr) return OwcaTuple{ empty_tuple };
		auto t = allocate<Tuple>(0, std::move(arguments));
		return OwcaTuple{ t };
	}
	OwcaTuple VM::create_tuple(OwcaTuple left, OwcaTuple right)
	{
		std::vector<OwcaValue> arguments;
		arguments.reserve(left.internal_value()->values.size() + right.internal_value()->values.size());
		arguments.insert(arguments.end(), left.internal_value()->values.begin(), left.internal_value()->values.end());
		arguments.insert(arguments.end(), right.internal_value()->values.begin(), right.internal_value()->values.end());
		return create_tuple(std::move(arguments));
	}
	OwcaTuple VM::create_tuple(OwcaTuple left, Number right)
	{
		if (right < 0) throw_invalid_operand_for_mul_string("a tuple", std::to_string(right));
		std::vector<OwcaValue> arguments;
		arguments.reserve(left.internal_value()->values.size() * (size_t)(std::floor(right)));
		for (Number i = 0; i < right; ++i) {
			arguments.insert(arguments.end(), left.internal_value()->values.begin(), left.internal_value()->values.end());
		}
		return create_tuple(std::move(arguments));
	}
	OwcaRange VM::create_range()
	{
		auto ds = allocate<Range>(0);
		return OwcaRange{ ds };
	}
	OwcaMap VM::create_map(const std::span<OwcaValue> &arguments)
	{
		auto vm = OwcaVM{ this };
		auto ds = allocate<DictionaryShared>(0);
		for (auto i = 0u; i < arguments.size(); i += 2) {
			ds->dict.write(arguments[i], arguments[i + 1]);
		}
		return OwcaMap{ ds };
	}
	OwcaMap VM::create_map(const std::span<std::pair<OwcaValue, OwcaValue>> &arguments)
	{
		auto vm = OwcaVM{ this };
		auto ds = allocate<DictionaryShared>(0);
		for(auto q : arguments) {
			ds->dict.write(q.first, q.second);
		}
		return OwcaMap{ ds };
	}
	OwcaMap VM::create_map(const std::span<std::pair<std::string, OwcaValue>> &arguments)
	{
		auto vm = OwcaVM{ this };
		auto ds = allocate<DictionaryShared>(0);
		for(auto q : arguments) {
			ds->dict.write(create_string_from_view(q.first), q.second);
		}
		return OwcaMap{ ds };
	}
	OwcaSet VM::create_set(const std::span<OwcaValue> &arguments)
	{
		auto vm = OwcaVM{ this };
		auto ds = allocate<SetShared>(0);
		for (auto i = 0u; i < arguments.size(); ++i) {
			ds->dict.write(arguments[i], OwcaEmpty{});
		}
		return OwcaSet{ ds };
	}
	OwcaNamespace VM::create_namespace(OwcaCode code, std::unordered_map<std::string_view, size_t> identifier_to_global_index) {
		auto ns = allocate<Namespace>(0, std::move(code), std::move(identifier_to_global_index));
		return OwcaNamespace{ ns };	
	}
	OwcaString VM::create_string_from_view(std::string_view txt)
	{
		auto str = precreate_string(txt.size());
		std::memcpy(str->pointer(), txt.data(), txt.size());
		return OwcaString{ str };
	}
	String *VM::precreate_string(size_t size) {
		if (size == 0) return empty_string;
		auto vm = OwcaVM{ this };
		if (size >= (size_t(1) << 32)) {
			throw_string_too_large(size);
		}
		auto str = allocate<String>(size, (std::uint32_t)size);
		return str;
	}

	OwcaString VM::create_string(OwcaString s, size_t start, size_t end)
	{
		if (start >= end) return OwcaString{ empty_string };
		auto size = end - start;
		return create_string_from_view(s.text().substr(start, size));
	}
	OwcaString VM::create_string(OwcaString str, size_t count)
	{
		if (count == 0) return OwcaString{ empty_string };
		if (str.size() == 0) return OwcaString{ empty_string };
		if (str.size() * count >= (size_t(1) << 32)) {
			throw_string_too_large(str.size() * count);
		}

		auto new_s = precreate_string(str.size() * count);
		for(size_t i = 0; i < count; ++i) {
			std::memcpy(new_s->pointer() + i * str.size(), str.text().data(), str.size());
		}
		return OwcaString{ new_s };
	}
	OwcaString VM::create_string(OwcaString l, OwcaString r)
	{
		if (l.size() == 0) return r;
		if (r.size() == 0) return l;
		if (l.size() + r.size() >= (size_t(1) << 32)) {
			throw_string_too_large(l.size() + r.size());
		}
		auto new_s = precreate_string(l.size() + r.size());
		std::memcpy(new_s->pointer(), l.text().data(), l.size());
		std::memcpy(new_s->pointer() + l.size(), r.text().data(), r.size());
		return OwcaString{ new_s };
	}

	bool VM::compare_values_eq(OwcaValue left, OwcaValue right) {
		return executor->execute_compare_eq(left, right);
	}
	bool VM::compare_values_is(OwcaValue left, OwcaValue right) {
		return executor->execute_compare_is(left, right);
	}
	bool VM::compare_values_less(OwcaValue left, OwcaValue right) {
		return executor->execute_compare_less(left, right);
	}
	bool VM::compare_values(OwcaValue left, OwcaValue right, CompareKind kind) {
		return executor->execute_compare(left, right, kind);
	}
	// bool VM::compare_values(CompareKind kind, OwcaValue left, OwcaValue right)
	// {
	// 	auto value = Internal::execute_compare(this, kind, left, right);
	// 	if (value == CompareResult::NotExecuted) [[unlikely]] {
	// 		throw_cant_compare(kind, left.type(), right.type());
	// 	}
	// 	return value == CompareResult::True;
	// }
	
	bool VM::calculate_if_true(OwcaValue r) {
		return r.visit(
			[&](OwcaEmpty value) -> bool {
				return false;
			},
			[&](OwcaCompleted value) -> bool {
				return false;
			},
			[&](OwcaRange value) -> bool {
				return value.lower() != value.upper();
			},
			[&](bool value) -> bool {
				return value;
			},
			[&](Number value) -> bool {
				return value != 0.0;
			},
			[&](OwcaString value) -> bool {
				return value.internal_value()->size() != 0;
			},
			[&](OwcaFunctions value) -> bool {
				return true;
			},
			[&](OwcaMap value) -> bool {
				return value.size() > 0;
			},
			[&](OwcaClass value) -> bool {
				return true;
			},
			[&](OwcaObject value) -> bool {
				return true;
			},
			[&](OwcaArray value) -> bool {
				return value.size() > 0;
			},
			[&](OwcaTuple value) -> bool {
				return value.size() > 0;
			},
			[&](OwcaSet value) -> bool {
				return value.size() > 0;
			},
			[&](OwcaException value) -> bool {
				return true;
			},
			[&](OwcaIterator value) -> bool {
				return !value.completed();
			},
			[](OwcaNamespace value) -> bool {
				return true;
			}
		);
	}
	size_t VM::calculate_hash(OwcaValue value) {
		auto vm = OwcaVM{ this };
		static auto calc_hash = [](const auto& q) {
			return std::hash<std::remove_cvref_t<decltype(q)>>()(q);
			};
		return value.visit(
			[](OwcaEmpty o) -> size_t { return 3; },
			[](OwcaCompleted o) -> size_t { return 12; },
			[](OwcaRange o) -> size_t { return o.internal_object()->hash(); },
			[](Number o) -> size_t { return calc_hash(o) * 1013 + 5; },
			[](bool o) -> size_t { return (o ? 1 : 0) * 1021 + 7; },
			[](OwcaString o) -> size_t { return o.hash() * 1031 + 8; },
			[](OwcaFunctions o) -> size_t { return calc_hash(o.name()) * 1033 + 9; },
			[&](OwcaMap o) -> size_t {
				throw_not_hashable(value.type());
			},
			[&](OwcaClass o) -> size_t {
				return std::hash<void*>()(o.internal_value()) * 1009 + 10;
			},
			[&](OwcaObject o) -> size_t {
				throw_not_hashable(value.type());
			},
			[&](OwcaException o) -> size_t {
				throw_not_hashable(value.type());
			},
			[&](OwcaArray o) -> size_t {
				throw_not_hashable(value.type());
			},
			[&](OwcaTuple o) -> size_t {
				return o.internal_value()->hash() + 11;
			},
			[&](OwcaSet o) -> size_t {
				throw_not_hashable(value.type());
			},
			[&](OwcaIterator o) -> size_t {
				throw_not_hashable(value.type());
			},
			[&](OwcaNamespace o) -> size_t {
				return std::hash<void*>()(o.internal_value()) * 1009 + 1002;
			}
		);
	}

	OwcaIterator VM::create_iterator(OwcaValue r)
	{
		auto func = try_member(r, "__iter__");
		if (!func)
			throw_not_iterable(r.type());
		auto val = execute_call(*func, {});
		return val.as_iterator(this);
	}

	void VM::run_gc() {
		auto ggc = GenerationGC{ ++generation_gc };

		// mark
		gc_mark_value(this, ggc, empty_tuple);
		gc_mark_value(this, ggc, empty_string);
		gc_mark_value(this, ggc, *executor);

		gc_mark_value(this, ggc, temp_gc_protect_list);

		// sweep
		AllocationBase *valid = &root_allocated_memory;
		while (valid->next != &root_allocated_memory) {
			auto obj = valid->next;
			if (obj->last_gc_mark != ggc) {
				valid->next = obj->next;
				obj->next->prev = valid;
				obj->~AllocationBase();
				std::free(obj);
			}
			else {
				valid = valid->next;
			}
		}
	}

	void gc_mark_value(const OwcaVM &vm, GenerationGC ggc, const AllocationBase* ptr) {
		if (ptr->last_gc_mark != ggc) {
			ptr->last_gc_mark = ggc;
			ptr->gc_mark(vm, ggc);
		}
	}
}