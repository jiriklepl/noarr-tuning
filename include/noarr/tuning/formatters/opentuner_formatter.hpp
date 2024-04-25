#ifndef NOARR_TUNING_OPENTUNER_FORMATTER_HPP
#define NOARR_TUNING_OPENTUNER_FORMATTER_HPP

#include <ostream>
#include <stdexcept>
#include <string_view>
#include <string>

#include "../builders.hpp"
#include "../formatter.hpp"
#include "../macros.hpp"

#include "common.hpp"

namespace noarr::tuning {

class opentuner_manipulator_formatter {
	static constexpr std::string indent(std::size_t level) {
		return std::string(level, ' ');
	}
public:
	opentuner_manipulator_formatter(std::ostream &out, std::size_t indent_level = 0)
		: indent_level_(indent_level)
		, out_(out)
	{}

	void header() const {
		out_ <<
			indent(indent_level_) << "def manipulator(self):\n" <<
			indent(indent_level_ + 2) << "manipulator = ConfigurationManipulator()\n";
	}

	void footer() const {
		out_ << indent(indent_level_ + 2) << "return manipulator\n";
	}

	void format(const char *name, const category_parameter &par) const {
		out_ << indent(indent_level_ + 2)  << "manipulator.add_parameter("
			<< "SwitchParameter('" << name << "', " << par.num_ << "))\n";
	}

	void format(const char *name, const permutation_parameter &par) const {
		out_ << indent(indent_level_ + 2)  << "manipulator.add_parameter("
			<< "PermutationParameter('" << name << "', range(" << par.num_ << ")))\n";
	}

	template<class Start, class End, class Step>
	void format(const char *name, const range_parameter<Start, End, Step> &par) const {
		if (par.step_ != 1)
			throw std::runtime_error("OpenTuner does not support step in range parameters");

		out_ << indent(indent_level_ + 2)  << "manipulator.add_parameter("
			<< "IntegerParameter('" << name << "', " << par.min_ << ", " << par.max_ << "))\n";
	}

private:
	std::size_t indent_level_;
	std::ostream &out_;
};

static_assert(IsTunerFormatter<opentuner_manipulator_formatter>);

template<IsCompileCommandBuilder CompileCommandBuilder, IsRunCommandBuilder RunCommandBuilder>
class opentuner_run_formatter {
	static constexpr std::string indent(std::size_t level) {
		return std::string(level, ' ');
	}
public:
	opentuner_run_formatter(std::ostream &out, CompileCommandBuilder compile_command_builder, RunCommandBuilder run_command_builder, std::string_view measure_command, std::size_t indent_level = 0)
		: out_(out)
		, compile_command_builder_(std::move(compile_command_builder))
		, run_command_builder_(std::move(run_command_builder))
		, measure_command_(measure_command)

		, indent_level_(indent_level)
	{
		compile_command_builder_.add_define("NOARR_PASS_BY_DEFINE");
	}

	void header() const {
		out_ <<
			indent(indent_level_) << "def run(self, desired_result, input, limit):\n" <<
			indent(indent_level_ + 2) << "config = desired_result.configuration.data\n";
	}

	void footer() {
		out_ <<
			indent(indent_level_ + 2) << "compile_result = self.call_program(f'''" << compile_command_builder_ << "''')\n" <<

#if defined(NOARR_TUNING_VERBOSE) && NOARR_TUNING_VERBOSE >= 1
			indent(indent_level_ + 2) << "log.info(f'''Compile time: {compile_result['time']}''')\n" <<
			indent(indent_level_ + 2) << "log.info(f'''Compile returncode: {compile_result['returncode']}''')\n" <<
#endif

			indent(indent_level_ + 2) << "if not compile_result['returncode'] == 0:\n" <<
#if defined(NOARR_TUNING_VERBOSE) && NOARR_TUNING_VERBOSE >= 2
			indent(indent_level_ + 4) << "log.info(f'''Compile stderr: {compile_result['stderr']}''')\n" <<
#endif
			indent(indent_level_ + 4) << "return Result(state='ERROR', time=math.inf)\n" <<

			indent(indent_level_ + 2) << "run_cmd = '" << run_command_builder_ << '\'' << "\n" <<

			indent(indent_level_ + 2) << "run_result = self.call_program(run_cmd)\n" <<

#if defined(NOARR_TUNING_VERBOSE) && NOARR_TUNING_VERBOSE >= 1
			indent(indent_level_ + 2) << "log.info(f'''Run time: {run_result['time']}''')\n" <<
			indent(indent_level_ + 2) << "log.info(f'''Run returncode: {run_result['returncode']}''')\n" <<
#endif

			indent(indent_level_ + 2) << "if not run_result['returncode'] == 0:\n" <<
			indent(indent_level_ + 4) << "return Result(state='ERROR', time=math.inf)\n" <<

			indent(indent_level_ + 2) << measure_command_ << "\n";
	}

	void format(const char *name, const category_parameter &) {
		using std::string_literals::operator""s;

		compile_command_builder_.add_define(
			"NOARR_TUNING_PARAMETER_VALUE_"s + name,
			"{config[\""s + name + "\"]}");
	}

	void format(const char *name, const permutation_parameter &) {
		using std::string_literals::operator""s;

		compile_command_builder_.add_define(
			"NOARR_TUNING_PARAMETER_VALUE_"s + name,
			"{str.join(\",\", map(str, config[\""s + name + "\"]))}");
	}

	template<class Start, class End, class Step>
	void format(const char *name, const range_parameter<Start, End, Step> &) {
		using std::string_literals::operator""s;

		compile_command_builder_.add_define(
			"NOARR_TUNING_PARAMETER_VALUE_"s + name,
			"{config[\""s + name + "\"]}");
	}

private:
	std::ostream &out_;

	CompileCommandBuilder compile_command_builder_;
	RunCommandBuilder run_command_builder_;
	std::string measure_command_;

	std::size_t indent_level_;
};


template<IsCompileCommandBuilder CompileCommandBuilder, IsRunCommandBuilder RunCommandBuilder>
class opentuner_formatter : public combined_formatter<lazy_formatter<opentuner_manipulator_formatter>, lazy_formatter<opentuner_run_formatter<CompileCommandBuilder, RunCommandBuilder>>> {
	using base = combined_formatter<lazy_formatter<opentuner_manipulator_formatter>, lazy_formatter<opentuner_run_formatter<CompileCommandBuilder, RunCommandBuilder>>>;

public:
	opentuner_formatter(std::ostream &out, CompileCommandBuilder compile_command_builder, RunCommandBuilder run_command_builder, const char *measure_command)
		: combined_formatter<lazy_formatter<opentuner_manipulator_formatter>, lazy_formatter<opentuner_run_formatter<CompileCommandBuilder, RunCommandBuilder>>>{
		std::make_unique<lazy_formatter<opentuner_manipulator_formatter>>(std::make_unique<opentuner_manipulator_formatter>(out, 2)),
		std::make_unique<lazy_formatter<opentuner_run_formatter<CompileCommandBuilder, RunCommandBuilder>>>(std::make_unique<opentuner_run_formatter<CompileCommandBuilder, RunCommandBuilder>>(out, std::move(compile_command_builder), std::move(run_command_builder), measure_command, 2))},
		out_(out)
	{}

	void header() {
		out_ << R"(#!/usr/bin/env python3
import math
import sys
import logging

log = logging.getLogger(__name__)

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

#endif // NOARR_TUNING_OPENTUNER_FORMATTER_HPP
