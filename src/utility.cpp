#include "configure.h"
#include "utility.h"

namespace zks
{

#ifdef ZKS_OS_WINDOWS_

#include <winsock2.h>
#include <IPHlpApi.h>
#pragma comment(lib, "IPHLPAPI.lib")

    std::vector<zks::u8string> get_mac_address() {
        std::vector<zks::u8string> ret;
        ULONG family = AF_UNSPEC;
        ULONG outBufLen = 0;
        PIP_ADAPTER_ADDRESSES pAddresses = NULL;
        DWORD dwRetVal = 0;
        ULONG flags = 0x000f | GAA_FLAG_SKIP_FRIENDLY_NAME;
        ULONG Iterations = 0;
        do {
            pAddresses = (IP_ADAPTER_ADDRESSES *)std::malloc(outBufLen);
            if (pAddresses == NULL) {
                return ret;
            }
            dwRetVal = GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen);
            if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
                std::free(pAddresses);
                pAddresses = NULL;
            }
            else {
                break;
            }
            Iterations++;
        }while ((dwRetVal == ERROR_BUFFER_OVERFLOW) && (Iterations < 3));

        if (dwRetVal == NO_ERROR) {
            PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses;
            while (pCurrAddresses) {
                if (pCurrAddresses->PhysicalAddressLength != 0) {
                    ret.emplace_back();
                    zks::u8string& str = ret[ret.size() - 1];
                    for (int i = 0; i < (int)pCurrAddresses->PhysicalAddressLength; ++i) {
                        str.append(7, "%.2X", (int)pCurrAddresses->PhysicalAddress[i]);
                    }
                    str.shrink_to_fit();
                }
                pCurrAddresses = pCurrAddresses->Next;
            }
            ret.shrink_to_fit();
        }

        return ret;
    }

#elif defined (ZKS_OS_GNULINUX_)

#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <ifaddrs.h>
#include <cstdio>
#include <cstring>

    std::vector<zks::u8string> get_ifnames()
    {
        std::vector<zks::u8string> ret;
        struct ifaddrs *ifap, *ifa;
        if (getifaddrs(&ifap) != 0) {
            return ret;
        }
        for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
            ret.emplace_back(ifa->ifa_name);
        }
        freeifaddrs(ifap);
        return ret;
    }

    std::vector<zks::u8string> get_mac_address()
    {
        std::vector<zks::u8string> ret;
        struct ifreq ifrequest;
        typedef struct if_nameindex ifname_t;
        int dsize = 6; //only for AF_INET;
#ifndef SIOCGIFADDR
        return ret;
#endif

        int nSD = socket( PF_INET, SOCK_STREAM, 0);
        if (nSD < 0) {
            return ret;
        }

        ifname_t* pNameList = if_nameindex();
        for (ifname_t* p = pNameList; *(char *) p != 0; ++p) {
            strncpy(ifrequest.ifr_name, p->if_name, IF_NAMESIZE);
            if (ioctl(nSD, SIOCGIFHWADDR, &ifrequest) != 0) {
                return ret;
            }
            ret.push_back("");
            zks::u8string& str = ret[ret.size() - 1];
            str.reserve(dsize * 2);
            for (int i = 0; i < dsize; ++i) {
                str.append(zks::as_hex(ifrequest.ifr_ifru.ifru_hwaddr.sa_data[i]));
            }
            str.shrink_to_fit();
        }
        ret.shrink_to_fit();
        if_freenameindex(pNameList);
        close(nSD);
        return ret;
    }

#endif // OS;

    u8string as_hex(uint8_t c)
    {
        u8string ret;
        ret.reserve(2);
        char hh = (c & 0xf0) >> 4;
        ret += char(hh < 10 ? '0' + hh : 'A' + hh - 10);
        c &= 0x0f;
        ret += char(c < 10 ? '0' + c : 'A' + c - 10);
        return ret;
    }
    u8string as_hex(uint8_t* p, size_t len) {
        u8string ret;
        for(size_t i=0; i<len; ++i, ++p) {
            ret += as_hex(*p);
            ret += ' ';
        }
        return ret;
    }

} // namespace zks;
