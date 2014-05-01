#include "stopwatch.h"
#include "simlog.h"

#include <iostream>
#include <fstream>

using namespace std;

extern zks::simlog logger;

int test_stopwatch()
{
    zks::StopWatch sw;
    sw.start("haha");
    for (size_t i = 0; i < 500; ++i) {
        cout << zks::u8string(40, "#") << "\n";
    }
    cout << endl;
    sw.tick("pint lots of sharp");
    for (size_t i = 0; i < 500; ++i) {
        cout << zks::u8string(40, "*") << "\n";
    }
    cout << endl;
    sw.tick("pint lots of asteriks");

    ZKS_ERROR(logger, "stopwatch", "%s", sw.u8str().c_str());

    return 0;
}
