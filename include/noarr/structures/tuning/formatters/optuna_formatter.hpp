#ifndef NOARR_STRUCTURES_TUNING_OPTUNA_FORMATTER_HPP
#define NOARR_STRUCTURES_TUNING_OPTUNA_FORMATTER_HPP

#include <ostream>
#include <string_view>
#include <string>

#include "noarr/structures_extended.hpp"

#include "noarr/structures/tuning/builders.hpp"
#include "noarr/structures/tuning/macros.hpp"
#include "noarr/structures/tuning/tuning.hpp"

namespace noarr::tuning {

template<IsCompileCommandBuilder CompileCommandBuilder, IsRunCommandBuilder RunCommandBuilder>
struct optuna_formatter {
	std::ostream &out_;

	CompileCommandBuilder compile_command_builder_;
	RunCommandBuilder run_command_builder_;
	std::string measure_command_;

	optuna_formatter(std::ostream &out, CompileCommandBuilder compile_command_builder, RunCommandBuilder run_command_builder, std::string_view measure_command)
		: out_(out)

		, compile_command_builder_(std::move(compile_command_builder))
		, run_command_builder_(std::move(run_command_builder))
		, measure_command_(measure_command)
	{
		compile_command_builder_.add_define("NOARR_PASS_BY_DEFINE");
	}

	void header() const {
		out_ << R"(#!/usr/bin/env python3

import optuna
from optuna import trial as trial_module

import itertools
import math
import os
import subprocess
import time

def application_tuner(trial: trial_module.Trial):
)";
	}

	void footer() const {
		out_ << "  compile_command = f'''" << compile_command_builder_ << "'''" << std::endl;
		out_ << "  compiled = " << "os.system(compile_command)" << std::endl;
		out_ << "  if compiled != 0:" << std::endl;
		out_ << "    return math.inf" << std::endl;

		out_ << "  run_command = '''" << run_command_builder_ << "'''" <<std::endl;
		out_ << "  start = time.time()" << std::endl;
		out_ << "  run_result = subprocess.run(run_command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)" << std::endl;
		out_ << "  end = time.time()" << std::endl;

		out_ << "  if run_result.returncode != 0:" << std::endl;
		out_ << "    return math.inf" << std::endl;

		out_ << "  run_time = end - start" << std::endl;

		out_ << "  " << measure_command_ << std::endl;

		out_ << R"(
study = optuna.create_study()
study.optimize(application_tuner, n_trials=100)
)";
	}

	void format(std::string_view name, const category_parameter &par) {
		using namespace std::string_literals;

		std::string name_ = "NOARR_PARAMETER_VALUE_"s;
		name_.append(name);

		auto value = "{" + name_ + "}";

		compile_command_builder_.add_define(name_, value);
		out_ << "  " << name_ << " = trial.suggest_categorical('" << name_ << "', range(" << par.num_ << "))" << std::endl;
	}

	void format(std::string_view name, const permutation_parameter &par) {
		using namespace std::string_literals;

		std::string name_ = "NOARR_PARAMETER_VALUE_"s;
		name_.append(name);

		auto value = "{" + name_ + "}";

		compile_command_builder_.add_define(name_, value);
		out_ << "  " << name_ << " = trial.suggest_categorical('" << name_ << "', list(map(lambda x: ','.join(x), itertools.permutations(map(str, range(" << par.num_ << "))))))" << std::endl;
	}

	template<class Start, class End, class Step>
	void format(std::string_view name, const range_parameter<Start, End, Step> &par) {
		using namespace std::string_literals;

		std::string name_ = "NOARR_PARAMETER_VALUE_"s;
		name_.append(name);

		auto value = "{" + name_ + "}";

		compile_command_builder_.add_define(name_, value);
		out_ << "  " << name_ << " = trial.suggest_int('" << name_ << "', " << par.min_ << ", " << par.max_ << ", " << par.step_ << ")" << std::endl;
	}
};

} // namespace noarr::tuning

#endif
