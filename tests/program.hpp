#include <chrono>
#include <iostream>
#include <sstream>
#include <utility>
#include <type_traits>

#include <noarr/structures_extended.hpp>
#include <noarr/structures/extra/traverser.hpp>
#include <noarr/structures/interop/bag.hpp>

#include <noarr/tuning/macros.hpp>
#include <noarr/tuning/tuning.hpp>

#ifndef SPECIFIC_TUNING_BEGIN

#include <noarr/tuning/formatters/dummy_formatter.hpp>

#define SPECIFIC_TUNING_BEGIN(...) NOARR_TUNE_BEGIN(noarr::tuning::dummy_formatter())

#endif

#ifndef SPECIFIC_GET_PAR

#define SPECIFIC_GET_PAR(name) (name)

#endif

#ifndef SPECIFIC_TUNING_END

#define SPECIFIC_TUNING_END(...) NOARR_TUNE_END()

#endif

using num_t = float;

namespace {

constexpr auto i_st = noarr::vector<'i'>();
constexpr auto j_st = noarr::vector<'j'>();
constexpr auto k_st = noarr::vector<'k'>();

struct tuning {
	SPECIFIC_TUNING_BEGIN();

	NOARR_TUNE_PAR(block_size, noarr::tuning::mapped_range, [](auto i) { return 1 << i; }, 1, 6);

	NOARR_TUNE_PAR(a_order, noarr::tuning::choice, i_st ^ k_st, k_st ^ i_st);
	NOARR_TUNE_PAR(b_order, noarr::tuning::choice, k_st ^ j_st, j_st ^ k_st);
	NOARR_TUNE_PAR(c_order, noarr::tuning::choice, i_st ^ j_st, j_st ^ i_st);

	NOARR_TUNE_PAR(block_i, noarr::tuning::choice,
		noarr::into_blocks<'i', 'I', 'i'>(*block_size),
		noarr::bcast<'I'>(noarr::lit<1>));

	NOARR_TUNE_PAR(block_j, noarr::tuning::choice,
		noarr::into_blocks<'j', 'J', 'j'>(*block_size),
		noarr::bcast<'J'>(noarr::lit<1>));

	NOARR_TUNE_PAR(block_k, noarr::tuning::choice,
		noarr::into_blocks<'k', 'K', 'k'>(*block_size),
		noarr::bcast<'K'>(noarr::lit<1>));

	NOARR_TUNE_PAR(block_order, noarr::tuning::mapped_permutation, [](auto ...pars) requires (sizeof...(pars) == 5) { return (... ^ pars); },
		noarr::hoist<'I'>(),
		noarr::hoist<'J'>(),
		noarr::hoist<'K'>(),
		noarr::hoist<'i'>(),
		noarr::hoist<'j'>());

	SPECIFIC_TUNING_END(
		SPECIFIC_GET_PAR(block_size),
		SPECIFIC_GET_PAR(a_order),
		SPECIFIC_GET_PAR(b_order),
		SPECIFIC_GET_PAR(c_order),
		SPECIFIC_GET_PAR(block_i),
		SPECIFIC_GET_PAR(block_j),
		SPECIFIC_GET_PAR(block_k),
		SPECIFIC_GET_PAR(block_order)
	);
} tuning;

constexpr auto reset(auto C) {
	return [=](auto state) {
		C[state] = 0;
	};
}

constexpr auto matmul(auto A, auto B, auto C) {
	return [=](auto trav) {
		num_t result = C[trav.state()];

		trav.for_each([=, &result](auto state) {
			result += A[state] * B[state];
		});

		C[trav.state()] = result;
	};
}

void run_matmul(auto ta, auto tb, auto tc, num_t *pa, num_t *pb, num_t *pc) {
	auto A = noarr::make_bag(ta, pa);
	auto B = noarr::make_bag(tb, pb);
	auto C = noarr::make_bag(tc, pc);

	noarr::traverser(C).for_each(reset(C));

	auto trav = noarr::traverser(A, B, C).order(*tuning.block_i ^ *tuning.block_j ^ *tuning.block_k ^ *tuning.block_order);

	trav.template for_sections<'I', 'J', 'K', 'i', 'j'>(matmul(A, B, C));
}

}

int main(int argc, char **argv) {
	if(argc != 3) {
		std::cerr << "Usage: PROGRAM FILE SIZE" << std::endl;
		std::exit(1);
	}

	std::size_t size;
	std::istringstream(argv[2]) >> size;

	auto ta = noarr::scalar<num_t>() ^ *tuning.a_order ^ noarr::set_length<'i', 'k'>(size, size);
	auto tb = noarr::scalar<num_t>() ^ *tuning.b_order ^ noarr::set_length<'k', 'j'>(size, size);
	auto tc = noarr::scalar<num_t>() ^ *tuning.c_order ^ noarr::set_length<'i', 'j'>(size, size);

	std::size_t a_sz = ta | noarr::get_size();
	std::size_t b_sz = tb | noarr::get_size();
	std::size_t c_sz = tc | noarr::get_size();

	num_t *data;

	if (!(data = (num_t *)malloc(a_sz + b_sz + c_sz))) {
		std::cerr << __FILE__ ":" << __LINE__ << ": error: failed to allocate memory" << std::endl;
		exit(1);
	}

	std::FILE *file = std::fopen(argv[1], "r");
	if(std::fread(data, 1, a_sz + b_sz, file) != a_sz + b_sz) {
		std::cerr << "Input error" << std::endl;
		std::abort();
	}
	std::fclose(file);

	run_matmul(ta, tb, tc, data, (data + a_sz / sizeof(num_t)), (data + (a_sz + b_sz) / sizeof(num_t)));
	run_matmul(ta, tb, tc, data, (data + a_sz / sizeof(num_t)), (data + (a_sz + b_sz) / sizeof(num_t)));
	run_matmul(ta, tb, tc, data, (data + a_sz / sizeof(num_t)), (data + (a_sz + b_sz) / sizeof(num_t)));

	auto start = std::chrono::high_resolution_clock::now();
	run_matmul(ta, tb, tc, data, (data + a_sz / sizeof(num_t)), (data + (a_sz + b_sz) / sizeof(num_t)));
	auto end = std::chrono::high_resolution_clock::now();

	auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
	std::cerr << "Total time: " << duration.count() << std::endl;

	std::fwrite(data + (a_sz + b_sz) / sizeof(num_t), 1, c_sz, stdout);

	free(data);

	return 0;
}
