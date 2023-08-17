#ifndef OPENTUNER_TEST_TEST_HPP
#define OPENTUNER_TEST_TEST_HPP

#include <memory>

#include "noarr/structures_extended.hpp"
#include "noarr/structures/extra/metabuilders.hpp"
#include "noarr/structures/extra/metamacros.hpp"
#include "noarr/structures/extra/metastructures.hpp"
#include "noarr/structures/extra/extraformatters.hpp"

// TODO: name
struct opentuner_ss_formatter {
	// TODO: this is bad
	const char *manipulator_name_;
	std::size_t indent_level_;
	std::ostream &out_;

	opentuner_ss_formatter(const char *manipulator_name, std::ostream &out, std::size_t indent_level = 0)
		: manipulator_name_(manipulator_name)
		, indent_level_(indent_level)
		, out_(out)
	{}

	void header() {
		out_ <<
			std::string(indent_level_, ' ') <<
				"def manipulator(self):" << std::endl <<
			std::string(indent_level_ + 2, ' ') <<
				manipulator_name_ << " = ConfigurationManipulator()" << std::endl;

		indent_level_ += 2;
	}

	void footer() {
		out_ << std::string(indent_level_, ' ') << "return " << manipulator_name_ << std::endl;
	}

	// TODO
	void format(const char *name, const noarr::tuning::category_parameter &par) {
		out_ << std::string(indent_level_, ' ')  << manipulator_name_ << ".add_parameter(";
		out_ << "SwitchParameter('" << name << "', " << par.num_ << "))" << std::endl;
	}

	// TODO
	void format(const char *, const noarr::tuning::multiple_choice_parameter &) {
		throw std::exception();
	}

	// TODO
	void format(const char *name, const noarr::tuning::permutation_parameter &par) {
		out_ << std::string(indent_level_, ' ')  << manipulator_name_ << ".add_parameter(";
		out_ << "PermutationParameter('" << name << "', range(" << par.num_ << ")))" << std::endl;
	}
};

static_assert(noarr::tuning::IsTunerFormatter<opentuner_ss_formatter>);

// TODO: name
template<noarr::tuning::IsCompileCommandBuilder CompileCommandBuilder, noarr::tuning::IsRunCommandBuilder RunCommandBuilder>
struct opentuner_run_formatter {
	// TODO: this is bad
	const char *cfg_name_;
	std::ostream &out_;

	std::shared_ptr<CompileCommandBuilder> compile_command_builder_;
	std::shared_ptr<RunCommandBuilder> run_command_builder_;
	const char *measure_command_;

	std::size_t indent_level_;

	opentuner_run_formatter(const char *cfg_name, std::ostream &out, std::shared_ptr<CompileCommandBuilder> compile_command_builder, std::shared_ptr<RunCommandBuilder> run_command_builder, const char *measure_command, std::size_t indent_level = 0)
		: cfg_name_(cfg_name)
		, out_(out)

		, compile_command_builder_(compile_command_builder)
		, run_command_builder_(run_command_builder)
		, measure_command_(measure_command)

		, indent_level_(indent_level)
	{
		compile_command_builder_->add_define("NOARR_PASS_BY_DEFINE");
	}

	void header() {
		out_ <<
			std::string(indent_level_, ' ') <<
				"def run(self, desired_result, input, limit):" << std::endl <<
			std::string(indent_level_ + 2, ' ') <<
				cfg_name_ << " = desired_result.configuration.data" << std::endl;

		indent_level_ += 2;
	}

