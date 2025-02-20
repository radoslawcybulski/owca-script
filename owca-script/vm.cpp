#include "stdafx.h"
#include "vm.h"
#include "owca_value.h"
#include "runtime_function.h"
#include "owca_code.h"
#include "code_buffer.h"
#include "owca_vm.h"
#include "owca_value.h"
#include "flow_control.h"
#include "dictionary.h"
#include "array.h"
#include "object.h"
#include "owca_iterator.h"
#include "string.h"

namespace OwcaScript::Internal {
	VM::VM() {
		root_allocated_memory.prev = root_allocated_memory.next = &root_allocated_memory;
		initialize_builtins();
		auto vm = OwcaVM{ this };
		empty_tuple = create_tuple({}).as_tuple(vm).object;
	}
	VM::~VM() {
		stacktrace.clear();
		allocated_objects.clear();
		builtin_objects.clear();
		run_gc();
	}

	namespace {
		template <typename T> struct FuncToTuple {

		};
		template <typename ... ARGS> struct FuncToTuple<OwcaValue(OwcaVM &, ARGS...)> {
			using type = std::tuple<std::remove_cvref_t<ARGS>...>;
		};
	}
	struct VM::BuiltinProvider : public OwcaVM::NativeCodeProvider {
		static auto convert_impl2(OwcaVM &vm, size_t I, bool *b, const OwcaValue &v) {
			if (v.kind() != OwcaValueKind::Bool) 
				VM::get(vm).throw_cant_call(std::format("{} argument ({}) can't be converted to bool", I + 1, v.type()));
			return v.as_bool(vm).internal_value();
		}
		template <std::integral T>
		static T convert_impl2(OwcaVM &vm, size_t I, T *, const OwcaValue &v) {
			if (v.kind() != OwcaValueKind::Int && v.kind() != OwcaValueKind::Float) 
				VM::get(vm).throw_cant_call(std::format("{} argument ({}) can't be converted to integer value", I + 1, v.type()));
			if (v.kind() == OwcaValueKind::Int) {
				auto b = (T)v.as_int(vm).internal_value();
				if (b != v.as_int(vm).internal_value())
					VM::get(vm).throw_cant_call(std::format("{} argument ({}) overflows integer value", I + 1, v.to_string()));
				return b;
			}
			else {
				auto b = (T)v.as_float(vm).internal_value();
				if (b != v.as_float(vm).internal_value())
					VM::get(vm).throw_cant_call(std::format("{} argument ({}) overflows integer value or doesn't round up to integer", I + 1, v.to_string()));
				return b;
			}
		}
		template <std::floating_point T>
		static auto convert_impl2(OwcaVM &vm, size_t I, T *, const OwcaValue &v) {
			if (v.kind() != OwcaValueKind::Int && v.kind() != OwcaValueKind::Float) 
				VM::get(vm).throw_cant_call(std::format("{} argument ({}) can't be converted to integer value", I + 1, v.type()));
			if (v.kind() == OwcaValueKind::Int) {
				return (T)v.as_int(vm).internal_value();
			}
			else {
				return (T)v.as_float(vm).internal_value();
			}
		}
		static std::string convert_impl2(OwcaVM &vm, size_t I, std::string *b, const OwcaValue &v) {
			if (v.kind() != OwcaValueKind::String) 
				VM::get(vm).throw_cant_call(std::format("{} argument ({}) is not a string", I + 1, v.type()));
			return std::string{ v.as_string(vm).text() };
		}
		static std::string_view convert_impl2(OwcaVM &vm, size_t I, std::string_view *b, const OwcaValue &v) {
			if (v.kind() != OwcaValueKind::String) 
				VM::get(vm).throw_cant_call(std::format("{} argument ({}) is not a string", I + 1, v.type()));
			return v.as_string(vm).text();
		}
		static OwcaEmpty convert_impl2(OwcaVM &vm, size_t I, OwcaEmpty *b, const OwcaValue &v) {
			if (v.kind() != OwcaValueKind::Empty) 
				VM::get(vm).throw_cant_call(std::format("{} argument ({}) is not a nul value", I + 1, v.type()));
			return {};
		}
		static auto convert_impl2(OwcaVM &vm, size_t I, OwcaRange *b, const OwcaValue &v) {
			if (v.kind() != OwcaValueKind::Range) 
				VM::get(vm).throw_cant_call(std::format("{} argument ({}) is not a range", I + 1, v.type()));
			return v.as_range(vm);
		}
		static auto convert_impl2(OwcaVM &vm, size_t I, OwcaInt *b, const OwcaValue &v) {
			if (v.kind() != OwcaValueKind::Int) 
				VM::get(vm).throw_cant_call(std::format("{} argument ({}) is not an integer", I + 1, v.type()));
			return v.as_int(vm);
		}
		static auto convert_impl2(OwcaVM &vm, size_t I, OwcaFloat *b, const OwcaValue &v) {
			if (v.kind() != OwcaValueKind::Float) 
				VM::get(vm).throw_cant_call(std::format("{} argument ({}) is not a floating point value", I + 1, v.type()));
			return v.as_float(vm);
		}
		static const auto &convert_impl2(OwcaVM &vm, size_t I, OwcaString *b, const OwcaValue &v) {
			if (v.kind() != OwcaValueKind::String) 
				VM::get(vm).throw_cant_call(std::format("{} argument ({}) is not a string", I + 1, v.type()));
			return v.as_string(vm);
		}
		static auto convert_impl2(OwcaVM &vm, size_t I, OwcaFunctions *b, const OwcaValue &v) {
			if (v.kind() != OwcaValueKind::Functions) 
				VM::get(vm).throw_cant_call(std::format("{} argument ({}) is not a function set", I + 1, v.type()));
			return v.as_functions(vm);
		}
		static auto convert_impl2(OwcaVM &vm, size_t I, OwcaMap *b, const OwcaValue &v) {
			if (v.kind() != OwcaValueKind::Map) 
				VM::get(vm).throw_cant_call(std::format("{} argument ({}) is not a dictionary", I + 1, v.type()));
			return v.as_map(vm);
		}
		static auto convert_impl2(OwcaVM &vm, size_t I, OwcaClass *b, const OwcaValue &v) {
			if (v.kind() != OwcaValueKind::Class) 
				VM::get(vm).throw_cant_call(std::format("{} argument ({}) is not a type", I + 1, v.type()));
			return v.as_class(vm);
		}
		static auto convert_impl2(OwcaVM &vm, size_t I, OwcaObject *b, const OwcaValue &v) {
			if (v.kind() != OwcaValueKind::Object) 
				VM::get(vm).throw_cant_call(std::format("{} argument ({}) is not an object", I + 1, v.type()));
			return v.as_object(vm);
		}
		static auto convert_impl2(OwcaVM &vm, size_t I, OwcaArray *b, const OwcaValue &v) {
			if (v.kind() != OwcaValueKind::Array) 
				VM::get(vm).throw_cant_call(std::format("{} argument ({}) is not an array", I + 1, v.type()));
			return v.as_array(vm);
		}
		static auto convert_impl2(OwcaVM &vm, size_t I, OwcaTuple *b, const OwcaValue &v) {
			if (v.kind() != OwcaValueKind::Tuple) 
				VM::get(vm).throw_cant_call(std::format("{} argument ({}) is not a tuple", I + 1, v.type()));
			return v.as_tuple(vm);
		}
		static auto convert_impl2(OwcaVM &vm, size_t I, OwcaSet *b, const OwcaValue &v) {
			if (v.kind() != OwcaValueKind::Set) 
				VM::get(vm).throw_cant_call(std::format("{} argument ({}) is not a set", I + 1, v.type()));
			return v.as_set(vm);
		}
		static OwcaValue convert_impl2(OwcaVM &vm, size_t I, OwcaValue *b, const OwcaValue &v) {
			return v;
		}

