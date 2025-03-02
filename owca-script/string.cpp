#include "stdafx.h"
#include "string.h"
#include "vm.h"

namespace OwcaScript::Internal {
	std::string_view String::text() const
	{
        flatten();
        return std::get<std::string>(data);
	}

    void String::flatten() const
    {
        if (std::get_if<std::string>(&data) != nullptr) return;

        std::string tmp;
        iterate_over_content(
            [&](std::string_view v) {
                tmp += v;
            }
        );
        data = std::move(tmp);
    }
	size_t String::size() const
	{
        return visit(
            [&](const std::string &t) {
                return t.size();
            },
            [&](Substr t) {
                return t.size;
            },
            [&](Mult t) {
                return t.child->size() * t.count;
            },
            [&](Add t) {
                return t.left->size() + t.right->size();
            }
        );
    }

    unsigned int String::depth() const
    {
        return visit(
            [&](const std::string &t) -> unsigned int {
                return 1;
            },
            [&](Substr t) {
                return t.depth;
            },
            [&](Mult t) {
                return t.depth;
            },
            [&](Add t) {
                return t.depth;
            }
        );
    }

    void String::iterate_over_content(std::function<void(std::string_view)> func) const
    {
        std::function<void(const String*, size_t, size_t)> proc = [&](const String *s, size_t start, size_t size) {
            s->visit(
                [&](const std::string &t) {
                    if (start >= t.size()) return;
                    if (start + size > t.size())
                        size = t.size() - start;
                    func(std::string_view{ t }.substr(start, size));
                },
                [&](Substr t) {
                    if (start >= t.size) return;
                    if (start + size > t.size)
                        size = t.size - start;
                    proc(t.child, t.start + start, size);
                },
                [&](Mult t) {
                    auto sz = t.child->size();
                    if (start >= sz * t.count) return;
                    if (start + size > sz * t.count)
                        size = sz * t.count - start;
                    
                    auto left_start = start % sz;
                    if (sz - left_start >= size) {
                        proc(t.child, left_start, size);
                        return;
                    }
                    proc(t.child, left_start, sz - left_start);
                    start += sz - left_start;
                    size -= sz - left_start;

                    while(size >= sz) {
                        proc(t.child, 0, sz);
                        size -= sz;
                    }

                    auto right_end = size % sz;
                    if (right_end > 0) {
                        proc(t.child, 0, right_end);
                    }
                },
                [&](Add t) {
                    auto lsz = t.left->size();
                    auto rsz = t.right->size();

                    if (start < lsz) {
                        if (start + size <= lsz) {
                            proc(t.left, start, size);
                            return;
                        }
                        proc(t.left, start, lsz - start);
                        size -= lsz - start;
                        start = lsz;
                    }
                    assert(start >= lsz);
                    if (start + size > lsz + rsz) {
                        size = lsz + rsz - start;
                    }
                    if (size > 0) {
                        proc(t.right, start - lsz, size);
                    }
                }
            );
        };
        proc(this, 0, std::numeric_limits<size_t>::max());
    }

	std::string String::to_string() const
	{
        std::string tmp;
        tmp.reserve(size());
        iterate_over_content([&](std::string_view t) {
            tmp += t;
        });
        assert(tmp.size() == size());
        return tmp;
	}

    size_t String::hash() const
    {
        if (!hash_calculated) {
            hash_calculated = true;
            hash_value = 0;
            iterate_over_content([&](std::string_view tmp) {
                hash_value = hash_value * 7057 + std::hash<std::string_view>()(tmp);
            });
        }
        return hash_value;
    }

	void String::gc_mark(OwcaVM vm, GenerationGC generation_gc)
	{
        visit(
            [&](const std::string &t) {},
            [&](Substr t) {
                VM::get(vm).gc_mark(t.child, generation_gc);
            },
            [&](Mult t) {
                VM::get(vm).gc_mark(t.child, generation_gc);
            },
            [&](Add t) {
                VM::get(vm).gc_mark(t.left, generation_gc);
                VM::get(vm).gc_mark(t.right, generation_gc);
            }
        );
	}
}