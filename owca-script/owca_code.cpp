#include "owca-script/identifier_index.h"
#include "owca-script/owca_value.h"
#include "stdafx.h"
#include "owca_code.h"
#include "exec_buffer.h"
#include "vm.h"
#include <string>

namespace OwcaScript {
    OwcaCodeBuffer::OwcaCodeBuffer(std::string filename, std::vector<unsigned char> code, std::vector<Internal::LineEntry> lines, Internal::DataKindsType data_kinds)
        : filename_(std::move(filename)), code_(std::move(code)), lines_(std::move(lines)), data_kinds_(std::move(data_kinds)) {}

    Internal::OwcaCode::OwcaCode(VM &vm, const OwcaCodeBuffer &code_buffer, std::shared_ptr<NativeCodeProvider> native_code_provider) : code_(std::make_shared<Impl>())
    {
#ifdef DEBUG
        auto code_pos = Internal::CodePosition{ code_buffer.code().data(), code_buffer.data_kinds() };
#else
        auto code_pos = Internal::CodePosition{ code_buffer.code().data() };
#endif

        code_->globals_count = code_pos.decode<std::uint32_t>();
        code_->max_values_count = code_pos.decode<std::uint32_t>();
        for(auto i = 0u; i < code_->globals_count; ++i) {
            auto ident = code_pos.decode<std::string_view>();
            auto ident_index = vm.get_identifier_index(ident);
            code_->identifier_to_global_index[ident_index] = i;
        }
        auto constants_strings = code_pos.decode<std::uint32_t>();
        auto constants_numbers = code_pos.decode<std::uint32_t>();
        code_->constants_vector.reserve(3 + constants_strings + constants_numbers);
        code_->constants_vector.push_back(OwcaEmpty{});
        code_->constants_vector.push_back(true);
        code_->constants_vector.push_back(false);
        for(auto i = 0u; i < constants_strings; ++i) {
            auto str = code_pos.decode<std::string_view>();
            code_->constants_vector.push_back(vm.create_string_from_view(str));
        }
        for(auto i = 0u; i < constants_numbers; ++i) {
            auto num = code_pos.decode<Number>();
            code_->constants_vector.push_back(num);
        }

        auto code_size = code_pos.decode<std::uint32_t>();
        const auto start_code = code_pos.value() - code_buffer.code().data();
        code_->code_offset = (std::uint32_t)start_code;
        code_->code.resize(code_size);
        std::memcpy(code_->code.data(), code_pos.value(), code_size);
        code_pos += code_size;

#ifdef DEBUG
        DataKindsType new_data_kinds;
        for(auto it : code_buffer.data_kinds()) {
            auto offset = it.first - code_buffer.code().data();
            if (offset >= start_code && offset <= start_code + code_size)
                new_data_kinds[code_->code.data() + offset - start_code] = it.second;
        }
        code_->data_kinds = std::move(new_data_kinds);
#else
        code_->data_kinds = code_buffer.data_kinds();
#endif

        auto identifier_count = code_pos.decode<std::uint32_t>();
        std::string temp;
        for (std::uint32_t i = 0; i < identifier_count; ++i) {
            temp = code_pos.decode<std::string_view>();
            auto ii = vm.get_identifier_index(temp);
            auto places = code_pos.decode<std::uint32_t>();
            for (std::uint32_t j = 0; j < places; ++j) {
                auto p = code_pos.value() - code_buffer.code().data();
                auto offset = code_pos.decode<std::uint32_t>();
                if (!(offset >= start_code && offset < start_code + code_size)) {
                    std::cerr << "Failed to process identifier `" << temp << "` at position " << p << " offset " << offset << " start " << start_code << " end " << (start_code + code_size) << std::endl;
                    assert(offset >= start_code && offset < start_code + code_size);
                }
                ExecuteBufferWriter::set_identifier_index(code_->code.data() + offset - start_code, ii);
            }
        }

        code_->filename = code_buffer.filename();
        code_->lines = code_buffer.lines();
        code_->native_code_provider = std::move(native_code_provider);

        size_t begin = 0;
        while(begin < code_->lines.size() && code_->lines[begin].code_pos < start_code) {
            ++begin;
        }
        size_t i = 0u;
        for(; begin + i < code_->lines.size() && code_->lines[begin + i].code_pos < start_code + code_size; ++i) {
            code_->lines[i] = { (std::uint32_t)(code_->lines[begin + i].code_pos - start_code), code_->lines[begin + i].line };
        }
        code_->lines.resize(i);
    }
    Internal::Line Internal::OwcaCode::get_line_by_position(Internal::CodePosition pos) const
    {
        auto offset = pos.value() - code().data();
        assert(offset == -1 || offset >= 0 && offset <= code().size());
        auto it = std::lower_bound(code_->lines.begin(), code_->lines.end(), offset, [](const Internal::LineEntry &entry, size_t pos) {
            return entry.code_pos < pos;
        });
        if (it == code_->lines.end()) {
            if (code_->lines.empty())
                return Internal::Line{ 0 };
            return Internal::Line{ code_->lines.back().line };
        }
        if (it == code_->lines.begin() || it->code_pos == offset) {
            return Internal::Line{ it->line };
        }
        --it;
        return Internal::Line{ it->line };
    }
    Internal::CodePosition Internal::OwcaCode::code_position() const {
#ifdef DEBUG
        return Internal::CodePosition{ code().data(), data_kinds() };
#else
        return Internal::CodePosition{ code().data() };
#endif
    }

    void Internal::gc_mark_value(GenerationGC gc, const Internal::OwcaCode &code) {
        if (gc != code.code_->gc) {
            code.code_->gc = gc;
            gc_mark_value(gc, code.code_->constants_vector);
        }
    }
}