		template <size_t I, typename ... ARGS> static auto convert_impl(OwcaVM &vm, std::span<OwcaValue> args) {
			if constexpr(I < sizeof...(ARGS)) {
				using T = std::remove_cvref_t<std::tuple_element_t<I, std::tuple<ARGS...>>>;
				std::tuple<T> tmp = { convert_impl2(vm, I, (T*)nullptr, args[I]) };
				auto res = convert_impl<I + 1, ARGS...>(vm, args);
				auto res2 = std::tuple_cat(std::move(tmp), std::move(res));
				return res2;
			}
			else {
				return std::tuple<>{};
			}
		}
		template <typename ... ARGS> static std::tuple<OwcaVM&, ARGS...> convert(OwcaVM &vm, std::span<OwcaValue> args) {
			assert(sizeof...(ARGS) == args.size());
			std::tuple<ARGS...> dst_args = convert_impl<0, ARGS...>(vm, args);
			return std::tuple_cat(std::tuple<OwcaVM&>(vm), std::move(dst_args));
		}
		template <typename ... ARGS> static auto convert2(OwcaVM &vm, std::span<OwcaValue> args, std::tuple<ARGS...> *) {
			return convert<ARGS...>(vm, args);
		}
		
		static OwcaValue range_init(OwcaVM &vm, OwcaRange, std::int64_t lower, std::int64_t upper) {
			return OwcaRange{ OwcaInt{ lower }, OwcaInt{ upper } };
		}
		static OwcaValue range_lower(OwcaVM &vm, OwcaRange r) {
			return r.lower();
		}
		static OwcaValue range_upper(OwcaVM &vm, OwcaRange r) {
			return r.upper();
		}
		static OwcaValue range_size(OwcaVM &vm, OwcaRange r) {
			auto v = std::abs(r.upper().internal_value() - r.lower().internal_value());
			if (v < 0)
				VM::get(vm).throw_overflow(std::format("range size for size {} -> {} overflows integer", r.lower(), r.upper()));
			auto v2 = OwcaInt{ v };
			if (v2.internal_value() != v) {
				VM::get(vm).throw_overflow(std::format("range size for size {} -> {} overflows integer", r.lower(), r.upper()));
			}
			return v2;
		}
		static OwcaValue bool_init(OwcaVM &vm, const OwcaValue &r) {
			return OwcaBool{ VM::get(vm).calculate_if_true(r) };
		}
		static OwcaValue int_init(OwcaVM &vm, const OwcaValue &r) {
			return OwcaInt { r.visit(
				[&](OwcaBool value) -> OwcaIntInternal {
					return value.internal_value() ? 1 : 0;
				},
				[&](OwcaInt value) -> OwcaIntInternal {
					return value.internal_value();
				},
				[&](OwcaFloat value) -> OwcaIntInternal {
					return (OwcaIntInternal)value.internal_value();
				},
				[&](const auto &) -> OwcaIntInternal {
					VM::get(vm).throw_cant_convert_to_integer(r.type());
				}
			) };
		}
		static OwcaValue float_init(OwcaVM &vm, const OwcaValue &r) {
			return OwcaFloat { r.visit(
				[&](OwcaBool value) -> OwcaFloatInternal {
					return value.internal_value() ? 1.0f : 0.0f;
				},
				[&](OwcaInt value) -> OwcaFloatInternal {
					return (OwcaFloatInternal)value.internal_value();
				},
				[&](OwcaFloat value) -> OwcaFloatInternal {
					return value.internal_value();
				},
				[&](const auto &) -> OwcaFloatInternal {
					VM::get(vm).throw_cant_convert_to_float(r.type());
				}
			) };
		}
		static OwcaValue string_init(OwcaVM &vm, const OwcaValue &, const OwcaValue &r) {
			return vm.create_string(r.to_string());
		}
		static OwcaValue string_size(OwcaVM &vm, const OwcaValue &r) {
			auto v = r.as_string(vm).internal_value()->size();
			auto v2 = (OwcaIntInternal)v;
			if (v2 != v)
				VM::get(vm).throw_overflow(std::format("string size {} doesn't fit integer", v));
			return OwcaInt{ v2 };
		}
		static OwcaValue function_bind(OwcaVM &vm, const OwcaValue &r, const OwcaValue &bind) {
			auto f = r.as_functions(vm);
			return VM::get(vm).bind_function(f, bind);
		}
		static OwcaValue function_bound_value(OwcaVM &vm, const OwcaValue &r) {
			auto f = r.as_functions(vm);
			auto bound = f.self_object->is_bound_function_self_object();
			if (bound) {
				return bound->self;
			}
			return OwcaObject{ static_cast<Object*>(f.self_object) };
		}
		static OwcaValue map_size(OwcaVM &vm, const OwcaMap &r) {
			auto v = r.size();
			auto v2 = (OwcaIntInternal)v;
			if (v2 != v)
				VM::get(vm).throw_overflow(std::format("map size {} doesn't fit integer", v));
			return OwcaInt{ v2 };
		}
		static OwcaValue set_size(OwcaVM &vm, const OwcaSet &r) {
			auto v = r.size();
			auto v2 = (OwcaIntInternal)v;
			if (v2 != v)
				VM::get(vm).throw_overflow(std::format("map size {} doesn't fit integer", v));
			return OwcaInt{ v2 };
		}
		static OwcaValue class_name(OwcaVM &vm, const OwcaValue &r) {
			return vm.create_string_from_view(r.as_class(vm).object->name);
		}
		static OwcaValue class_full_name(OwcaVM &vm, const OwcaValue &r) {
			return vm.create_string_from_view(r.as_class(vm).object->full_name);
		}
		static OwcaValue array_init(OwcaVM &vm, const OwcaArray &self, const OwcaValue &r) {
			r.visit(
				[&](OwcaArray o) {
					self.object->values = o.object->values;
				},
				[&](OwcaTuple o) {
					self.object->values = o.object->values;
				},
				[&](OwcaMap o) {
					for(const auto &val : o) {
						self.object->values.push_back(
							VM::get(vm).create_tuple({ val.first, val.second })
						);
					}
				},
				[&](OwcaSet o) {
					for(const auto &val : o) {
						self.object->values.push_back(val);
					} 
				},
				[&](OwcaString o) {
					self.object->values.reserve(o.internal_value()->size());
					o.internal_value()->iterate_over_content(
						[&](std::string_view txt) {
							for(auto q : txt) {
								self.object->values.push_back(vm.create_string_from_view(std::string_view{ &q, 1 }));
							}
						}
					);
				},
				[&](const auto &) {
					VM::get(vm).throw_wrong_type(std::format("can't create an array from {}", r.type()));
				}
			);
			return {};
		}
		static OwcaValue array_size(OwcaVM &vm, const OwcaArray &self) {
			auto v = self.object->values.size();
			auto v2 = (OwcaIntInternal)v;
			if (v2 != v)
				VM::get(vm).throw_overflow(std::format("array size {} doesn't fit integer", v));
			return OwcaInt{ v2 };
		}
		static OwcaValue tuple_init(OwcaVM &vm, const OwcaArray &self, const OwcaValue &r) {
			r.visit(
				[&](OwcaArray o) {
					self.object->values = o.object->values;
				},
				[&](OwcaTuple o) {
					self.object->values = o.object->values;
				},
				[&](OwcaMap o) {
					for(const auto &val : o) {
						self.object->values.push_back(
							VM::get(vm).create_tuple({ val.first, val.second })
						);
					} 
				},
				[&](OwcaSet o) {
					for(const auto &val : o) {
						self.object->values.push_back(val);
					}
				},
				[&](OwcaString o) {
					self.object->values.reserve(o.internal_value()->size());
					o.internal_value()->iterate_over_content(
						[&](std::string_view txt) {
							for(auto q : txt) {
								self.object->values.push_back(vm.create_string_from_view(std::string_view{ &q, 1 }));
							}
						}
					);
				},
				[&](const auto &) {
					VM::get(vm).throw_wrong_type(std::format("can't create an array from {}", r.type()));
				}
			);
			return {};
		}
		static OwcaValue tuple_size(OwcaVM &vm, const OwcaArray &self) {
			auto v = self.object->values.size();
			auto v2 = (OwcaIntInternal)v;
			if (v2 != v)
				VM::get(vm).throw_overflow(std::format("tuple size {} doesn't fit integer", v));
			return OwcaInt{ v2 };
		}
		static OwcaValue hash(OwcaVM &vm, const OwcaValue &r) {
			auto v = VM::get(vm).calculate_hash(r);
			return OwcaInt{ (OwcaIntInternal)v };
		}

