/*
 * test_distributor.h
 *
 *  Created on: Jul 6, 2014
 *      Author: zks
 */

#ifndef TEST_DISTRIBUTOR_H_
#define TEST_DISTRIBUTOR_H_

#include "distributor.h"
#include "random.h"
#include "simlog.h"

#include <vector>
#include <thread>

extern zks::simlog logger;

inline
void sleep_worker(const int& t) {
    ZKS_INFO(logger, "distributor", "sleep for %d seconds.", t);
    std::this_thread::sleep_for(std::chrono::seconds(t));
    return;
}

inline
int sleep_worker2(const int& t) {
    int ret = 2 * t;
    ZKS_INFO(logger, "distributor", "sleep for %d seconds. ret: %d", t, ret);
    std::this_thread::sleep_for(std::chrono::seconds(t));
    return ret;
}

inline
void test_foreach() {
    const int count = 10;
    std::vector<int> tv(count);
    zks::sample(tv.begin(), tv.end(), zks::RangedRNGen<int>(1,5));
    zks::for_each(tv.begin(), tv.end(), sleep_worker, 2);
    return;
}

inline
void test_foreach2() {
    const int count = 10;
    std::vector<int> tv(count), rv;
    zks::sample(tv.begin(), tv.end(), zks::RangedRNGen<int>(1,10));
    rv = zks::for_each(tv.begin(), tv.begin() + 4, sleep_worker2, 2);
    for(auto i : rv) {
        ZKS_INFO(logger, "test_foreach2", "rv: %d", i);
    }
    return;
}


inline
void test_dist() {
    const int count = 10;
    std::vector<int> tv(count);
    zks::sample(tv.begin(), tv.end(), zks::RangedRNGen<int>(1,5));
    zks::distribute(sleep_worker, tv, 2);
    return;
}

inline
void test_dist2() {
    const int count = 10;
    std::vector<int> tv(count), rv;
    zks::sample(tv.begin(), tv.end(), zks::RangedRNGen<int>(1,10));
    zks::distribute(sleep_worker2, tv, &rv, 2);
    for(auto i : rv) {
        ZKS_INFO(logger, "test_dist2", "rv: %d", i);
    }
    return;
}

#endif /* TEST_DISTRIBUTOR_H_ */
