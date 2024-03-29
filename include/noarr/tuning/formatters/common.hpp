#ifndef NOARR_TUNING_FORMATTERS_COMMON_HPP
#define NOARR_TUNING_FORMATTERS_COMMON_HPP

#include <memory>
#include <utility>
#include <vector>

#include "../formatter.hpp"

namespace noarr::tuning {

// TODO: access to a specific formatter
template<class ...Formatters>
class combined_formatter {
public:
	combined_formatter(std::unique_ptr<Formatters> &&...formatters)
		: formatters_(std::move(formatters)...)
	{}

	constexpr void header() {
		return header_impl(std::index_sequence_for<Formatters...>());
	}

	constexpr void footer() {
		return footer_impl(std::index_sequence_for<Formatters...>());
	}

	template<class ...Pars>
	constexpr void format(Pars &&...pars) {
		return format_impl(std::index_sequence_for<Formatters...>(), std::forward<Pars>(pars)...);
	}

private:
	template<std::size_t ...Idxs>
	constexpr void header_impl(std::index_sequence<Idxs...>) {
		return (... , std::get<Idxs>(formatters_)->header());
	}

	template<std::size_t ...Idxs>
	constexpr void footer_impl(std::index_sequence<Idxs...>) {
		return (... , std::get<Idxs>(formatters_)->footer());
	}

	template<class ...Pars, std::size_t ...Idxs>
	constexpr void format_impl(std::index_sequence<Idxs...>, Pars &&...pars) {
		return (... , std::get<Idxs>(formatters_)->format(std::forward<Pars>(pars)...));
	}

	std::tuple<std::unique_ptr<Formatters> ...> formatters_;
};

static_assert(IsTunerFormatter<combined_formatter<>>);

template<class Formatter>
class lazy_formatter {
public:
	template<class ...Args>
	lazy_formatter(Args &&...args) noexcept(noexcept(std::unique_ptr<Formatter>(std::forward<Args>(args)...)))
		: formatter_(std::forward<Args>(args)...)
	{}

	class arg_pack_base {
	public:
		virtual ~arg_pack_base() = default;
		virtual constexpr void format(Formatter &formatter) const = 0;
	};

	template<class ...Args>
	class arg_pack : public arg_pack_base {
	private:
		std::tuple<Args &&...> args_;

	public:
		arg_pack(Args &&...args) : args_(std::forward<Args>(args)...) {}
		~arg_pack() override = default;

		constexpr void format(Formatter &formatter) const noexcept override {
			format_impl(formatter, std::index_sequence_for<Args...>());
		}

	private:
		template<std::size_t ...Idxs>
		constexpr void format_impl(Formatter &formatter, std::index_sequence<Idxs...>) const noexcept {
			formatter.format(std::get<Idxs>(args_)...);
		}
	};

	constexpr void header() const noexcept {
		// we are lazy!
	}

	void footer() {
		// oh *%^!, time to do the job...
		formatter_->header();

		for (auto &&arg_pack : cache_)
			arg_pack->format(*formatter_);

		formatter_->footer();
	}

	template<class ...Args>
	void format(Args &&...args) {
		cache_.emplace_back(std::make_unique<arg_pack<Args...>>(std::forward<Args>(args)...));
	}

private:
	std::unique_ptr<Formatter> formatter_;
	std::vector<std::unique_ptr<arg_pack_base>> cache_;
};

// TODO: add static_asserts for lazy_formatter

} // namespace noarr::tuning

#endif // NOARR_TUNING_FORMATTERS_COMMON_HPP