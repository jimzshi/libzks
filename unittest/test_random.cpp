/*
 * test_random.cpp
 *
 *  Created on: Jul 6, 2014
 *      Author: zks
 */
#include "test_random.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <numeric>

using namespace std;

int main(int argc, char* argv[]) {
	if (!test_init(argc, argv)) {
		return -1;
	}

    //test_rand();
	int  pool_size(-1), pick_balls(-1), pick_rounds(-1);
	g_settings.option_num("lotto", "pool_size", &pool_size);
	g_settings.option_num("lotto", "pick_balls", &pick_balls);
	g_settings.option_num("lotto", "pick_rounds", &pick_rounds);
	collect_balls(pool_size, pick_balls, pick_rounds);

    return 0;
}

void test_rand()
{
	std::vector<int> v1(10);
	zks::sample(v1.begin(), v1.end(), zks::RangedRNGen<int>(13, 26));
	ZKS_INFO(g_logger, "test_rand", "v1: %s", to_string(v1).c_str());

	std::vector<double> v2(15);
	zks::sample(v2.begin(), v2.end(), zks::RangedRNGen<double>(11.3, 100.50));
	ZKS_INFO(g_logger, "test_rand", "v2: %s", to_string(v2).c_str());

	std::string str1;
	str1.resize(20);
	zks::sample(str1.begin(), str1.end(), zks::rand_char<>);
	ZKS_INFO(g_logger, "test_rand", "str: %s", str1.c_str());

	std::vector<zks::u8string> v3(128);
	zks::sample(v3.begin(), v3.end(), zks::rand_u8string<>);
	ZKS_INFO(g_logger, "test_rand", "v1: %s", to_string(v3).c_str());

	return;
}

int collect_balls(size_t pool_size, size_t n, size_t rounds) {
	if (n > pool_size) {
		return -1;
	}

	std::ostringstream oss;

	for (size_t t = 0; t < rounds; ++t) {
		std::vector<int> balls;
		balls.resize(pool_size);
		std::iota(balls.begin(), balls.end(), 1);
		//std::shuffle(balls.begin(), balls.end(), zks::global_rng<>());
		oss.str("");
		for (auto b : balls) {
			oss << "(" << b << "), ";
		}
		ZKS_DEBUG(g_logger, "Balls", "%s", oss.str().c_str());
		std::vector<int> res;
		size_t psz(pool_size), left(n);
		for (; left > 0; --left, --psz) {
			int b = g_test_rng() % psz;
			res.push_back(balls[b]);
			balls.erase(balls.begin() + b);
		}

		std::sort(res.begin(), res.end());

		oss.str("");
		for (auto r : res) {
			oss << r << ", ";
		}
		ZKS_NOTICE(g_logger, "Collection", "%s", oss.str().c_str());
	}

	return 0;
}