		template <typename F>
		static std::function<OwcaValue(OwcaVM&, std::span<OwcaValue> args)> adapt(F &&f) {
			return [f = std::move(f)](OwcaVM &vm, std::span<OwcaValue> args) -> OwcaValue {
				using T = typename FuncToTuple<std::remove_cvref_t<F>>::type;
				auto dest_args = convert2(vm, args, (T*)nullptr);
				return std::apply(f, dest_args);
			};
		}

		std::optional<Function> native_function(std::string_view full_name, OwcaVM::FunctionToken token, std::span<const std::string_view> param_names) const override {
			if (full_name == "Range.__init__") return adapt(range_init);
			if (full_name == "Range.lower") return adapt(range_lower);
			if (full_name == "Range.upper") return adapt(range_upper);
			if (full_name == "Range.size") return adapt(range_size);
			if (full_name == "Bool.__init__") return adapt(bool_init);
			if (full_name == "Int.__init__") return adapt(int_init);
			if (full_name == "Float.__init__") return adapt(float_init);
			if (full_name == "String.__init__") return adapt(string_init);
			if (full_name == "String.size") return adapt(string_size);
			if (full_name == "Function.bound_value") return adapt(function_bound_value);
			if (full_name == "Function.bind") return adapt(function_bind);
			if (full_name == "Map.size") return adapt(map_size);
			if (full_name == "Set.size") return adapt(set_size);
			if (full_name == "Class.name") return adapt(class_name);
			if (full_name == "Class.full_name") return adapt(class_full_name);
			if (full_name == "Array.__init__") return adapt(array_init);
			if (full_name == "Array.size") return adapt(array_size);
			if (full_name == "Tuple.__init__") return adapt(tuple_init);
			if (full_name == "Tuple.size") return adapt(tuple_size);
			if (full_name == "hash") return adapt(hash);
			return std::nullopt;
		}
		std::unique_ptr<OwcaClass::NativeClassInterface> native_class(std::string_view full_name, OwcaVM::ClassToken token) const override {
			return nullptr;
		}
	};
	void VM::initialize_builtins()
	{
		auto code = std::string{ R"(
class Nul {
}
class Range {
	function native __init__(self, lower, upper);
	function native lower(self);
	function native upper(self);
	function native size(self);
}
class Bool {
	function native __init__(self, value);
}
class Int {
	function native __init__(self, value);
}
class Float {
	function native __init__(self, value);
}
class String {
	function native __init__(self, value);
	function native size(self);
}
class Function {
	function native bound_value(self);
	function native bind(self, value);
}
class Map {
	function native size(self);
}
class Set {
	function native size(self);
}
class Class {
	function native name(self);
	function native full_name(self);
}
class Array {
	function native __init__(self, value);
	function native size(self);
}
class Tuple {
	function native __init__(self, value);
	function native size(self);
}
function native hash(value);
)" };
		auto vm = OwcaVM{ this };
		auto code_compiled = vm.compile("<builtin>", std::move(code), std::make_unique<BuiltinProvider>());
		OwcaValue builtin_dictionary;
		vm.execute(std::move(code_compiled), builtin_dictionary);

		auto read = [&](const OwcaValue &val) -> Class*{
			return ensure_is_class(val);
		};
		auto dct = builtin_dictionary.as_map(vm);
		for(auto value_pair : dct) {
			if (value_pair.first.kind() == OwcaValueKind::String) {
				auto key = value_pair.first.as_string(vm).text();
				if (key == "Nul") {
					c_nul = read(value_pair.second);
					c_nul->allocator_override = []() -> OwcaValue { return OwcaEmpty{}; };
				}
				else if (key == "Range") {
					c_range = read(value_pair.second);
					c_range->allocator_override = []() -> OwcaValue { return OwcaRange{ 0, 0 }; };
				}
				else if (key == "Bool") {
					c_bool = read(value_pair.second);
					c_bool->allocator_override = []() -> OwcaValue { return OwcaBool{ false }; };
				}
				else if (key == "Int") {
					c_int = read(value_pair.second);
					c_int->allocator_override = []() -> OwcaValue { return OwcaInt{ 0 }; };
				}
				else if (key == "Float") {
					c_float = read(value_pair.second);
					c_float->allocator_override = []() -> OwcaValue { return OwcaFloat{ 0 }; };
				}
				else if (key == "String") {
					c_string = read(value_pair.second);
					c_string->allocator_override = [&]() -> OwcaValue { return create_string(""); };
				}
				else if (key == "Function") {
					c_function = read(value_pair.second);
					c_function->allocator_override = [&]() -> OwcaValue {
						throw_not_callable(c_function->full_name);
					};
				}
				else if (key == "Map") {
					c_map = read(value_pair.second);
					c_map->allocator_override = [&]() -> OwcaValue {
						return create_map({});
					};
				}
				else if (key == "Class") {
					c_class = read(value_pair.second);
					c_class->allocator_override = [&]() -> OwcaValue {
						throw_not_callable(c_function->full_name);
					};
				}
				else if (key == "Array") {
					c_array = read(value_pair.second);
					c_array->allocator_override = [&]() -> OwcaValue {
						return create_array({});
					};
				}
				else if (key == "Tuple") {
					c_tuple = read(value_pair.second);
					c_tuple->allocator_override = [&]() -> OwcaValue {
						return create_tuple({});
					};
				}
				else if (key == "Set") {
					c_set = read(value_pair.second);
					c_set->allocator_override = [&]() -> OwcaValue {
						return create_set({});
					};
				}
				builtin_objects[std::string{ key }] = std::move(value_pair.second);
			}
		}
	}
	VM& VM::get(const OwcaVM& v)
	{
		return *v.vm;
	}

