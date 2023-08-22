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

#include "include/auc_bandit.hpp"
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

	std::map<std::string, std::any> parameters_;

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
		atf::tuner tuner;

		auto compile_script = compile_command_builder_->to_string();
		std::cout << compile_script << std::endl;

		auto cost_function = atf::generic::cost_function(run_command_builder_->to_string())
			.compile_script(";" + compile_script);

		tuner.tuning_parameters(*parameters...);
		tuner.search_technique(atf::auc_bandit());
		tuner.tune(cost_function, atf::evaluations(1));
	}

	// TODO
	decltype(auto) format(const char *name, const noarr::tuning::category_parameter &par) {
		using namespace std::string_literals;
		constexpr auto new_tp = [](const char *name, std::size_t end) {
			return atf::tuning_parameter(name, atf::interval((std::size_t)0, end - 1));
		};
		using tp_type = decltype(new_tp(name, par.num_));

		compile_command_builder_->add_define("NOARR_PARAMETER_VALUE_"s + name, "$"s + name);

		return std::any_cast<tp_type&>(parameters_.emplace(name, new_tp(name, par.num_)).first->second);
	}

	template<class Constraint>
	decltype(auto) format(const char *name, const noarr::tuning::category_parameter &par, const Constraint &constraint) {
		using namespace std::string_literals;
		constexpr auto new_tp = [](const char *name, std::size_t end, const Constraint &constraint) {
			return atf::tuning_parameter(name, atf::interval((std::size_t)0, end - 1, constraint));
		};
		using tp_type = decltype(new_tp(name, par.num_, constraint));

		compile_command_builder_->add_define("NOARR_PARAMETER_VALUE_"s + name, "$"s + name);

		return std::any_cast<tp_type&>(parameters_.emplace(name, new_tp(name, par.num_, constraint)).first->second);
	}

	// TODO
	void format(const char *, const noarr::tuning::multiple_choice_parameter &) const {
		throw std::exception();
	}

	void format(const char *, const noarr::tuning::multiple_choice_parameter &, const auto &) const {
		throw std::exception();
	}

	// TODO
	decltype(auto) format(const char *name, const noarr::tuning::permutation_parameter &par) {
		using namespace std::string_literals;
		constexpr auto new_tp = [](const char *name, std::size_t end) {
			return atf::tuning_parameter(name, atf::interval((std::size_t)0, end - 1));
		};
		using tp_type = decltype(new_tp(name, par.num_));

		compile_command_builder_->add_define("NOARR_PARAMETER_VALUE_"s + name, "$"s + name);

		return std::any_cast<tp_type&>(parameters_.emplace(name, new_tp(name, par.num_)).first->second);
	}

	template<class Constraint>
	decltype(auto) format(const char *name, const noarr::tuning::permutation_parameter &par, const Constraint &constraint) {
		using namespace std::string_literals;
		constexpr auto new_tp = [](const char *name, std::size_t end, const Constraint &constraint) {
			return atf::tuning_parameter(name, atf::interval((std::size_t)0, end - 1, constraint));
		};
		using tp_type = decltype(new_tp(name, par.num_, constraint));

		compile_command_builder_->add_define("NOARR_PARAMETER_VALUE_"s + name, "$"s + name);

		return std::any_cast<tp_type&>(parameters_.emplace(name, new_tp(name, par.num_, constraint)).first->second);
	}
};

#endif
