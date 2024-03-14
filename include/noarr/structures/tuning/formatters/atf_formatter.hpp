#ifndef NOARR_STRUCTURES_TUNING_ATF_FORMATTER_HPP
#define NOARR_STRUCTURES_TUNING_ATF_FORMATTER_HPP

#include <cstring>

#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>

#include <atf.hpp>

#include "noarr/structures_extended.hpp"

#include "noarr/structures/tuning/builders.hpp"
#include "noarr/structures/tuning/macros.hpp"
#include "noarr/structures/tuning/tuning.hpp"

#define NOARR_ATF_TP(name) (&*NOARR_PARAMETER_DEFINITION(name))

namespace noarr::tuning {

template<IsCompileCommandBuilder CompileCommandBuilder, IsRunCommandBuilder RunCommandBuilder>
class atf_formatter {
public:
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

	auto format(const char *name, const category_parameter &par) {
		using namespace std::string_literals;

		compile_command_builder_->add_define("NOARR_PARAMETER_VALUE_"s + name, "$"s + name);

		return atf::tuning_parameter(name, atf::interval((std::size_t)0, par.num_ - 1));
	}

	template<class Constraint>
	auto format(const char *name, const category_parameter &par, Constraint &&constraint) {
		using namespace std::string_literals;

		compile_command_builder_->add_define("NOARR_PARAMETER_VALUE_"s + name, "$"s + name);

		return atf::tuning_parameter(name, atf::interval((std::size_t)0, par.num_ - 1), std::forward<Constraint>(constraint));
	}

	template<class Start, class End, class Step>
	auto format(const char *name, const range_parameter<Start, End, Step> &par) {
		using namespace std::string_literals;

		compile_command_builder_->add_define("NOARR_PARAMETER_VALUE_"s + name, "$"s + name);

		return atf::tuning_parameter(name, atf::interval(par.min_, par.max_ - (T)1, par.step_));
	}

	template<class Start, class End, class Step, class Constraint>
	auto format(const char *name, const range_parameter<Start, End, Step> &par, Constraint &&constraint) {
		using namespace std::string_literals;

		compile_command_builder_->add_define("NOARR_PARAMETER_VALUE_"s + name, "$"s + name);

		return atf::tuning_parameter(name, atf::interval(par.min_, par.max_ - (T)1, par.step_), std::forward<Constraint>(constraint));
	}

	auto format(const char *name, const permutation_parameter &par) {
		using namespace std::string_literals;

		compile_command_builder_->add_define("NOARR_PARAMETER_VALUE_"s + name, "$"s + name);

		return atf::tuning_parameter(name, atf::interval((std::size_t)0, par.num_ - 1));
	}

	template<class Constraint>
	auto format(const char *name, const permutation_parameter &par, Constraint &&constraint) {
		using namespace std::string_literals;

		compile_command_builder_->add_define("NOARR_PARAMETER_VALUE_"s + name, "$"s + name);

		return atf::tuning_parameter(name, atf::interval((std::size_t)0, par.num_ - 1), std::forward<Constraint>(constraint));
	}

private:
	std::ostream &out_;

	std::shared_ptr<CompileCommandBuilder> compile_command_builder_;
	std::shared_ptr<RunCommandBuilder> run_command_builder_;
};

} // namespace noarr::tuning

#endif
