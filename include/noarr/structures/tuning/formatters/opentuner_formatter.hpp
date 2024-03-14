#ifndef NOARR_STRUCTURES_TUNING_OPENTUNER_FORMATTER_HPP
#define NOARR_STRUCTURES_TUNING_OPENTUNER_FORMATTER_HPP

#include <exception>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <string>

#include "noarr/structures_extended.hpp"

#include "noarr/structures/tuning/builders.hpp"
#include "noarr/structures/tuning/formatters/common.hpp"
#include "noarr/structures/tuning/macros.hpp"
#include "noarr/structures/tuning/tuning.hpp"

namespace noarr::tuning {

class opentuner_manipulator_formatter {
public:
	opentuner_manipulator_formatter(std::ostream &out, std::size_t indent_level = 0)
		: indent_level_(indent_level)
		, out_(out)
	{}

	void header() const {
		out_ <<
			std::string(indent_level_, ' ') <<
				"def manipulator(self):" << std::endl <<
			std::string(indent_level_ + 2, ' ') <<
				"manipulator = ConfigurationManipulator()" << std::endl;
	}

	void footer() const {
		out_ << std::string(indent_level_ + 2, ' ') << "return manipulator" << std::endl;
	}

	void format(const char *name, const category_parameter &par) const {
		out_ << std::string(indent_level_ + 2, ' ')  << "manipulator.add_parameter("
			<< "SwitchParameter('" << name << "', " << par.num_ << "))" << std::endl;
	}

	void format(const char *name, const permutation_parameter &par) const {
		out_ << std::string(indent_level_ + 2, ' ')  << "manipulator.add_parameter("
			<< "PermutationParameter('" << name << "', range(" << par.num_ << ")))" << std::endl;
	}

	template<class Start, class End, class Step>
	void format(const char *name, const range_parameter<Start, End, Step> &par) const {
		if (par.step_ != 1)
			throw std::runtime_error("OpenTuner does not support step in range parameters");

		out_ << std::string(indent_level_ + 2, ' ')  << "manipulator.add_parameter("
			<< "IntegerParameter('" << name << "', " << par.min_ << ", " << par.max_ << "))" << std::endl;
	}

private:
	std::size_t indent_level_;
	std::ostream &out_;
};

static_assert(IsTunerFormatter<opentuner_manipulator_formatter>);

template<IsCompileCommandBuilder CompileCommandBuilder, IsRunCommandBuilder RunCommandBuilder>
class opentuner_run_formatter {
public:
	opentuner_run_formatter(std::ostream &out, std::shared_ptr<CompileCommandBuilder> compile_command_builder, std::shared_ptr<RunCommandBuilder> run_command_builder, std::string_view measure_command, std::size_t indent_level = 0)
		: out_(out)

		, compile_command_builder_(compile_command_builder)
		, run_command_builder_(run_command_builder)
		, measure_command_(measure_command)

		, indent_level_(indent_level)
	{
		compile_command_builder_->add_define("NOARR_PASS_BY_DEFINE");
	}

	void header() const {
		out_ <<
			std::string(indent_level_, ' ') <<
				"def run(self, desired_result, input, limit):" << std::endl <<
			std::string(indent_level_ + 2, ' ') <<
				"config = desired_result.configuration.data" << std::endl;
	}

	void footer() const {
		out_ <<
			std::string(indent_level_ + 2, ' ') <<
				"compile_result = self.call_program(f'''" << *compile_command_builder_ << "''')" << std::endl <<

			std::string(indent_level_ + 2, ' ') <<
				"if not compile_result['returncode'] == 0:" << std::endl <<
			std::string(indent_level_ + 4, ' ') <<
				"return Result(state='ERROR', time=math.inf)" << std::endl <<

			std::string(indent_level_ + 2, ' ') <<
				"run_cmd = '" << *run_command_builder_ << '\'' << std::endl <<

			std::string(indent_level_ + 2, ' ') <<
				"run_result = self.call_program(run_cmd)" << std::endl <<

			std::string(indent_level_ + 2, ' ') <<
				"if not run_result['returncode'] == 0:" << std::endl <<
			std::string(indent_level_ + 4, ' ') <<
				"return Result(state='ERROR', time=math.inf)" << std::endl <<

			std::string(indent_level_ + 2, ' ') <<
				measure_command_ << std::endl;
	}

	void format(const char *name, const category_parameter &) {
		using std::string_literals::operator""s;

		compile_command_builder_->add_define(
			"NOARR_PARAMETER_VALUE_"s + name,
			"{config[\""s + name + "\"]}");
	}

	void format(const char *name, const permutation_parameter &) {
		using std::string_literals::operator""s;

		compile_command_builder_->add_define(
			"NOARR_PARAMETER_VALUE_"s + name,
			"{str.join(\",\", map(str, config[\""s + name + "\"]))}");
	}

	template<class Start, class End, class Step>
	void format(const char *name, const range_parameter<Start, End, Step> &) {
		using std::string_literals::operator""s;

		compile_command_builder_->add_define(
			"NOARR_PARAMETER_VALUE_"s + name,
			"{config[\""s + name + "\"]}");
	}

private:
	std::ostream &out_;

	std::shared_ptr<CompileCommandBuilder> compile_command_builder_;
	std::shared_ptr<RunCommandBuilder> run_command_builder_;
	std::string measure_command_;

	std::size_t indent_level_;
};


template<IsCompileCommandBuilder CompileCommandBuilder, IsRunCommandBuilder RunCommandBuilder>
class opentuner_formatter : public combined_formatter<lazy_formatter<opentuner_manipulator_formatter>, lazy_formatter<opentuner_run_formatter<CompileCommandBuilder, RunCommandBuilder>>> {
	using base = combined_formatter<lazy_formatter<opentuner_manipulator_formatter>, lazy_formatter<opentuner_run_formatter<CompileCommandBuilder, RunCommandBuilder>>>;

public:
	opentuner_formatter(std::ostream &out, std::shared_ptr<CompileCommandBuilder> compile_command_builder, std::shared_ptr<RunCommandBuilder> run_command_builder, const char *measure_command)
		: combined_formatter<lazy_formatter<opentuner_manipulator_formatter>, lazy_formatter<opentuner_run_formatter<CompileCommandBuilder, RunCommandBuilder>>>{
		std::make_unique<lazy_formatter<opentuner_manipulator_formatter>>(std::make_unique<opentuner_manipulator_formatter>(out, 2)),
		std::make_unique<lazy_formatter<opentuner_run_formatter<CompileCommandBuilder, RunCommandBuilder>>>(std::make_unique<opentuner_run_formatter<CompileCommandBuilder, RunCommandBuilder>>(out, compile_command_builder, run_command_builder, measure_command, 2))},
		out_(out)
	{}

	void header() {
		out_ << R"(#!/usr/bin/env python3
import math

import opentuner
from opentuner import ConfigurationManipulator
from opentuner import IntegerParameter
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

} // namespace noarr::tuning

#endif
