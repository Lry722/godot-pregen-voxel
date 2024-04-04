#pragma once
#include <functional>
#include <type_traits>

namespace pgvoxel {
template <typename F, typename... Args>
requires requires(F f, Args... args) { std::invoke(f, args...); }
struct scope_guard {
	F f;
	std::tuple<Args...> values;

	template <typename Fn, typename... Ts>
	scope_guard(Fn &&func, Ts &&...args) :
			f{ std::forward<Fn>(func) }, values{ std::forward<Ts>(args)... } {}
	~scope_guard() {
		if (!released) {
			std::apply(f, values);
		}
	}
	scope_guard(const scope_guard &) = delete;
	void release() { released = true; }

private:
	bool released{ false };
};

template <typename F, typename... Args>
scope_guard(F &&, Args &&...) -> scope_guard<std::decay_t<F>, std::decay_t<Args>...>;
} // namespace pgvoxel
