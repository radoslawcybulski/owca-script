#ifndef RC_OWCA_SCRIPT_NATIVE_CLASS_INTERFACE_H
#define RC_OWCA_SCRIPT_NATIVE_CLASS_INTERFACE_H

namespace OwcaScript {
	class OwcaValue;
	class GenerationGC;
	class OwcaVM;
	
	struct NativeClassInterface {
		virtual ~NativeClassInterface() = default;

		virtual void initialize_storage(void* ptr, size_t s) = 0;
		virtual void destroy_storage(void* ptr, size_t s) = 0;
		virtual void gc_mark_members(const void* ptr, size_t s, OwcaVM vm, GenerationGC generation_gc) = 0;
		virtual size_t native_storage_size() = 0;
		virtual bool get_member(OwcaVM vm, std::string_view, std::span<char> native_storage, OwcaValue &) { return false; }
		virtual bool set_member(OwcaVM vm, std::string_view, std::span<char> native_storage, const OwcaValue &) { return false; }
	};
	template <typename T> struct NativeClassInterfaceSimpleImplementation : public NativeClassInterface {
		void initialize_storage(void* ptr, size_t s) override { new (ptr) T(); }
		void destroy_storage(void* ptr, size_t s) override { ((T*)ptr)->~T(); }
		void gc_mark_members(const void* ptr, size_t s, OwcaVM vm, GenerationGC generation_gc) override { gc_mark_value(vm, generation_gc, *(const T*)ptr); }
		size_t native_storage_size() override { return sizeof(T); }
	};
}

#endif