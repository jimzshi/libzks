#include "libzks.h"

#include <iostream>
#include <vector>
#include <thread>
#include <future>

using namespace std;

extern zks::simlog logger;

inline
long long log_thread()
{
    zks::StopWatch sw;
    sw.start("log_thread");
    for (int i = 0; i < 100000; ++i) {
        ZKS_INFO(logger, "test_simlog", "i=%d", i);
    }
    sw.tick("100000 log entries");
    ZKS_INFO(logger, "test_simlog", "%s", sw.u8str().c_str());
    return 0;
}

inline
int test_simlog()
{
    vector<future<long long>> fv(5);
    for (int i = 0; i < 5; ++i) {
        fv[i] = (std::async(std::launch::async, log_thread));
    }
    for (size_t i = 0; i < fv.size(); ++i) {
        fv[i].get();
    }

    return 0;
}

//int main() {
//	logger.configure((R"(C:\github\jimzshi\simLog\src\simlog.ini)"));
//	logger.reset();
//	vector<future<long long>> fv(5);
//	for (int i = 0; i < 5; ++i) {
//		fv[i] = (std::async(std::launch::async, test));
//	}
//	for (int i = 0; i < fv.size(); ++i) {
//		fv[i].get();
//	}
//	return 0;
//}
