#ifndef NOARR_TUNING_FORMATTER_HPP
#define NOARR_TUNING_FORMATTER_HPP

#include <cstddef>
#include <concepts>

namespace noarr::tuning {

// TODO: distribution?
struct category_parameter {
	constexpr category_parameter(std::size_t num) noexcept : num_(num) {}

	std::size_t num_;
};

template<class Start, class End, class Step>
struct range_parameter {
	constexpr range_parameter(End max) noexcept : min_(0), max_(max), step_((Step)1) {}
	constexpr range_parameter(Start min, End max, Step step = (Step)1) noexcept : min_(min), max_(max), step_(step) {}

	Start min_;
	End max_;
	Step step_;
};

struct permutation_parameter {
	constexpr permutation_parameter(std::size_t num) noexcept : num_(num) {}

	std::size_t num_;
};

struct predicate_parameter_base {
	virtual ~predicate_parameter_base() = default;
};

template<class Predicate>
struct predicate_parameter {
private:
	Predicate predicate_;

public:
	constexpr predicate_parameter(Predicate predicate) noexcept : predicate_(predicate) {}

	Predicate predicate() const noexcept { return predicate_; }

	template<class T>
	constexpr bool operator()(T &&arg) const noexcept {
		return predicate_(arg);
	}

	constexpr Predicate get() const noexcept { return predicate_; }
	constexpr operator Predicate() const noexcept { return predicate_; }
};

template<class T>
concept IsTunerFormatter = requires(T t) {
	{ t.header() };
	{ t.footer() };

	{ t.format("name", category_parameter(0)) };
	{ t.format("name", permutation_parameter(0)) };
	{ t.format("name", range_parameter<std::size_t, std::size_t, std::size_t>(0, 0, 0)) };
};

template<class T>
concept IsConstrainedTunerFormatter = IsTunerFormatter<T> && requires(T t) {
	{ t.format("name", category_parameter(0), predicate_parameter([](auto &&) { return true; })) };
	{ t.format("name", permutation_parameter(0), predicate_parameter([](auto &&) { return true; })) };
	{ t.format("name", range_parameter<std::size_t, std::size_t, std::size_t>(0, 0, 0), predicate_parameter([](auto &&) { return true; })) };
};

} // namespace noarr::tuning

#endif // NOARR_TUNING_FORMATTER_HPP
