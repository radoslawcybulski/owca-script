#ifndef RC_OWCA_SCRIPT_OWCA_CODE_H
#define RC_OWCA_SCRIPT_OWCA_CODE_H

#include "stdafx.h"
#include "exec_buffer.h"
#include "line.h"

namespace OwcaScript {
	class OwcaCode {
        struct Impl {
            std::span<const unsigned char> code;
            std::span<const Internal::ExecuteBufferReader::DataKind> data_kinds;
            std::span<const Internal::ExecuteBufferReader::LineEntry> lines;
			std::string_view filename;
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
        OwcaCode(std::string_view filename, std::span<const unsigned char> code, std::span<const Internal::ExecuteBufferReader::DataKind> data_kinds, std::span<const Internal::ExecuteBufferReader::LineEntry> lines, std::function<void()> on_destroy) : code_(std::make_shared<Impl>()) {
            code_->filename = filename;
            code_->code = code;
            code_->data_kinds = data_kinds;
            code_->lines = lines;
            code_->on_destroy = std::move(on_destroy);
        }

		const auto filename() const { return code_->filename; }
        const auto code() const { return code_->code; }
        const auto data_kinds() const { return code_->data_kinds; }
        const auto lines() const { return code_->lines; }
		Internal::Line first_line() const;
	};
}

#endif
