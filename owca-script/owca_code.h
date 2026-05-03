#ifndef RC_OWCA_SCRIPT_OWCA_CODE_H
#define RC_OWCA_SCRIPT_OWCA_CODE_H

#include "stdafx.h"
#include "line.h"

namespace OwcaScript {
    struct NativeCodeProvider;
    namespace Internal {
        enum class DataKind : std::uint8_t;
        struct LineEntry;
        class CodePosition;
    }

	class OwcaCode {
        struct Impl {
            using DataKindsType = std::unordered_map<const unsigned char *, Internal::DataKind>;
            std::span<const unsigned char> code;
            const DataKindsType *data_kinds;
            std::span<const Internal::LineEntry> lines;
			std::string_view filename;
            std::shared_ptr<NativeCodeProvider> native_code_provider;
            std::function<void()> on_destroy;

            Impl() = default;
            Impl(Impl&&) = default;
            Impl& operator=(Impl&&) = default;
            Impl(const Impl&) = delete;
            Impl& operator=(const Impl&) = delete;
            ~Impl() {
                if (on_destroy) on_destroy();
            }
        };
        std::shared_ptr<Impl> code_;
    public:
        OwcaCode(std::string_view filename, std::span<const unsigned char> code, const Impl::DataKindsType &data_kinds, std::span<const Internal::LineEntry> lines, std::shared_ptr<NativeCodeProvider> native_code_provider, std::function<void()> on_destroy) : code_(std::make_shared<Impl>()) {
            code_->filename = filename;
            code_->code = code;
            code_->data_kinds = &data_kinds;
            code_->lines = lines;
            code_->native_code_provider = std::move(native_code_provider);
            code_->on_destroy = std::move(on_destroy);
        }

		const auto filename() const { return code_->filename; }
        const auto code() const { return code_->code; }
        const auto &data_kinds() const { return *code_->data_kinds; }
        const auto lines() const { return code_->lines; }
        const auto &native_code_provider() const { return code_->native_code_provider; }
		Internal::Line first_line() const;
        Internal::Line get_line_by_position(Internal::CodePosition pos) const;
	};
}

#endif