	void VM::throw_division_by_zero() const
	{
		assert(false);
		throw 1;
	}

	void VM::throw_mod_division_by_zero() const
	{
		assert(false);
		throw 1;
	}

	void VM::throw_cant_convert_to_float(std::string_view type) const
	{
		assert(false);
		throw 1;
		//throw OwcaException{ std::format("can't convert value of type `{}` to floating point", type()) };
	}

	void VM::throw_cant_convert_to_integer(OwcaFloatInternal val) const
	{
		assert(false);
		throw 1;
		//throw OwcaException{ std::format("can't convert value `{}` to integer - the conversion would loose data", f) };
	}

	void VM::throw_cant_convert_to_integer(std::string_view type) const
	{
		assert(false);
		throw 1;
		// throw OwcaException{ std::format("can't convert value of type `{}` to integer", type()) };
	}

	void VM::throw_not_a_number(std::string_view type) const
	{
		assert(false);
		throw 1;
	}

	void VM::throw_overflow(std::string_view msg) const
	{
		assert(false);
		throw 1;
	}

	void VM::throw_cant_compare(AstExprCompare::Kind kind, std::string_view left, std::string_view right) const
	{
		assert(false);
		throw 1;
	}

	void VM::throw_index_out_of_range(std::string msg) const
	{
		assert(false);
		throw 1;
	}

	void VM::throw_value_not_indexable(std::string_view type, std::string_view key_type) const
	{
		assert(false);
		throw 1;
	}

	void VM::throw_missing_member(std::string_view type, std::string_view ident) const
	{
		assert(false);
		throw 1;
	}

	void VM::throw_cant_call(std::string_view msg) const
	{
		assert(false);
		throw 1;
	}

	void VM::throw_not_callable(std::string_view type) const
	{
		assert(false);
		throw 1;
	}
	
	void VM::throw_not_callable_wrong_number_of_params(std::string_view type, unsigned int) const
	{
		assert(false);
		throw 1;
	}

	void VM::throw_wrong_type(std::string_view type, std::string_view expected) const
	{
		assert(false);
		throw 1;
	}

	void VM::throw_wrong_type(std::string_view msg) const
	{
		assert(false);
		throw 1;
	}

	void VM::throw_unsupported_operation_2(std::string_view oper, std::string_view left, std::string_view right) const
	{
		assert(false);
		throw 1;
	}

	void VM::throw_invalid_operand_for_mul_string(std::string_view val) const
	{
		assert(false);
		throw 1;
	}

	void VM::throw_missing_key(std::string_view key) const
	{
		assert(false);
		throw 1;
	}

	void VM::throw_not_hashable(std::string_view type) const
	{
		assert(false);
		throw 1;
	}

	void VM::throw_value_cant_have_fields(std::string_view type) const
	{
		assert(false);
		throw 1;
	}

	void VM::throw_missing_native(std::string_view msg) const
	{
		assert(false);
		throw 1;
	}

	void VM::throw_not_iterable(std::string_view msg) const
	{
		assert(false);
		throw 1;
	}

	void VM::throw_readonly(std::string_view msg) const
	{
		assert(false);
		throw 1;
	}

