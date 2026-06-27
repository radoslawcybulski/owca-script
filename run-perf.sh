#!/bin/bash

die() { echo "$*" 1>&2 ; exit 1; }

rm -rf build
echo "Building..."
MODE=Release ./build.sh > /dev/null || die "Build failed"

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
	exit 1
}

trap cleanup SIGINT SIGTERM

V1R=""
V2R=""

C=7
for i in $(seq 1 $C)
do
	echo -ne "Running 1 ($i of $C)\r"
	V=$(taskset -c 2 build/owca-script-test --gtest_filter=PerformanceTest.DISABLED_simple_1 --gtest_also_run_disabled_tests | grep "Time taken: " | sed -n 's/.*Time taken: \([0-9.]*\) seconds.*/\1/p')
	V1R="$V1R\\n$V"

	echo -ne "Running 2 ($i of $C)\r"
	V=$(taskset -c 2 build/owca-script-test --gtest_filter=PerformanceTest.DISABLED_simple_2 --gtest_also_run_disabled_tests | grep "Time taken: " | sed -n 's/.*Time taken: \([0-9.]*\) seconds.*/\1/p')
	V2R="$V2R\\n$V"
done

V1=$(echo -e "$V1R" | sort -n | head -n 2 | tail -n 1)
V2=$(echo -e "$V2R" | sort -n | head -n 2 | tail -n 1)
V3=$(echo -e "$V1R" | sort -n | tail -n 1)
V4=$(echo -e "$V2R" | sort -n | tail -n 1)

echo "Performance test results $1"
echo "  Test 1: $V1 seconds (lowest of $C) (highest: $V3)"
echo "  Test 2: $V2 seconds (lowest of $C) (highest: $V4)"

cleanup
