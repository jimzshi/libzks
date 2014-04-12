#include "utility.h"

namespace zks {

#ifdef OS_WINDOWS

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
        } while ((dwRetVal == ERROR_BUFFER_OVERFLOW) && (Iterations < 3));

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


#endif // OS_WINDOWS

    u8string as_hex(uint8_t c) {
        u8string ret;
        ret.reserve(2);
        char hh = (c & 0xf0) >> 4;
        ret += char(hh < 10 ? '0' + hh : 'A' + hh - 10);
        c &= 0x0f;
        ret += char(c < 10 ? '0' + c : 'A' + c - 10);
        return ret;
    }

} // namespace zks;