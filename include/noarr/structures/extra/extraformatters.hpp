#ifndef NOARR_STRUCTURES_EXTRAFORMATTERS_HPP
#define NOARR_STRUCTURES_EXTRAFORMATTERS_HPP

#include <cstdint>
#include <memory>
#include <utility>
#include <variant>
#include <vector>

#include "../extra/metaformatter.hpp"

namespace noarr::tuning {

// TODO: access to a specific formatter
template<class ...Formatters>
class combined_formatter {
public:
	combined_formatter(std::unique_ptr<Formatters> &&...formatters)
		: formatters_(std::forward<std::unique_ptr<Formatters>>(formatters)...)
	{}

	void header() {
		return header_impl(std::make_index_sequence<sizeof...(Formatters)>());
	}

	void footer() {
		return footer_impl(std::make_index_sequence<sizeof...(Formatters)>());
	}

	void format(const begin_parameter &par) {
		return format_impl(par, std::make_index_sequence<sizeof...(Formatters)>());
	}

	void format(const end_parameter &par) {
		return format_impl(par, std::make_index_sequence<sizeof...(Formatters)>());
	}

	void format(const name_parameter &par) {
		return format_impl(par, std::make_index_sequence<sizeof...(Formatters)>());
	}

	void format(const category_parameter &par) {
		return format_impl(par, std::make_index_sequence<sizeof...(Formatters)>());
	}

	void format(const multiple_choice_parameter &par) {
		return format_impl(par, std::make_index_sequence<sizeof...(Formatters)>());
	}

	void format(const permutation_parameter &par) {
		return format_impl(par, std::make_index_sequence<sizeof...(Formatters)>());
	}

private:
	template<std::size_t ...Idxs>
	void header_impl(std::index_sequence<Idxs...>) {
		return (... , std::get<Idxs>(formatters_)->header());
	}

	template<std::size_t ...Idxs>
	void footer_impl(std::index_sequence<Idxs...>) {
		return (... , std::get<Idxs>(formatters_)->footer());
	}

	template<std::size_t ...Idxs>
	void format_impl(const begin_parameter &par, std::index_sequence<Idxs...>) {
		return (... , std::get<Idxs>(formatters_)->format(par));
	}

	template<std::size_t ...Idxs>
	void format_impl(const end_parameter &par, std::index_sequence<Idxs...>) {
		return (... , std::get<Idxs>(formatters_)->format(par));
	}

	template<std::size_t ...Idxs>
	void format_impl(const name_parameter &par, std::index_sequence<Idxs...>) {
		return (... , std::get<Idxs>(formatters_)->format(par));
	}

	template<std::size_t ...Idxs>
	void format_impl(const category_parameter &par, std::index_sequence<Idxs...>) {
		return (... , std::get<Idxs>(formatters_)->format(par));
	}

	template<std::size_t ...Idxs>
	void format_impl(const multiple_choice_parameter &par, std::index_sequence<Idxs...>) {
		return (... , std::get<Idxs>(formatters_)->format(par));
	}

	template<std::size_t ...Idxs>
	void format_impl(const permutation_parameter &par, std::index_sequence<Idxs...>) {
		return (... , std::get<Idxs>(formatters_)->format(par));
	}

	std::tuple<std::unique_ptr<Formatters> ...> formatters_;
};

static_assert(IsTunerFormatter<combined_formatter<>>);

template<class Formatter>
class lazy_formatter {
public:
	lazy_formatter(std::unique_ptr<Formatter> &&formatter)
		: formatter_(std::forward<std::unique_ptr<Formatter>>(formatter))
	{}

	void header() const noexcept {
		// we are lazy!
	}

	void footer() const {
		// oh *%^!, time to do the job...
		formatter_->header();

		for (auto &&par : cache_) {
			std::visit(
				[this](const auto *par) { return formatter_->format(*par); },
				par);
		}

		formatter_->footer();
	}


	void format(const begin_parameter &par) {
		cache_.emplace_back(&par);
	}

	void format(const end_parameter &par) {
		cache_.emplace_back(&par);
	}

	void format(const name_parameter &par) {
		cache_.emplace_back(&par);
	}

	void format(const category_parameter &par) {
		cache_.emplace_back(&par);
	}

	void format(const multiple_choice_parameter &par) {
		cache_.emplace_back(&par);
	}

	void format(const permutation_parameter &par) {
		cache_.emplace_back(&par);
	}

private:
	std::unique_ptr<Formatter> formatter_;
	std::vector<std::variant<
		const begin_parameter *,
		const end_parameter *,
		const name_parameter *,
		const category_parameter *,
		const multiple_choice_parameter *,
		const permutation_parameter *>> cache_;
};

// TODO: add static_asserts for lazy_formatter

} // namespace noarr::tuning

#endif // NOARR_STRUCTURES_EXTRAFORMATTERS_HPP