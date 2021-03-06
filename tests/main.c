#include "tests.h"

#include <stdio.h>
#include <time.h>

// Typedef that all test function signatures must follow
typedef struct {
	const char *name;
	void (*func)(void);
} Test;

#define TEST(name) { #name, Test_ ## name }

#ifdef CLOCK_REALTIME
// Starting and ending times of tests
struct timespec start, end;
#endif

// Constant environment variables accessible by tests
JDXDataset *example_dataset = NULL;

// Variable set by tests to indicate if they passed, failed, or not executed (declared in header)
TestState final_state;

static void print_duration(void) {
#ifdef CLOCK_REALTIME
	long duration = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;

	printf(" | ");

	if (duration < 1000) {
		printf("%ldμs\n", duration);
	} else if (duration < 1000000) {
		printf("%ldms\n", duration / 1000);
	} else {
		printf("%fs\n", (double) duration / 1000000.0);
	}
#else
	printf("\n");
#endif
}

static void print_pass(void) {
	printf("\x1b[32mpassed\x1b[0m");
	print_duration();
}

static void print_fail(void) {
	printf("\x1b[31mfailed\x1b[0m");
	print_duration();
}

static void print_na(void) {
	printf("\x1b[34mN/A\x1b[0m\n");
}

static void init_testing_env(void) {
#ifndef CLOCK_REALTIME
	printf("\x1b[33m[\x1b[1mWARNING\x1b[0;33m]\x1b[0m CLOCK_REALTIME is not defined in 'time.h'. Timing of tests is disabled.\n\n");
#endif

	example_dataset = JDX_AllocDataset();
	JDX_ReadDatasetFromPath(example_dataset, "./res/example.jdx");
}

static void destroy_testing_env(void) {
	JDX_FreeDataset(example_dataset);
}

int main(void) {
	Test tests[] = {
		TEST(CompareVersions),
		TEST(ReadHeaderFromPath),
		TEST(CopyHeader),
		TEST(ReadDatasetFromPath),
		TEST(WriteDatasetToPath),
		TEST(CopyDataset),
		TEST(AppendDataset)
	};

	init_testing_env();

	int test_count = sizeof(tests) / sizeof(Test);
	int pass_count = 0;
	int na_count = 0;

	// Run each test and print according to whether it passed or failed
	for (int t = 0; t < test_count; t++) {
		final_state = STATE_FAILURE;

		printf("\x1b[33m[\x1b[1m%s\x1b[0;33m]\x1b[0m ", tests[t].name);

#ifdef CLOCK_REALTIME
		clock_gettime(CLOCK_REALTIME, &start);
#endif

		tests[t].func();

#ifdef CLOCK_REALTIME
		clock_gettime(CLOCK_REALTIME, &end);
#endif

		if (final_state == STATE_SUCCESS) {
			print_pass();
			pass_count++;
		} else if (final_state == STATE_FAILURE) {
			print_fail();
		} else if (final_state == STATE_NOEXECUTE) {
			print_na();
			na_count++;
		}
	}

	// Corresponds to green ASCII color code if failed 0, red otherwise
	int fail_color_code = 31 + (pass_count + na_count == test_count ? 1 : 0);

	printf("\nPassed \x1b[32m%d\x1b[0m tests.\n", pass_count);
	printf("Failed \x1b[%dm%d\x1b[0m tests.\n", fail_color_code, test_count - pass_count - na_count);

	if (na_count > 0)
		printf("Did not execute \x1b[34m%d\x1b[0m tests.\n", na_count);

	destroy_testing_env();
}
