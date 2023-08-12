#ifndef OPENTUNER_TEST_TEST_HPP
#define OPENTUNER_TEST_TEST_HPP

#include <memory>

#include "noarr/structures_extended.hpp"
#include "noarr/structures/extra/metabuilders.hpp"
#include "noarr/structures/extra/metamacros.hpp"
#include "noarr/structures/extra/metastructures.hpp"

// TODO: name

template<noarr::tuning::IsCompileCommandBuilder CompileCommandBuilder, noarr::tuning::IsRunCommandBuilder RunCommandBuilder>
struct optuna_formatter {
	std::string name_;
	std::ostream &out_;

	std::shared_ptr<CompileCommandBuilder> compile_command_builder_;
	std::shared_ptr<RunCommandBuilder> run_command_builder_;
	const char *measure_command_;

	optuna_formatter(std::ostream &out, std::shared_ptr<CompileCommandBuilder> compile_command_builder, std::shared_ptr<RunCommandBuilder> run_command_builder, const char *measure_command)
		: out_(out)

		, compile_command_builder_(compile_command_builder)
		, run_command_builder_(run_command_builder)
		, measure_command_(measure_command)
	{
		compile_command_builder_->add_define("NOARR_PASS_BY_DEFINE");
	}

	void header() {
		out_ << R"(#!/usr/bin/env python3

import optuna
from optuna import trial as trial_module

import itertools
import math
import os
import time

def application_tuner(trial: trial_module.Trial):
)";
	}

	void footer() {
		out_ << "  compile_command = f'''" << *compile_command_builder_ << "'''" << std::endl;
		out_ << "  compiled = " << "os.system(compile_command)" << std::endl;
		out_ << "  if compiled != 0:" << std::endl;
		out_ << "    return math.inf" << std::endl;

		out_ << "  run_command = '''" << *run_command_builder_ << "'''" <<std::endl;
		out_ << "  start = time.time()" << std::endl;
		out_ << "  run = " << "os.system(run_command)" << std::endl;
		out_ << "  end = time.time()" << std::endl;

		out_ << "  if run != 0:" << std::endl;
		out_ << "    return math.inf" << std::endl;

		out_ << "  run_time = end - start" << std::endl;

		out_ << "  " << measure_command_ << std::endl;

		out_ << R"(
study = optuna.create_study()
study.optimize(application_tuner, n_trials=100)
)";
	}

	// TODO
	void format(const noarr::tuning::begin_parameter &) {
	}

	// TODO
	void format(const noarr::tuning::end_parameter &) {
		out_ << std::endl;
	}

	// TODO
	void format(const noarr::tuning::name_parameter &name) {
		name_ = "NOARR_PARAMETER_VALUE_" + std::string(name.name_);

		out_ << "  " << name_ << '=';

		auto value = "{" + name_ + "}";
		compile_command_builder_->add_define(name_.c_str(), value.c_str());
	}

	// TODO
	void format(const noarr::tuning::category_parameter &par) {
		out_ << "  trial.suggest_categorical('" << name_ << "', range(" << par.num_ << "))";
	}

	// TODO
	void format(const noarr::tuning::multiple_choice_parameter &) {
		throw std::exception();
	}

	// TODO
	void format(const noarr::tuning::permutation_parameter &par) {
		out_ << "  trial.suggest_categorical('" << name_ << "', list(map(lambda x: ','.join(x), itertools.permutations(map(str, range(" << par.num_ << "))))))";
	}
};

#endif
