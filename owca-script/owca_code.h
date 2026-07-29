#ifndef RC_OWCA_SCRIPT_OWCA_CODE_H
#define RC_OWCA_SCRIPT_OWCA_CODE_H

#include "owca-script/identifier_index.h"
#include "stdafx.h"
#include "line.h"
#include "garbage.h"

namespace OwcaScript {
    struct NativeCodeProvider;
    namespace Internal {
        enum class DataKind : std::uint8_t;
        struct LineEntry;
        class CodePosition;
        using DataKindsType = std::unordered_map<const unsigned char *, DataKind>;
    }

    class OwcaCodeBuffer {
        std::string filename_;
        std::vector<unsigned char> code_;
        std::vector<Internal::LineEntry> lines_;
        Internal::DataKindsType data_kinds_;
    public:
        OwcaCodeBuffer(std::string filename, std::vector<unsigned char> code, std::vector<Internal::LineEntry> lines, Internal::DataKindsType data_kinds);

        const auto& filename() const { return filename_; }
        const auto& code() const { return code_; }
        const auto& lines() const { return lines_; }
        const auto& data_kinds() const { return data_kinds_; }
    };

    namespace Internal {
        class VM;

    	class OwcaCode {
            struct Impl {
                std::vector<unsigned char> code;
                Internal::DataKindsType data_kinds;
                std::vector<Internal::LineEntry> lines;
    			std::string filename;
                std::vector<OwcaValue> constants_vector;
                std::unordered_map<IdentifierIndex, size_t> identifier_to_global_index;
                std::uint32_t string_constants_count;
                std::uint32_t globals_count;
                std::uint32_t max_values_count;
                std::shared_ptr<NativeCodeProvider> native_code_provider;
                GenerationGC gc;
            };
            std::shared_ptr<Impl> code_;
        public:
            OwcaCode(VM &vm, const OwcaCodeBuffer &code_buffer, std::shared_ptr<NativeCodeProvider> native_code_provider);

    		std::string_view filename() const { return code_->filename; }
            std::span<const unsigned char> code() const { return code_->code; }
            const auto &data_kinds() const { return code_->data_kinds; }
            const auto &lines() const { return code_->lines; }
            const auto &native_code_provider() const { return code_->native_code_provider; }
            std::uint32_t string_constants_count() const { return code_->string_constants_count; }
            std::uint32_t globals_count() const { return code_->globals_count; }
            std::uint32_t max_values_count() const { return code_->max_values_count; }
            auto constants() { return code_->constants_vector.data(); }
            const auto &identifier_to_global_index() const { return code_->identifier_to_global_index; }

            Internal::CodePosition code_position() const;
    		Internal::Line first_line() const;
            Internal::Line get_line_by_position(Internal::CodePosition pos) const;

            friend void gc_mark_value(GenerationGC gc, const Internal::OwcaCode &);
    	};
    }
}

#endif
