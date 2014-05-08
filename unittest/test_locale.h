#include "libzks.h"

#include <string>
#include <fstream>
#include <iostream>
#include <locale>
#include <codecvt>
#include <vector>
#include <iomanip>

using namespace std;
using namespace zks;

extern zks::simlog logger;

void prepare_file()
{
    // UTF-16le data (if host system is little-endian)
    char16_t utf16le[4] = { 0x007a,          // latin small letter 'z' U+007a
            0x6c34,          // CJK ideograph "water"  U+6c34
            0xd834, 0xdd0b }; // musical sign segno U+1d10b
    // store in a file
    std::ofstream fout("text.txt");
    fout.write(reinterpret_cast<char*>(utf16le), sizeof utf16le);
}

int test_locale(int argc, char* argv[])
{
    //prepare_file();
    //// open as a byte stream
    //std::wifstream fin("text.txt", std::ios::binary);
    //// apply facet
    //fin.imbue(std::locale(fin.getloc(),
    //	new std::codecvt_utf16<wchar_t, 0x10ffff, std::little_endian>));

    //for (wchar_t c; fin.get(c);)
    //	std::clog << std::showbase << std::hex << c << '\n';
    size_t len { 0 };
    bool v { false };

    vector<string> utf8strs = { { (char) 0xf0, (char) 0xa4, (char) 0xad, (char) 0xa2 }, { (char) 0xf0, (char) 0xe4, (char) 0xad, (char) 0xa2 }, { (char) 0xf0,
            (char) 0xa4, (char) 0xed, (char) 0xa2 }, { (char) 0xf0, (char) 0xa4, (char) 0xad, (char) 0xe2 } };
    for (auto& u8s : utf8strs) {
        v = unicode::validate_u8char(u8s.c_str(), &len);
        std::clog << "utf8: " << u8s << "\nvalide: " << v;
        if (v) {
            std::clog << "\nutf8 char has " << len << " bytes." << endl;
        }
        else {
            std::clog << "\nutf8 is invalid, the " << len << "(th) byte in string is invalid" << endl;
        }
    }

    for (auto& u8s : utf8strs) {
        v = unicode::validate(u8s.c_str(), &len);
        std::clog << "utf8: " << u8s << "\nvalide: " << v;
        if (v) {
            std::clog << "\nutf8 char has " << len << " bytes." << endl;
        }
        else {
            std::clog << "\nutf8 is invalid, the " << len << "(th) byte in string is invalid" << endl;
        }
    }

#ifdef _HAS_CHAR_T_SUPPORT
    u32string utf32 = {0x64, 0x345, 0x4321, 0x19b07, 0x1d3006, 0x2000ff, 0x400003f, 0x0};
    for (auto& u32c : utf32) {
        string u8c = unicode::u32u8_cvt.to_bytes(u32c);
        v = unicode::validate_u8char(u8c.c_str(), &len);
        std::clog << "utf8: " << u8c << "\nvalide: " << v;
        if (v) {
            std::clog << "\nutf8 char has " << len << " bytes." << endl;
        }
        else {
            std::clog << "\nutf8 is invalid, the " << len + 1 << "(th) byte in string is invalid" << endl;
        }
    }
#endif

    return 0;
}

int test_assign()
{
    string u8c { (char) 0xf0, (char) 0xa4, (char) 0xad, (char) 0xa2 };
    u8string s;
    s.assign(10, u8c.c_str());
    std::clog << "new str: " << s << endl;
    u8string s2;
    s2.assign(s.c_str(), 4, 4);
    std::clog << "s2: " << s2 << endl;

    return 0;
}

int test_access()
{
#ifdef _HAS_CHAR_T_SUPPORT
    string u8c {(char)0xf0, (char)0xa4, (char)0xad, (char)0xa2};
    u8string s {(char32_t)0x10481, (char32_t)0x10482, (char32_t)0x1D306};
    //s.assign(10, u8c.c_str());
    std::clog << "new str: " << s << endl;
    u8string s2;
    s2.assign(s.c_str(), 4, 4);
    std::clog << "s2: " << s2 << endl;

    std::clog << s.at(2) << endl;
    s.at(2) = (char)0xae;

    std::clog << "new s:" << s << endl;
#endif
    return 0;
}

int test_insert()
{
#ifdef _HAS_CHAR_T_SUPPORT
    string u8c {(char)0xf0, (char)0xa4, (char)0xad, (char)0xa2};
    u8string s {(char32_t)0x10481, (char32_t)0x10482, (char32_t)0x1D306};
    std::clog << "u8c: " << u8c << "\n" << "u8s: " << s << "\n";
    s.insert(4, 5, u8c.data(), u8c.size());
    std::clog << "new u8s: " << s << std::endl;
#endif
    return 0;
}

int test_format()
{
#ifdef _HAS_CHAR_T_SUPPORT
    string u8c {(char)0xf0, (char)0xa4, (char)0xad, (char)0xa2};
    u8string s {(char32_t)0x10481, (char32_t)0x10482, (char32_t)0x1D306, (char32_t)0x20000};
    std::clog << "u8c: " << u8c << "\n" << "u8s: " << s << "\n";
    std::clog << "size: " << s.size() << "\tcap: " << s.capacity() << endl;
    s.format(100, "change %d \n\t %s", 3, "warning");
    std::clog << "new u8s: " << s << std::endl;
    std::clog << "size: " << s.size() << "\tcap: " << s.capacity() << endl;
    s.append(100, "%d\n\t%s", 3, "warning");
    std::clog << "new u8s: " << s << std::endl;
    std::clog << "size: " << s.size() << "\tcap: " << s.capacity() << endl;
#endif
    return 0;
}