	void footer() {
		out_ <<
			std::string(indent_level_, ' ') <<
				"compile_result = self.call_program(f'''" << *compile_command_builder_ << "''')" << std::endl <<

			std::string(indent_level_, ' ') <<
				"if not compile_result['returncode'] == 0:" << std::endl <<
			std::string(indent_level_ + 2, ' ') <<
				"return Result(state='ERROR', time=math.inf)" << std::endl <<

			std::string(indent_level_, ' ') <<
				"run_cmd = '" << *run_command_builder_ << '\'' << std::endl <<

			std::string(indent_level_, ' ') <<
				"run_result = self.call_program(run_cmd)" << std::endl <<

			std::string(indent_level_, ' ') <<
				"if not run_result['returncode'] == 0:" << std::endl <<
			std::string(indent_level_ + 2, ' ') <<
				"return Result(state='ERROR', time=math.inf)" << std::endl <<

			std::string(indent_level_, ' ') <<
				measure_command_ << std::endl;
	}

	// TODO
	void format(const char *name, const noarr::tuning::category_parameter &) {
		// TODO: fix this
		compile_command_builder_->add_define(
			(std::string("NOARR_PARAMETER_VALUE_") + name).c_str(),
			(std::string("{")  + cfg_name_ + "[\"" + name + "\"]}").c_str());
	}

	// TODO
	void format(const char *, const noarr::tuning::multiple_choice_parameter &) {
		throw std::exception();
	}

	// TODO
	void format(const char *name, const noarr::tuning::permutation_parameter &) {
		compile_command_builder_->add_define(
			(std::string("NOARR_PARAMETER_VALUE_") + name).c_str(),
			(std::string("{str.join(\",\", map(str, ") + cfg_name_ + "[\"" + name + "\"]))}").c_str());
	}
};


template<noarr::tuning::IsCompileCommandBuilder CompileCommandBuilder, noarr::tuning::IsRunCommandBuilder RunCommandBuilder>
class opentuner_formatter : public noarr::tuning::combined_formatter<noarr::tuning::lazy_formatter<opentuner_ss_formatter>, noarr::tuning::lazy_formatter<opentuner_run_formatter<CompileCommandBuilder, RunCommandBuilder>>> {
	using base = noarr::tuning::combined_formatter<noarr::tuning::lazy_formatter<opentuner_ss_formatter>, noarr::tuning::lazy_formatter<opentuner_run_formatter<CompileCommandBuilder, RunCommandBuilder>>>;

public:
	opentuner_formatter(std::ostream &out, std::shared_ptr<CompileCommandBuilder> compile_command_builder, std::shared_ptr<RunCommandBuilder> run_command_builder, const char *measure_command)
		: noarr::tuning::combined_formatter<noarr::tuning::lazy_formatter<opentuner_ss_formatter>, noarr::tuning::lazy_formatter<opentuner_run_formatter<CompileCommandBuilder, RunCommandBuilder>>>{
		std::make_unique<noarr::tuning::lazy_formatter<opentuner_ss_formatter>>(std::make_unique<opentuner_ss_formatter>("manipulator", out, 2)),
		std::make_unique<noarr::tuning::lazy_formatter<opentuner_run_formatter<CompileCommandBuilder, RunCommandBuilder>>>(std::make_unique<opentuner_run_formatter<CompileCommandBuilder, RunCommandBuilder>>("ctx", out, compile_command_builder, run_command_builder, measure_command, 2))},
		out_(out)
	{}

	void header() {
		out_ << R"(#!/usr/bin/env python3
import math

import opentuner
from opentuner import ConfigurationManipulator
from opentuner import IntegerParameter
from opentuner import EnumParameter
from opentuner import MeasurementInterface
from opentuner import Result
from opentuner import SwitchParameter
from opentuner import PermutationParameter

class ApplicationTuner(MeasurementInterface):
  def save_final_config(self, configuration):
    self.manipulator().save_to_file(configuration.data, 'mmm_final_config.json')
)";
		base::header();
	}

	void footer() {
		base::footer();
		out_ << R"(if __name__ == '__main__':
  argparser = opentuner.default_argparser()
  ApplicationTuner.main(argparser.parse_args())
)";
	}

private:
	std::ostream &out_;
};

#endif
