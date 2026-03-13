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

namespace OwcaScript {
    namespace Internal {
		template <typename T, typename ... F> auto visit_variant(T && t, F &&...fns) {
			struct overloaded : F... {
				using F::operator()...;
			};
			return std::visit(overloaded{std::forward<F>(fns)...}, std::forward<T>(t));
		}	
    }
}
#endif
