#ifndef NOARR_TUNING_DUMMY_FORMATTER_HPP
#define NOARR_TUNING_DUMMY_FORMATTER_HPP

namespace noarr::tuning {

class dummy_formatter {
public:
    void header() const noexcept { }

    template<class ...Args>
    void footer(Args &&...) const noexcept { }

    template<class Parameter>
    void format(const char *name, const Parameter &par) const noexcept { }

    template<class Parameter, class Constraint>
    auto format(const char *name, const Parameter &par, Constraint &&constraint) const noexcept { }
};

} // namespace noarr::tuning

#endif // NOARR_TUNING_DUMMY_FORMATTER_HPP