	OwcaValue VM::member(const OwcaValue& val, const std::string& key)
	{
		auto v = try_member(val, key);
		if (!v) {
			throw_missing_member(val.type(), key);
		}
		return *v;
	}

	std::optional<OwcaValue> VM::try_member(const OwcaValue& val, const std::string& key)
	{
		auto vm = OwcaVM{ this };
		auto read_member = [&](Class *cls) -> OwcaValue * {
			auto it = cls->values.find(key);
			if (it == cls->values.end()) return nullptr;
			return &it->second;
		};
		bool bind_if_needed = false;
		auto v = val.visit(
			[&](const OwcaEmpty& o) -> OwcaValue* { return read_member(c_nul); },
			[&](const OwcaRange& o) -> OwcaValue* { return read_member(c_range); },
			[&](const OwcaInt& o) -> OwcaValue* { return read_member(c_int); },
			[&](const OwcaFloat& o) -> OwcaValue* { return read_member(c_float); },
			[&](const OwcaBool& o) -> OwcaValue* { return read_member(c_bool); },
			[&](const OwcaString& o) -> OwcaValue* { return read_member(c_string); },
			[&](const OwcaFunctions& o) -> OwcaValue* { return read_member(c_function); },
			[&](const OwcaMap& o) -> OwcaValue* { return read_member(c_map); },
			[&](const OwcaClass& o) -> OwcaValue* { return read_member(c_class); },
			[&](const OwcaObject& o) -> OwcaValue* {
				auto it = o.object->values.find(key);
				if (it != o.object->values.end()) {
					return &it->second;
				}

				it = o.object->type_->values.find(key);
				if (it != o.object->type_->values.end()) {
					bind_if_needed = true;
					return &it->second;
				}

				return nullptr;
			},
			[&](const OwcaArray& o) -> OwcaValue* { return read_member(c_array); },
			[&](const OwcaTuple& o) -> OwcaValue* { return read_member(c_tuple); },
			[&](const OwcaSet& o) -> OwcaValue* { return read_member(c_set); }
		);

		if (v && v->kind() == OwcaValueKind::Functions) {
			auto f = v->as_functions(vm);
			if (f.self_object == nullptr)
				return bind_function(f, val);
		}

		if (!v) return std::nullopt;

		return *v;
	}

	OwcaFunctions VM::bind_function(OwcaFunctions src, const OwcaValue &val)
	{
		auto vm = OwcaVM{ this };
		if (val.kind() == OwcaValueKind::Object) {
			return OwcaFunctions{ src.functions, val.as_object(vm).object };
		}
		else {
			auto bound = allocate<BoundFunctionSelfObject>(0, val);
			return OwcaFunctions{ src.functions, bound };
		}
	}

	void VM::member(const OwcaValue& val, const std::string& key, OwcaValue value)
	{
		val.visit(
			[&](const OwcaObject &o) {
				o.object->values[key] = std::move(value);
			},
			[&](const auto &) {
				throw_value_cant_have_fields(val.type());
			}
		);
	}

	void VM::update_execution_line(Line line)
	{
		stacktrace.back().line = line;
	}

	VM::PopStack VM::prepare_exec(RuntimeFunctions* runtime_functions, unsigned int index, bool has_self_value)
	{
		auto it = runtime_functions->functions.find(index + (has_self_value ? 1 : 0));
		if (it == runtime_functions->functions.end()) {
			it = runtime_functions->functions.find(index);
			if (it == runtime_functions->functions.end() || it->second->is_method) {
				auto tmp = std::string{ "function " };
				tmp += runtime_functions->name;
				throw_not_callable_wrong_number_of_params(std::move(tmp), index);
			}
		}

		stacktrace.push_back({ it->second->fileline });
		auto& s = stacktrace.back();
		s.runtime_functions = runtime_functions;
		s.runtime_function = it->second;
		s.runtime_function->visit(
			[&](const RuntimeFunction::NativeFunction& nf) {
				s.values.resize(nf.parameter_names.size());
			},
			[&](const RuntimeFunction::ScriptFunction& sf) {
				s.values.resize(sf.identifier_names.size());
				assert(sf.copy_from_parents.size() == sf.values_from_parents.size());

				for (auto i = 0u; i < sf.copy_from_parents.size(); ++i) {
					s.values[sf.copy_from_parents[i].index_in_child] = sf.values_from_parents[i];
				}
			});

		return PopStack{ this };
	}

	OwcaValue VM::execute()
	{
		auto pp = AllocatedObjectsPointer{ *this };
		auto& s = stacktrace.back();
		auto vm = OwcaVM{ this };

		return s.runtime_function->visit(
			[&](const RuntimeFunction::NativeFunction& nf) -> OwcaValue {
				auto vm = OwcaVM{ this };

				return nf.function(vm, std::span{ s.values.begin(), s.values.end() });
			},
			[&](const RuntimeFunction::ScriptFunction& sf) -> OwcaValue {
				try {
					sf.body->execute(vm);
					return {};
				}
				catch (FlowControlReturn o) {
					return std::move(o.value);
				}
			});
	}

