#ifndef NOARR_TUNING_BUILDERS_HPP
#define NOARR_TUNING_BUILDERS_HPP

#include <ostream>
#include <sstream>
#include <string_view>
#include <filesystem>

namespace noarr::tuning {

template<class T>
concept IsRunCommandBuilder = requires(std::remove_cvref_t<T> builder, const char *long_name, char short_name, const char *value, std::ostream &out) {
	{ builder.add_operand(value) } -> std::same_as<void>;
	{ builder.add_option(long_name, value) } -> std::same_as<void>;
	{ builder.add_option(long_name) } -> std::same_as<void>;
	{ builder.add_option(short_name, value) } -> std::same_as<void>;
	{ builder.add_option(short_name) } -> std::same_as<void>;
	{ builder.print(out) } -> std::same_as<std::ostream &>;
	{ builder.to_string() } -> std::same_as<std::string>;
};

constexpr std::ostream &operator<<(std::ostream &out, const IsRunCommandBuilder auto &builder) {
	return builder.print(out);
}

class direct_run_command_builder {
public:
	direct_run_command_builder(std::string_view command, std::string_view options = "")
		: command_(command), options_(options)
	{}

	void add_operand(std::string_view value) {
		options_ += ' ';
		options_ += value;
	}

	void add_option(std::string_view name, std::string_view value) {
		options_ += " --";
		options_ += name;
		options_ += ' ';
		options_ += value;
	}

	void add_option(char name, std::string_view value) {
		options_ += " -";
		options_ += name;
		options_ += ' ';
		options_ += value;
	}

	void add_option(std::string_view name) {
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

	std::string to_string() const {
		return std::string(command_) + ' ' + options_;
	}

private:
	std::string command_;
	std::string options_;
};

static_assert(IsRunCommandBuilder<direct_run_command_builder>);


template<class T>
concept IsCompileCommandBuilder = requires(std::remove_cvref_t<T> builder, const char *include, const char *define, std::ostream &out) {
	{ builder.add_include(include) } -> std::same_as<void>;
	{ builder.add_define(define) } -> std::same_as<void>;
	{ builder.add_define(define, define) } -> std::same_as<void>;
	{ builder.print(out) } -> std::same_as<std::ostream &>;
	{ builder.to_string() } -> std::same_as<std::string>;
};

constexpr std::ostream &operator<<(std::ostream &out, IsCompileCommandBuilder auto &builder) {
	return builder.print(out);
}

// This relies on the environment's ability to parse long strings and pass them to the shell (also string interpolation)
class direct_compile_command_builder {
public:
	direct_compile_command_builder(std::string_view compiler, std::string_view flags = "")
		: compiler_(compiler), flags_(flags)
	{}

	void add_include(std::string_view include) {
		flags_ += " -I";
		flags_ += include;
	}

	void add_define(std::string_view define) {
		flags_ += " -D";
		flags_ += define;
	}

	void add_define(std::string_view define, std::string_view value) {
		flags_ += " -D";
		flags_ += define;
		flags_ += '=';
		flags_ += value;
	}

	std::ostream &print(std::ostream &out) const {
		return out << compiler_ << ' ' << flags_;
	}

	std::string to_string() const {
		return std::string(compiler_) + ' ' + flags_;
	}

private:
	std::string compiler_;
	std::string flags_;
};

static_assert(IsCompileCommandBuilder<direct_compile_command_builder>);

class cmake_compile_command_builder {
public:
	cmake_compile_command_builder(std::filesystem::path cmake_lists_path, std::filesystem::path build_dir, std::string_view target, std::string_view flags = "", std::string_view quote = "\'")
		: cmake_lists_path_(cmake_lists_path)
		, build_dir_(build_dir)
		, target_(target)
		, flags_(flags)
		, quote_(quote)
	{}

	void add_include(std::string_view include) {
		flags_ += " -I";
		flags_ += include;
	}

	void add_define(std::string_view define) {
		flags_ += " -D";
		flags_ += define;
	}

	void add_define(std::string_view define, std::string_view value) {
		flags_ += " -D";
		flags_ += define;
		flags_ += '=';
		flags_ += value;
	}

	std::ostream &print(std::ostream &out) {
		if (first_) {
			first_ = false;

			out <<
				"cmake -E make_directory " << build_dir_ << "; " <<
				"cmake -S " << cmake_lists_path_ <<
				" -B " << build_dir_ <<
				" -DCMAKE_BUILD_TYPE=Release; ";
		}

		return out <<
			"cmake --build " << build_dir_ <<
			" --target " << target_ <<
			" -- -B CXX_DEFINES=" << quote_ << flags_ << quote_ << " ";
	}

	std::string to_string() {
		std::ostringstream out;
		print(out);
		return out.str();
	}

private:
	std::filesystem::path cmake_lists_path_;
	std::filesystem::path build_dir_;
	std::string target_;
	std::string flags_;
	std::string quote_;
	bool first_ = true;
};

static_assert(IsCompileCommandBuilder<cmake_compile_command_builder>);

} // namespace noarr::tuning

#endif // NOARR_TUNING_BUILDERS_HPP