int unicode_output()
{
#ifdef _HAS_CHAR_T_SUPPORT
    u8string s;
    s.reserve(8);
    for (char32_t i = 0x00100; i < 0x2FA1F; ++i) {
        if (unicode::invalid_codepoint(i)) continue;
        s = i;
        std::clog << "U+" << std::setbase(16) << std::setw(5) << std::setfill('0') << i << "\t" << s << "\n";
    }
#endif

    return 0;
}

int test_append()
{
#ifdef _HAS_CHAR_T_SUPPORT
    string u8c {(char)0xf0, (char)0xa4, (char)0xad, (char)0xa2};
    u8string s {(char32_t)0x10481, (char32_t)0x10482, (char32_t)0x1D306, (char32_t)0x20000};
    std::clog << "u8c: " << u8c << "\n" << "u8s: " << s << "\n";
    std::clog << "left char* " + s + " right char* \n";
    std::clog << L"left wchar*" + s + L" right wchar* \n";
    u8string s2 {L"哈哈"};
    std::wstring s3 {L"测试wstring"};
    //std::clog << "chinese: " << u8c + s + s3 << "\n";
    u8string s4 {"hello world!"};
#endif
    return 0;
}

int test_split()
{
    std::vector<u8string> cases { R"(Year,Make,Model,Description,Price)", R"(1997,Ford,E350,"ac, abs, moon",3000.00)",
            R"(1999,Chevy,"Venture ""Extended Edition""","",4900.00)", R"(1999,Chevy,"Venture ""Extended Edition, Very Large""",,5000.00)",
            R"(1996,Jeep,Grand Cherokee,"MUST SELL! - 
air, moon roof, loaded",4799.00)" };
    for (auto s : cases) {
        std::clog << u8string(20, '=') << "\nraw string: R(" << s << ")R\nafter split: \n";
        for (auto i : s.split()) {
            std::clog << "original: R(" << i << ")R, unquoted: R(" << i.unquote() << ")R\n";
        }
    }
    return 0;
}
int test_join()
{
    std::vector<u8string> cases { R"(Year,Make,Model,Description,Price)", R"(1997,Ford,E350,"ac, abs, moon",3000.00)",
            R"(1999,Chevy,"Venture ""Extended Edition""","",4900.00)", R"(1999,Chevy,"Venture ""Extended Edition, Very Large""",,5000.00)",
            R"(1996,Jeep,Grand Cherokee,"MUST SELL! - 
air, moon roof, loaded",4799.00)" };
    for (auto s : cases) {
        std::clog << u8string(20, '=') << "\nraw string: R(" << s << ")R\nafter split: \n";
        std::vector<u8string> items = s.split(false);
        for (auto i : items) {
            std::clog << "original: R(" << i << ")R\n";
        }
        std::clog << "Newly joined string: R(" << u8string { "\t" }.join(items, "`", "\\") << ")R\n";
    }
    return 0;
}

int test_caseit()
{
    std::clog << "current name: " << std::locale("").name() << "\n";
    u8string sv { L"äöåß" };
    std::clog << "sv: " << sv << "\n";
    std::clog << "to upper: " << sv.toupper(std::locale("de-DE")) << "\n";
    std::clog << "to lower: " << sv.tolower(std::locale("de-DE")) << "\n";
    return 0;
}

int test_code()
{
#ifdef ZKS_OS_GNULINUX_
    std::locale loc = std::locale("");
    ZKS_INFO(logger, "test", "user's loc: %s", loc.name().c_str());
    std::locale latin1 = std::locale("en_US.iso88591");
    ZKS_INFO(logger, "test", "latin1 name: %s", latin1.name().c_str());
    std::string latin1str { "\x33\x31\xEA\x35\x36" };
    u8string u8str = zks::unicode::decode("en_US.iso88591", latin1str);
    ZKS_INFO(logger, "test", "decode from latin1: %s", u8str.c_str());
    std::string gb18030str = zks::unicode::encode("zh_CN.gb18030", u8str);
    ZKS_INFO(logger, "test", "encode to GB18030: %s", gb18030str.c_str());
    return 0;
#elif defined(ZKS_OS_WINDOWS_)
    std::locale loc = std::locale("");
    ZKS_INFO(logger, "test", "user's loc: %s", loc.name().c_str());
    std::locale latin1 = std::locale(".1252");
    ZKS_INFO(logger, "test", "latin1 name: %s", latin1.name().c_str());
    std::string latin1str {"\x33\x31\xEA\x35\x36"};
    u8string u8str = zks::unicode::decode(1252, latin1str);
    ZKS_INFO(logger, "test", "decode from latin1: %s", u8str.c_str());
    std::string gb18030str = zks::unicode::encode(54936, u8str);
    ZKS_INFO(logger, "test", "encode to GB18030: %s", gb18030str.c_str());
    return 0;
#endif
}
