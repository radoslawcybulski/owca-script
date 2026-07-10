#!/bin/sh

(MODE=Release ./build.sh > /dev/null 2> /dev/null) && \
	echo "git: $(git log -n 1 --oneline)" && \
	taskset -c 2 build/owca-script-test --gtest_filter=PerformanceTest.DISABLED_measure_1 --gtest_also_run_disabled_tests && \
	taskset -c 2 build/owca-script-test --gtest_filter=PerformanceTest.DISABLED_measure_2 --gtest_also_run_disabled_tests
