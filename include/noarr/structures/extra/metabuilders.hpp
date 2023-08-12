#ifndef NOARR_STRUCTURES_METABUILDERS_HPP
#define NOARR_STRUCTURES_METABUILDERS_HPP

#include <ostream>

namespace noarr::tuning {

template<class T>
concept IsRunCommandBuilder = requires(T builder, const char *long_name, char short_name, const char *value, std::ostream &out) {
	{ builder.add_operand(value) } -> std::same_as<void>;
	{ builder.add_option(long_name, value) } -> std::same_as<void>;
	{ builder.add_option(long_name) } -> std::same_as<void>;
	{ builder.add_option(short_name, value) } -> std::same_as<void>;
	{ builder.add_option(short_name) } -> std::same_as<void>;
	{ builder.print(out) } -> std::same_as<std::ostream &>;
};

constexpr std::ostream &operator<<(std::ostream &out, IsRunCommandBuilder auto &&builder) {
	return builder.print(out);
}

class direct_run_command_builder {
public:
	direct_run_command_builder(const char *command, const char *options = "")
		: command_(command), options_(options)
	{}

	void add_operand(const char *value) {
		options_ += ' ';
		options_ += value;
	}

	void add_option(const char *name, const char *value) {
		options_ += " --";
		options_ += name;
		options_ += ' ';
		options_ += value;
	}

	void add_option(char name, const char *value) {
		options_ += " -";
		options_ += name;
		options_ += ' ';
		options_ += value;
	}

	void add_option(const char *name) {
		options_ += " --";
		options_ += name;
	}

	void add_option(char name) {
		options_ += " -";
		options_ += name;
	}

	std::ostream &print(std::ostream &out) const {
		return out << command_ << ' ' << options_;
	}

private:
	const char *command_;
	std::string options_;
};

static_assert(IsRunCommandBuilder<direct_run_command_builder>);


template<class T>
concept IsCompileCommandBuilder = requires(T builder, const char *include, const char *define, std::ostream &out) {
	{ builder.add_include(include) } -> std::same_as<void>;
	{ builder.add_define(define) } -> std::same_as<void>;
	{ builder.add_define(define, define) } -> std::same_as<void>;
	{ builder.print(out) } -> std::same_as<std::ostream &>;
};

constexpr std::ostream &operator<<(std::ostream &out, IsCompileCommandBuilder auto &&builder) {
	return builder.print(out);
}

// This relies on the environment's ability to parse long strings and pass them to the shell (also string interpolation)
class direct_compile_command_builder {
public:
	direct_compile_command_builder(const char *compiler, const char *flags = "")
		: compiler_(compiler), flags_(flags)
	{}

	void add_include(const char *include) {
		flags_ += " -I";
		flags_ += include;
	}

	void add_define(const char *define) {
		flags_ += " -D";
		flags_ += define;
	}

	void add_define(const char *define, const char *value) {
		flags_ += " -D";
		flags_ += define;
		flags_ += '=';
		flags_ += value;
	}

	std::ostream &print(std::ostream &out) const {
		return out << compiler_ << ' ' << flags_;
	}

private:
	const char *compiler_;
	std::string flags_;
};

static_assert(IsCompileCommandBuilder<direct_compile_command_builder>);

class cmake_compile_command_builder {
public:
	cmake_compile_command_builder(const char *cmake_file, const char *build_dir, const char *target, const char *flags = "")
		: cmake_file_(cmake_file)
		, build_dir_(build_dir)
		, target_(target)
		, flags_(flags)
	{}

	void add_include(const char *include) {
		flags_ += " -I";
		flags_ += include;
	}

	void add_define(const char *define) {
		flags_ += " -D";
		flags_ += define;
	}
	
	void add_define(const char *define, const char *value) {
		flags_ += " -D";
		flags_ += define;
		flags_ += '=';
		flags_ += value;
	}

	std::ostream &print(std::ostream &out) const {
		return out <<
			"cmake -E make_directory " << build_dir_ <<
			" && cd " << build_dir_ <<
			" && cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS='" << flags_ << "' " << cmake_file_ <<
			" && cmake --build . --target " << target_;
	}

private:
	const char *cmake_file_;
	const char *build_dir_;
	const char *target_;
	std::string flags_;
};

static_assert(IsCompileCommandBuilder<cmake_compile_command_builder>);

} // namespace noarr::tuning

#endif // NOARR_STRUCTURES_METABUILDERS_HPP
