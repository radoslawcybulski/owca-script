#!/bin/bash

OLD_BOOST="$(cat /sys/devices/system/cpu/cpu2/cpufreq/boost)"
(echo '0' | sudo tee /sys/devices/system/cpu/cpu2/cpufreq/boost) > /dev/null

OLD_SCALING_GOVERNOR="$(cat /sys/devices/system/cpu/cpu2/cpufreq/scaling_governor)"
(echo userspace | sudo tee /sys/devices/system/cpu/cpu2/cpufreq/scaling_governor) > /dev/null

OLD_SCALING_SPEED="$(cat /sys/devices/system/cpu/cpu2/cpufreq/scaling_setspeed)"
(echo 3401000 | sudo tee /sys/devices/system/cpu/cpu2/cpufreq/scaling_setspeed) > /dev/null

cleanup() {
	(echo "${OLD_BOOST}" | sudo tee /sys/devices/system/cpu/cpu2/cpufreq/boost) > /dev/null
	(echo "${OLD_SCALING_SPEED}" | sudo tee /sys/devices/system/cpu/cpu2/cpufreq/scaling_setspeed) > /dev/null
	(echo "${OLD_SCALING_GOVERNOR}" | sudo tee /sys/devices/system/cpu/cpu2/cpufreq/scaling_governor) > /dev/null
}

trap cleanup SIGINT SIGTERM

taskset -c 2 build/owca-script-test --gtest_filter=PerformanceTest.DISABLED_simple_1 --gtest_also_run_disabled_tests
taskset -c 2 build/owca-script-test --gtest_filter=PerformanceTest.DISABLED_simple_2 --gtest_also_run_disabled_tests

cleanup

