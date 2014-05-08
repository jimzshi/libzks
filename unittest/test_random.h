#include "libzks.h"
#include "random.h"
#include "u8string.h"

#include <vector>
#include <string>

extern zks::simlog logger;

void test_rand()
{
    zks::randomize();

    std::vector<int> v1(10);
    zks::sample(v1.begin(), v1.end(), zks::RangedRNGen<int>(13, 26));
    ZKS_INFO(logger, "test_rand", "v1: %s", to_string(v1).c_str());

    std::vector<double> v2(15);
    zks::sample(v2.begin(), v2.end(), zks::RangedRNGen<double>(11.3, 100.50));
    ZKS_INFO(logger, "test_rand", "v2: %s", to_string(v2).c_str());

    std::string str1;
    str1.resize(20);
    zks::sample(str1.begin(), str1.end(), zks::rand_char<>);
    ZKS_INFO(logger, "test_rand", "str: %s", str1.c_str());

    std::vector<zks::u8string> v3(128);
    zks::sample(v3.begin(), v3.end(), zks::rand_u8string<>);
    ZKS_INFO(logger, "test_rand", "v1: %s", to_string(v3).c_str());

    return;
}

