#include "libzks.h"

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

extern zks::simlog logger;

int test_parse() {
	ofstream ofs(R"(C:\log.txt)");
	ofs << zks::u8string(20, '=') << __func__ << endl;
	zks::simconf conf("=", ",", ";");
	conf.parse(R"(C:\github\jimzshi\simconf\src\test.ini)", &ofs);
	vector<zks::u8string> vec;
	{
		ofs << "read sec2-vector: " << conf.option_vec("sec2", "vector", &vec) << endl;
		for (auto u : vec) {
			ofs << "read in:" << u << "\n";
		}
		ofs << endl;
	}
	{
		ofs << "read sec2-vector2: " << conf.option_vec("sec2", "vector2", &vec) << endl;
		for (auto u : vec) {
			ofs << "read in:" << u << "\n";
		}
		ofs << endl;
	}
	return 0;
}