	OwcaValue VM::execute_code_block(const OwcaCode &oc, const std::unordered_map<std::string, OwcaValue> *values, OwcaValue *dict_output)
	{
		auto pp = AllocatedObjectsPointer{ *this };
		auto vm = OwcaVM{ this };
		OwcaValue val;
		{
			stacktrace.push_back({ oc.code_->root()->line });
			auto pop_stack = PopStack{ this };
			RuntimeFunction::ScriptFunction sf;
			RuntimeFunction rt_temp{ oc.code_, "", Line{0}, 0, false};
			rt_temp.data = std::move(sf);
			stacktrace.back().runtime_function = &rt_temp;
			val = oc.code_->root()->execute(vm);
		}
		assert(val.kind() == OwcaValueKind::Functions);
		auto functions = val.as_functions(vm);
		auto pop_stack = prepare_exec(functions.functions, 0, false);
		auto& s = stacktrace.back();
		s.runtime_function->visit(
			[&](RuntimeFunction::ScriptFunction& sf) -> void {
				std::unordered_map<std::string_view, unsigned int> value_index_map;
				for (auto i = 0u; i < sf.identifier_names.size(); ++i) {
					value_index_map[sf.identifier_names[i]] = i;
				}
				
				for(auto &it : builtin_objects) {
					auto it2 = value_index_map.find(it.first);
					assert(it2 != value_index_map.end());
					set_identifier(it2->second, it.second);
				}
				if (values) {
					for (auto& it : *values) {
						auto it2 = value_index_map.find(it.first);
						if (it2 != value_index_map.end()) {
							set_identifier(it2->second, it.second);
						}
					}
				}
			},
			[&](RuntimeFunction::NativeFunction&) -> void {
				assert(false);
			}
		);

		struct CopyBack {
			VM *self;
			OwcaValue *dict_output;

			CopyBack(VM *self, OwcaValue *dict_output) : self(self), dict_output(dict_output) {}
			~CopyBack() {
				if (dict_output) {
					auto &dout = *dict_output;
					if (dout.kind() != OwcaValueKind::Map) {
						dout = self->create_map({});
					}
					auto owca_vm = OwcaVM{ self };

					auto doout_obj = dout.as_map(owca_vm);
					auto& s = self->stacktrace.back();
					s.runtime_function->visit(
						[&](RuntimeFunction::ScriptFunction& sf) -> void {
							for (auto i = 0u; i < sf.identifier_names.size(); ++i) {
								auto key = self->create_string_from_view(sf.identifier_names[i]);
								doout_obj[key] = self->get_identifier(i);
							}
						},
						[&](RuntimeFunction::NativeFunction&) -> void {
							assert(false);
						}
					);
				}
			}
		};
		auto copy_back = CopyBack{ this, dict_output };

		try {
			return execute();
		}
		catch (FlowControlException) {
			assert(false);
			return {};
		}
	}
	OwcaValue VM::execute_call(const OwcaValue &func, std::span<OwcaValue> arguments)
	{
		auto pp = AllocatedObjectsPointer{ *this };
		return func.visit(
			[&](const OwcaFunctions& of) -> OwcaValue {
				auto vm = OwcaVM{ this };
				auto runtime_functions = func.as_functions(vm).functions;
				auto pop_stack = prepare_exec(runtime_functions, (unsigned int)arguments.size(), of.self_object != nullptr);
				auto& s = stacktrace.back();
				bool self = s.runtime_function->is_method;
				if (self) {
					if (of.self_object) {
						auto self_object_helper = of.self_object->is_bound_function_self_object();
						if (self_object_helper) {
							s.values[0] = self_object_helper->self;
						}
						else {
							s.values[0] = OwcaObject{ static_cast<Object*>(of.self_object) };
						}
					}
					else {
						throw_cant_call(std::format("can't call {} - missing self value", func.to_string()));
					}
				}
				for (auto i = (self ? 1u : 0u); i < s.runtime_function->param_count; ++i) {
					s.values[i] = arguments[i - (self ? 1u : 0u)];
				}

				return execute();
			},
			[&](const OwcaClass& oc) -> OwcaValue {
				auto vm = OwcaVM{ this };
				auto cls = func.as_class(vm).object;

				OwcaValue obj;
				
				if (cls->allocator_override) {
					obj = cls->allocator_override();
				}
				else {
					obj = OwcaObject{ allocate<Object>(cls->native_storage_total, cls) };
				}

				auto it = cls->values.find("__init__");
				if (it == cls->values.end()) {
					if (!arguments.empty()) {
						throw_cant_call(std::format("type {} has no __init__ function defined - expected constructor's call with no parameters, instead got {}", cls->full_name, arguments.size()));
					}
				}
				else {
					auto of = it->second.as_functions(vm);
					auto it2 = of.functions->functions.find((unsigned int)(1 + arguments.size()));
					if (it2 == of.functions->functions.end()) {
						throw_cant_call(std::format("type {} has __init__ function, but not one with {} parameters", cls->full_name, 1 + arguments.size()));
					}
					else {
						auto of2 = bind_function(of.functions, obj);
						execute_call(of2, arguments);
					}
				}

				return obj;
			},
			[&](const auto&) -> OwcaValue {
				throw_not_callable(func.type());
			}
		);
	}
	OwcaValue VM::create_array(std::vector<OwcaValue> arguments)
	{
		auto t = allocate<Array>(0, std::move(arguments));
		return OwcaArray{ t };
	}
	OwcaValue VM::create_tuple(std::vector<OwcaValue> arguments)
	{
		if (arguments.empty() && empty_tuple != nullptr) return OwcaTuple{ empty_tuple };
		auto t = allocate<Array>(0, std::move(arguments), true);
		return OwcaTuple{ t };
	}
	OwcaValue VM::create_map(const std::vector<OwcaValue> &arguments)
	{
		auto vm = OwcaVM{ this };
		auto ds = allocate<DictionaryShared>(0, vm, true);
		for (auto i = 0u; i < arguments.size(); i += 2) {
			ds->dict.write(arguments[i], std::move(arguments[i + 1]));
		}
		return OwcaValue{ OwcaMap{ ds } };
	}
	OwcaValue VM::create_set(const std::vector<OwcaValue> &arguments)
	{
		auto vm = OwcaVM{ this };
		auto ds = allocate<DictionaryShared>(0, vm, false);
		for (auto i = 0u; i < arguments.size(); ++i) {
			ds->dict.write(arguments[i], OwcaEmpty{});
		}
		return OwcaValue{ OwcaSet{ ds } };
	}
	static constexpr const size_t small_string_max_size = 32;
	OwcaValue VM::create_string_from_view(std::string_view txt)
	{
		if (txt.size() > small_string_max_size) {
			return create_string(std::string{ txt });
		}
		auto it = small_strings.find(txt);
		if (it != small_strings.end())
			return it->second;
		return create_string(std::string{ txt });
	}
	OwcaValue VM::create_string(std::string txt)
	{
		if (txt.size() <= small_string_max_size) {
			auto it = small_strings.find(txt);
			if (it != small_strings.end())
				return it->second;
		}
		auto vm = OwcaVM{ this };
		if ((std::uint32_t)txt.size() != txt.size()) {
			throw_overflow(std::format("string is too large ({}) - it's size will not fit in 32 bit unsigned integer", txt.size()));
		}
		auto str = allocate<String>(0, std::move(txt));
		auto os = OwcaString{ str };
		if (txt.size() <= small_string_max_size) {
			small_strings[str->text()] = os;
		}
		return OwcaValue{ os };
	}
	OwcaValue VM::create_string(OwcaValue str, size_t start, size_t end)
	{
		if (start >= end) return create_string("");
		auto size = end - start;
		auto vm = OwcaVM{ this };
		auto s = str.as_string(vm);
		auto sz = s.size();
		if (start >= sz) return create_string("");
		if (start + size > sz) size = sz - start;
		if (size == 0) return create_string("");
		auto dpth = s.internal_value()->depth();
		auto new_s = allocate<String>(0, String::Substr{ s.internal_value(), start, size, dpth + 1 });
		if (new_s->depth() > String::max_depth) {
			new_s->flatten();
		}
		auto os = OwcaString{ new_s };
		return os;
	}
	OwcaValue VM::create_string(OwcaValue str, size_t count)
	{
		if (count == 0) return create_string("");
		auto vm = OwcaVM{ this };
		auto s = str.as_string(vm);
		auto dpth = s.internal_value()->depth();
		auto new_s = allocate<String>(0, String::Mult{ s.internal_value(), count, dpth + 1 });
		if (new_s->depth() > String::max_depth) {
			new_s->flatten();
		}
		auto os = OwcaString{ new_s };
		return os;
	}
	OwcaValue VM::create_string(OwcaValue left, OwcaValue right)
	{
		auto vm = OwcaVM{ this };
		auto l = left.as_string(vm);
		auto r = right.as_string(vm);
		if (l.size() == 0) return right;
		if (r.size() == 0) return left;
		auto dpth = std::max(l.internal_value()->depth(), r.internal_value()->depth());
		auto new_s = allocate<String>(0, String::Add{ l.internal_value(), r.internal_value(), dpth + 1 });
		if (new_s->depth() > String::max_depth) {
			new_s->flatten();
		}
		auto os = OwcaString{ new_s };
		return os;
	}

