#ifndef RC_OWCA_SCRIPT_STDAFX_H
#define RC_OWCA_SCRIPT_STDAFX_H

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <charconv>
#include <chrono>
#include <chrono>
#include <cmath>
#include <coroutine>
#include <cstdint>
#include <deque>
#include <exception>
#include <format>
#include <functional>
#include <generator>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <memory>
#include <optional>
#include <span>
#include <sstream>
#include <string_view>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

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
