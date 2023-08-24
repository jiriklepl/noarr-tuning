#ifndef ATF_TEST_TEST_HPP
#define ATF_TEST_TEST_HPP

#include <cstring>

#include <any>
#include <chrono>
#include <memory>
#include <string>
#include <type_traits>
#include <map>

#include <atf.hpp>

#include "noarr/structures_extended.hpp"
#include "noarr/structures/extra/metabuilders.hpp"
#include "noarr/structures/extra/metamacros.hpp"
#include "noarr/structures/extra/metastructures.hpp"
#include "noarr/structures/extra/extraformatters.hpp"

#define ATF_GET_TP(name) (&*NOARR_PARAMETER_DEFINITION(name))

template<noarr::tuning::IsCompileCommandBuilder CompileCommandBuilder, noarr::tuning::IsRunCommandBuilder RunCommandBuilder>
struct atf_formatter {
	std::ostream &out_;

	std::shared_ptr<CompileCommandBuilder> compile_command_builder_;
	std::shared_ptr<RunCommandBuilder> run_command_builder_;

	atf_formatter(std::ostream &out, std::shared_ptr<CompileCommandBuilder> compile_command_builder, std::shared_ptr<RunCommandBuilder> run_command_builder)
		: out_(out)
		, compile_command_builder_(compile_command_builder)
		, run_command_builder_(run_command_builder)
	{
		compile_command_builder_->add_define("NOARR_PASS_BY_DEFINE");
	}

	constexpr void header() const noexcept {}

	template<class ...Pars>
	void footer(Pars &&...parameters) const noexcept {
		auto compile_script = compile_command_builder_->to_string();
		auto run_script = run_command_builder_->to_string();

		auto cost_function = atf::generic::cost_function(run_script)
			.compile_script(";" + compile_script);

		auto tuner = atf::tuner()
			.tuning_parameters(*parameters...)
			.search_technique(atf::auc_bandit())
			.tune(cost_function, atf::evaluations(10));
	}

	auto format(const char *name, const noarr::tuning::category_parameter &par) {
		using namespace std::string_literals;

		compile_command_builder_->add_define("NOARR_PARAMETER_VALUE_"s + name, "$"s + name);

		return atf::tuning_parameter(name, atf::interval((std::size_t)0, par.num_ - 1));
	}

	template<class Constraint>
	auto format(const char *name, const noarr::tuning::category_parameter &par, Constraint &&constraint) {
		using namespace std::string_literals;

		compile_command_builder_->add_define("NOARR_PARAMETER_VALUE_"s + name, "$"s + name);

		return atf::tuning_parameter(name, atf::interval((std::size_t)0, par.num_ - 1), std::forward<Constraint>(constraint));
	}

	// TODO
	void format(const char *, const noarr::tuning::multiple_choice_parameter &) const {
		throw std::exception();
	}

	void format(const char *, const noarr::tuning::multiple_choice_parameter &, auto &&) const {
		throw std::exception();
	}

	template<class T>
	auto format(const char *name, const noarr::tuning::range_parameter<T> &par) {
		using namespace std::string_literals;

		compile_command_builder_->add_define("NOARR_PARAMETER_VALUE_"s + name, "$"s + name);

		return atf::tuning_parameter(name, atf::interval(par.min_, par.max_ - (T)1, par.step_));
	}

	template<class T, class Constraint>
	auto format(const char *name, const noarr::tuning::range_parameter<T> &par, Constraint &&constraint) {
		using namespace std::string_literals;

		compile_command_builder_->add_define("NOARR_PARAMETER_VALUE_"s + name, "$"s + name);

		return atf::tuning_parameter(name, atf::interval(par.min_, par.max_ - (T)1, par.step_), std::forward<Constraint>(constraint));
	}

	auto format(const char *name, const noarr::tuning::permutation_parameter &par) {
		using namespace std::string_literals;

		compile_command_builder_->add_define("NOARR_PARAMETER_VALUE_"s + name, "$"s + name);

		return atf::tuning_parameter(name, atf::interval((std::size_t)0, par.num_ - 1));
	}

	template<class Constraint>
	auto format(const char *name, const noarr::tuning::permutation_parameter &par, Constraint &&constraint) {
		using namespace std::string_literals;

		compile_command_builder_->add_define("NOARR_PARAMETER_VALUE_"s + name, "$"s + name);

		return atf::tuning_parameter(name, atf::interval((std::size_t)0, par.num_ - 1), std::forward<Constraint>(constraint));
	}
};

#endif