	OwcaValue VM::get_identifier(unsigned int index)
	{
		auto& s = stacktrace.back();
		assert(index < s.values.size());
		return s.values[index];
	}
	void VM::set_identifier(unsigned int index, OwcaValue value, bool function_write)
	{
		auto& s = stacktrace.back();
		assert(index < s.values.size());
		if (function_write) {
			assert(value.kind() == OwcaValueKind::Functions);
			auto vm = OwcaVM{ this };
			auto fnc = value.as_functions(vm);
			assert(fnc.functions->functions.size() == 1);
			auto &dst = s.values[index];
			if (dst.kind() == OwcaValueKind::Functions) {
				auto dst_fnc = dst.as_functions(vm);
				for(auto it : fnc.functions->functions) {
					dst_fnc.functions->functions[it.first] = it.second;
				}
				return;
			}
		}
		s.values[index] = std::move(value);
	}
	std::shared_ptr<CodeBuffer> VM::currently_running_code() const
	{
		auto& s = stacktrace.back();
		return s.runtime_function->code;
	}
	bool VM::compare_values(const OwcaValue& left, const OwcaValue& right)
	{
		auto vm = OwcaVM{ this };
		return AstExprCompare::compare_equal(vm, left, right);
	}
	
	Class* VM::ensure_is_class(const OwcaValue&v)
	{
		auto vm = OwcaVM{ this };
		return v.as_class(vm).object;
	}

	Object* VM::ensure_is_object(const OwcaValue&v)
	{
		auto vm = OwcaVM{ this };
		return v.as_object(vm).object;
	}

	bool VM::calculate_if_true(const OwcaValue &r) {
		return r.visit(
			[&](OwcaEmpty value) -> bool {
				return false;
			},
			[&](OwcaRange value) -> bool {
				return value.lower().internal_value() != value.upper().internal_value();
			},
			[&](OwcaBool value) -> bool {
				return value.internal_value();
			},
			[&](OwcaInt value) -> bool {
				return value.internal_value() != 0;
			},
			[&](OwcaFloat value) -> bool {
				return value.internal_value() != 0.0;
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
			}
		);
	}
	size_t VM::calculate_hash(const OwcaValue& value) {
		auto vm = OwcaVM{ this };
		static auto calc_hash = [](const auto& q) {
			return std::hash<std::remove_cvref_t<decltype(q)>>()(q);
			};
		return value.visit(
			[](const OwcaEmpty& o) -> size_t { return 3; },
			[](const OwcaRange& o) -> size_t { 
				return calc_hash(o.lower().internal_value()) * 1009 + calc_hash(o.upper().internal_value()) + 4;
			},
			[](const OwcaInt& o) -> size_t { return calc_hash((OwcaFloatInternal)o.internal_value()) * 1013 + 5; },
			[](const OwcaFloat& o) -> size_t { return calc_hash(o.internal_value()) * 1013 + 5; },
			[](const OwcaBool& o) -> size_t { return (o.internal_value() ? 1 : 0) * 1021 + 7; },
			[](const OwcaString& o) -> size_t { return o.hash() * 1031 + 8; },
			[](const OwcaFunctions& o) -> size_t { return calc_hash(o.name()) * 1033 + 9; },
			[&](const OwcaMap& o) -> size_t {
				throw_not_hashable(value.type());
			},
			[&](const OwcaClass& o) -> size_t {
				return std::hash<void*>()(o.object) * 1009;
			},
			[&](const OwcaObject& o) -> size_t {
				throw_not_hashable(value.type());
			},
			[&](const OwcaArray &o) -> size_t {
				throw_not_hashable(value.type());
			},
			[&](const OwcaTuple &o) -> size_t {
				return o.object->hash();
			},
			[&](const OwcaSet &o) -> size_t {
				throw_not_hashable(value.type());
			}
		);
	}

