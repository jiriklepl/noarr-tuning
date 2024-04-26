#ifndef NOARR_TUNING_ATF_FORMATTER_HPP
#define NOARR_TUNING_ATF_FORMATTER_HPP

#include <cstring>

#include <stdexcept>
#include <string>
#include <utility>

#include <atf.hpp>

#include "../builders.hpp"
#include "../formatter.hpp"
#include "../macros.hpp"

#define NOARR_ATF_TP(name) (&*NOARR_TUNING_PARAMETER_DEFINITION(name))

namespace noarr::tuning {

template<class abort_condition_t, IsCompileCommandBuilder CompileCommandBuilder, IsRunCommandBuilder RunCommandBuilder>
class atf_formatter {
public:
	atf_formatter(abort_condition_t abort_condition, CompileCommandBuilder compile_command_builder, RunCommandBuilder run_command_builder)
		: abort_condition_(std::move(abort_condition))
		, compile_command_builder_(std::move(compile_command_builder))
		, run_command_builder_(std::move(run_command_builder))
	{
		compile_command_builder_.add_flag("-DNOARR_PASS_BY_DEFINE");
	}

	constexpr void header() const noexcept {}

	template<class ...Pars>
	void footer(Pars &&...parameters) {
		auto compile_script = compile_command_builder_.to_string();
		auto run_script = run_command_builder_.to_string();

		auto cost_function = atf::generic::cost_function(run_script)
			.compile_command(compile_script);

		auto tuner = atf::tuner()
			.tuning_parameters(*parameters...)
			.search_technique(atf::auc_bandit())
			.tune(cost_function, abort_condition_);
	}

	auto format(const char *name, const category_parameter &par) {
		using namespace std::string_literals;

		compile_command_builder_.add_define("NOARR_TUNING_PARAMETER_VALUE_"s + name, "$"s + name);

		return atf::tuning_parameter(name, atf::interval((std::size_t)0, par.num_ - 1));
	}

	template<class Constraint>
	auto format(const char *name, const category_parameter &par, Constraint &&constraint) {
		using namespace std::string_literals;

		compile_command_builder_.add_define("NOARR_TUNING_PARAMETER_VALUE_"s + name, "$"s + name);

		return atf::tuning_parameter(name, atf::interval((std::size_t)0, par.num_ - 1), std::forward<Constraint>(constraint));
	}

	template<class Start, class End, class Step>
	auto format(const char *name, const range_parameter<Start, End, Step> &par) {
		using namespace std::string_literals;

		compile_command_builder_.add_define("NOARR_TUNING_PARAMETER_VALUE_"s + name, "$"s + name);

		return atf::tuning_parameter(name, atf::interval(par.min_, par.max_, par.step_));
	}

	template<class Start, class End, class Step, class Constraint>
	auto format(const char *name, const range_parameter<Start, End, Step> &par, Constraint &&constraint) {
		using namespace std::string_literals;

		compile_command_builder_.add_define("NOARR_TUNING_PARAMETER_VALUE_"s + name, "$"s + name);

		return atf::tuning_parameter(name, atf::interval(par.min_, par.max_, par.step_), std::forward<Constraint>(constraint));
	}

	auto format(const char *name, const permutation_parameter &par) {
		using namespace std::string_literals;

		throw std::runtime_error("Permutation parameters are not supported by ATF");

		compile_command_builder_.add_define("NOARR_TUNING_PARAMETER_VALUE_"s + name, "$"s + name);

		return atf::tuning_parameter(name, atf::interval((std::size_t)0, par.num_ - 1));
	}

	template<class Constraint>
	auto format(const char *name, const permutation_parameter &par, Constraint &&constraint) {
		using namespace std::string_literals;

		throw std::runtime_error("Permutation parameters are not supported by ATF");

		compile_command_builder_.add_define("NOARR_TUNING_PARAMETER_VALUE_"s + name, "$"s + name);

		return atf::tuning_parameter(name, atf::interval((std::size_t)0, par.num_ - 1), std::forward<Constraint>(constraint));
	}

private:
	abort_condition_t abort_condition_;
	CompileCommandBuilder compile_command_builder_;
	RunCommandBuilder run_command_builder_;
};

} // namespace noarr::tuning

#endif // NOARR_TUNING_ATF_FORMATTER_HPP
