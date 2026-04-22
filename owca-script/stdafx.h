#ifndef RC_OWCA_SCRIPT_STDAFX_H
#define RC_OWCA_SCRIPT_STDAFX_H

#include <cstdint>
#include <vector>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <variant>
#include <cassert>
#include <span>
#include <format>
#include <optional>
#include <tuple>
#include <limits>
#include <charconv>
#include <sstream>
#include <functional>
#include <iterator>
#include <algorithm>
#include <coroutine>
#include <iostream>
#include <deque>
#include <array>
#include <cstring>
#include <cmath>
#include <bit>
#include <chrono>
#include <list>
#include <generator>

namespace OwcaScript {
    using Number = double;
	class OwcaValue;
    class Generator;
    class Function;
	class OwcaVM;
	class GenerationGC;
	class OwcaEmpty {};
	class OwcaCompleted {};

    namespace Internal {
		template <typename T, typename ... F> auto visit_variant(T && t, F &&...fns) {
			struct overloaded : F... {
				using F::operator()...;
			};
			return std::visit(overloaded{std::forward<F>(fns)...}, std::forward<T>(t));
        }
        struct StringHash {
            using is_transparent = void;
            size_t operator()(std::string_view str) const {
                return std::hash<std::string_view>{}(str);
            }
            size_t operator()(const std::string &str) const {
                return std::hash<std::string_view>{}(str);
            }
        };
        struct StringCmp {
            using is_transparent = void;
            bool operator () (std::string_view a, std::string_view b) const {
                return a == b;
            }
            bool operator () (const std::string &a, std::string_view b) const {
                return a == b;
            }
            bool operator () (std::string_view a, const std::string &b) const {
                return a == b;
            }
            bool operator () (const std::string &a, const std::string &b) const {
                return a == b;
            }
        };
    }
}
#endif
