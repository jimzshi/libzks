#include "simlog.h"

#include <iostream>
#include <vector>
#include <thread>
#include <future>

using namespace std;

zks::simlog logger;

long long test() {
	for (int i = 0; i < 100000; ++i) {
		ZKS_ERROR(logger, "profile", "i=%d", i);
	}
	return 0;
}

int main() {
	logger.configure((R"(C:\github\jimzshi\simLog\src\simlog.ini)"));
	logger.reset();
	vector<future<long long>> fv(5);
	for (int i = 0; i < 5; ++i) {
		fv[i] = (std::async(std::launch::async, test));
	}
	for (int i = 0; i < fv.size(); ++i) {
		fv[i].get();
	}
	return 0;
}