	std::unique_ptr<IteratorBase> VM::create_iterator(const OwcaValue &r)
	{
		return r.visit(
			[](const OwcaRange& o) -> std::unique_ptr<IteratorBase> {
				auto lower = o.lower().internal_value();
				auto upper = o.upper().internal_value();
				OwcaIntInternal step = (lower > upper) ? -1 : 1;
				struct RangeIterator : public IteratorBase {
					OwcaIntInternal lower, upper, step;
					OwcaValue tmp;

					RangeIterator(OwcaIntInternal lower, OwcaIntInternal upper, OwcaIntInternal step) : lower(lower), upper(upper), step(step) {}

					OwcaValue *get() override {
						if (lower == upper) return nullptr;
						tmp = OwcaInt{ lower };
						return &tmp;
					}
					void next() override {
						lower += step;
					}
					size_t remaining_size() override {
						if (step > 0) return upper - lower;
						return lower - upper;
					}
				};
				return std::make_unique<RangeIterator>(lower, upper, step);
			},
			[&](const OwcaString& o) -> std::unique_ptr<IteratorBase> {
				struct StringIterator : public IteratorBase {
					OwcaValue tmp;
					OwcaString str;
					VM &vm;
					size_t pos = 0;

					StringIterator(OwcaString str, VM &vm) : str(str), vm(vm) {}

					OwcaValue *get() override {
						if (pos >= str.size()) return nullptr;
						tmp = str.substr(pos, 1);
						return &tmp;
					}
					void next() override {
						if (pos < str.size())
							pos += 1;
					}
					size_t remaining_size() override {
						return str.size() - pos;
					}
				};
				return std::make_unique<StringIterator>(o, *this);
			},
			[&](const OwcaMap& s) -> std::unique_ptr<IteratorBase> {
				struct MapIterator : public IteratorBase {
					OwcaValue tmp;
					OwcaMap::Iterator beg, end;
					VM &vm;
					size_t remaining = 0;

					MapIterator(VM &vm, OwcaMap m) : beg(m.begin()), end(m.end()), vm(vm), remaining(m.size()) {}

					OwcaValue *get() override {
						if (beg != end && remaining > 0) {
							auto v = *beg;
							tmp = vm.create_tuple({ v.first, v.second });
							return &tmp;
						}
						return nullptr;
					}
					void next() override {
						if (beg != end && remaining > 0) {
							++beg;
							--remaining;
						}
					}
					size_t remaining_size() override {
						return remaining;
					}
				};
				return std::make_unique<MapIterator>(*this, s);
			},
			[&](const OwcaArray& s) -> std::unique_ptr<IteratorBase> {
				struct ArrayIterator : public IteratorBase {
					OwcaArray s;
					size_t pos = 0;

					ArrayIterator(OwcaArray s) : s(s) {}

					OwcaValue *get() override {
						if (pos < s.object->values.size()) return &s.object->values[pos];
						return nullptr;
					}
					void next() override {
						if (pos < s.object->values.size())
							++pos;
					}
					size_t remaining_size() override {
						return s.object->values.size() - pos;
					}
				};
				return std::make_unique<ArrayIterator>(s);
			},
			[&](const OwcaTuple& s) -> std::unique_ptr<IteratorBase> {
				struct TupleIterator : public IteratorBase {
					OwcaTuple s;
					size_t pos = 0;

					TupleIterator(OwcaTuple s) : s(s) {}

					OwcaValue *get() override {
						if (pos < s.object->values.size()) return &s.object->values[pos];
						return nullptr;
					}
					void next() override {
						if (pos < s.object->values.size())
							++pos;
					}
					size_t remaining_size() override {
						return s.object->values.size() - pos;
					}
				};
				return std::make_unique<TupleIterator>(s);
			},
			[&](const OwcaSet& s) -> std::unique_ptr<IteratorBase> {
				struct SetIterator : public IteratorBase {
					OwcaValue tmp;
					OwcaSet::Iterator beg, end;
					VM &vm;
					size_t remaining = 0;

					SetIterator(VM &vm, OwcaSet m) : beg(m.begin()), end(m.end()), vm(vm), remaining(m.size()) {}

					OwcaValue *get() override {
						if (beg != end && remaining > 0) {
							return const_cast<OwcaValue*>(&*beg);
						}
						return nullptr;
					}
					void next() override {
						if (beg != end && remaining > 0) {
							++beg;
							--remaining;
						}
					}
					size_t remaining_size() override {
						return remaining;
					}
				};
				return std::make_unique<SetIterator>(*this, s);
			},
			[&](const auto &) -> std::unique_ptr<IteratorBase> {
				throw_not_iterable(r.type());
			}
			);
	}

	void VM::run_gc() {
		assert(stacktrace.empty());

		auto ggc = GenerationGC{ ++generation_gc };
		// mark
		for(auto &s : small_strings) {
			gc_mark(s.second, ggc);
		}
		for (auto& s : stacktrace) {
			gc_mark(s.runtime_function, ggc);
			gc_mark(s.runtime_functions, ggc);
			gc_mark(s.values, ggc);
		}
		for(auto s : allocated_objects) {
			gc_mark(s, ggc);
		}
		for(auto &s : builtin_objects) {
			gc_mark(s.second, ggc);
		}

		// sweep
		AllocationBase *valid = &root_allocated_memory;
		while (valid->next != &root_allocated_memory) {
			auto obj = valid->next;
			if (obj->last_gc_mark != ggc) {
				valid->next = obj->next;
				obj->next->prev = valid;
				delete obj;
			}
			else {
				valid = valid->next;
			}
		}
	}

	void VM::gc_mark(AllocationBase* ptr, GenerationGC ggc)
	{
		if (ptr->last_gc_mark != ggc) {
			ptr->last_gc_mark = ggc;
			ptr->gc_mark(*this, ggc);
		}
	}

	void VM::gc_mark(const OwcaValue& o, GenerationGC ggc)
	{
		o.visit(
			[](const OwcaEmpty& o) { },
			[](const OwcaRange& o) { },
			[](const OwcaInt& o) { },
			[](const OwcaFloat& o) { },
			[](const OwcaBool& o) { },
			[](const OwcaString& o) { },
			[&](const OwcaFunctions& s) {
				gc_mark(s.functions, ggc);
				if (s.self_object)
					gc_mark(s.self_object, ggc);
			},
			[&](const OwcaMap& s) {
				gc_mark(s.dictionary, ggc);
			},
			[&](const OwcaClass& s) {
				gc_mark(s.object, ggc);
			},
			[&](const OwcaObject& s) {
				gc_mark(s.object, ggc);
			},
			[&](const OwcaArray& s) {
				gc_mark(s.object->values, ggc);
			},
			[&](const OwcaTuple& s) {
				gc_mark(s.object->values, ggc);
			},
			[&](const OwcaSet& s) {
				gc_mark(s.dictionary, ggc);
			}
			);
	}
	void VM::gc_mark(const std::vector<OwcaValue>& o, GenerationGC ggc)
	{
		for (auto& q : o) {
			gc_mark(q, ggc);
		}
	}

}