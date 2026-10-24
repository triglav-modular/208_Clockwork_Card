# Always rebuilt: make's one-second timestamps miss an edit made in the same second as the last build.
.PHONY: test
test:
	c++ -std=c++17 -Wall -Wno-unused-variable -Istub -DARDUINO=10819 -o host_test host_test.cpp
	./host_test
