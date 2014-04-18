//#include "test_locale.h"
#include "test.h"

#include <iostream>
#include <clocale>

using namespace std;

zks::simlog logger;

#define _AMD64_

#include <Stringapiset.h>

int main(int argc, char* argv[]) {
	if (argc < 3) {
		cerr << argv[0] << " log.ini log-file" << endl;
		return 0;
	}
	logger.configure(argv[1]);
	logger.config.output.file_name = argv[2];
	logger.reset();

	//test_locale(argc, argv);
	//test_assign();
	//test_access();
	//test_insert();
	//test_format();
	//unicode_output();
	//test_append();
	//test_split();
	//test_join();
	//test_caseit();
	//test_stopwatch(logger);
	//test_simlog();
    //test_permutations();
    //test_mac();
    //test_hash1();
    //test_bit();
	//test_chunk_array();
    std::locale loc = std::locale("");
    ZKS_ERROR(logger, "test", "user's loc: %s", loc.name().c_str());
    std::locale latin1 = std::locale(".1252");
    ZKS_ERROR(logger, "test", "latin1 name: %s", latin1.name().c_str());
    std::wcout.imbue(latin1);
    std::string latin1str{ "\x33\x31\xEA\x35\x36" };
    u8string u8str = zks::decode(".1252", latin1str);
    std::wstring res;
    res.resize(13);
    //int len = ::MultiByteToWideChar(1252, 0, latin1str.data(), -1, &res[0], res.size());

    std::setlocale(LC_ALL, ".1252");
    std::mbstate_t state = std::mbstate_t();
    const char* mbstr = latin1str.data();
    int len = 1 + std::mbsrtowcs(NULL, &mbstr, 0, &state);
    std::vector<wchar_t> wstr(len);
    std::mbsrtowcs(&wstr[0], &mbstr, wstr.size(), &state);
	return 0;
}
