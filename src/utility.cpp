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
#elif defined (ZKS_OS_MACOSX_)

#include <stdio.h>

#include <CoreFoundation/CoreFoundation.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/network/IONetworkInterface.h>
#include <IOKit/network/IOEthernetController.h>

    static kern_return_t FindEthernetInterfaces(io_iterator_t *matchingServices);

    static kern_return_t FindEthernetInterfaces(io_iterator_t *matchingServices)
    {
        kern_return_t           kernResult;
        CFMutableDictionaryRef	matchingDict;
        CFMutableDictionaryRef	propertyMatchDict;

        matchingDict = IOServiceMatching(kIOEthernetInterfaceClass);

        if (NULL == matchingDict) {
            printf("IOServiceMatching returned a NULL dictionary.\n");
        }
        else {
            propertyMatchDict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                &kCFTypeDictionaryKeyCallBacks,
                &kCFTypeDictionaryValueCallBacks);

            if (NULL == propertyMatchDict) {
                printf("CFDictionaryCreateMutable returned a NULL dictionary.\n");
            }
            else {
                CFDictionarySetValue(propertyMatchDict, CFSTR(kIOPrimaryInterface), kCFBooleanTrue);

                CFDictionarySetValue(matchingDict, CFSTR(kIOPropertyMatchKey), propertyMatchDict);
                CFRelease(propertyMatchDict);
            }
        }

        kernResult = IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDict, matchingServices);
        if (KERN_SUCCESS != kernResult) {
            printf("IOServiceGetMatchingServices returned 0x%08x\n", kernResult);
        }

        return kernResult;
    }

    std::vector<zks::u8string> get_mac_address()
    {
        io_iterator_t intfIterator;
        io_object_t intfService;
        io_object_t controllerService;
        kern_return_t kernResult = KERN_FAILURE;

        std::vector<zks::u8string> ret;
        kernResult = FindEthernetInterfaces(&intfIterator);
        if (KERN_SUCCESS != kernResult) {
            return ret;
        }

        while ((intfService = IOIteratorNext(intfIterator)))
        {
            CFTypeRef MACAddressAsCFData;

            kernResult = IORegistryEntryGetParentEntry(intfService, kIOServicePlane, &controllerService);
            if (KERN_SUCCESS == kernResult) {
                MACAddressAsCFData = IORegistryEntryCreateCFProperty(controllerService,
                    CFSTR(kIOMACAddress),
                    kCFAllocatorDefault,
                    0);
                if (MACAddressAsCFData) {
                    UInt8 MACAddress[kIOEthernetAddressSize];
                    bzero(MACAddress, kIOEthernetAddressSize);
                    CFDataGetBytes((CFDataRef)MACAddressAsCFData, CFRangeMake(0, kIOEthernetAddressSize), MACAddress);
                    ret.push_back(as_hex(MACAddress, 6));
                    CFRelease(MACAddressAsCFData);
                }
                (void)IOObjectRelease(controllerService);
            }
            (void)IOObjectRelease(intfService);
        }
        (void)IOObjectRelease(intfIterator);

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
        for (size_t i = 0; i < len; ++i, ++p) {
            ret += as_hex(*p);
            ret += ' ';
        }
        return ret;
    }

    namespace {

        // handle all next_prime(i) for i in [1, 210), special case 0
        const unsigned small_primes[] =
        {
            0,
            2,
            3,
            5,
            7,
            11,
            13,
            17,
            19,
            23,
            29,
            31,
            37,
            41,
            43,
            47,
            53,
            59,
            61,
            67,
            71,
            73,
            79,
            83,
            89,
            97,
            101,
            103,
            107,
            109,
            113,
            127,
            131,
            137,
            139,
            149,
            151,
            157,
            163,
            167,
            173,
            179,
            181,
            191,
            193,
            197,
            199,
            211
        };

        // potential primes = 210*k + indices[i], k >= 1
        //   these numbers are not divisible by 2, 3, 5 or 7
        //   (or any integer 2 <= j <= 10 for that matter).
        const unsigned indices[] =
        {
            1,
            11,
            13,
            17,
            19,
            23,
            29,
            31,
            37,
            41,
            43,
            47,
            53,
            59,
            61,
            67,
            71,
            73,
            79,
            83,
            89,
            97,
            101,
            103,
            107,
            109,
            113,
            121,
            127,
            131,
            137,
            139,
            143,
            149,
            151,
            157,
            163,
            167,
            169,
            173,
            179,
            181,
            187,
            191,
            193,
            197,
            199,
            209
        };

    } //anonymous namespace;

    bool is_prime(std::size_t n)
    {
        const size_t N = sizeof(small_primes) / sizeof(small_primes[0]);
        for (std::size_t i = 3; i < N; ++i)
        {
            const std::size_t p = small_primes[i];
            const std::size_t q = n / p;
            if (q < p)
                return true;
            if (n == q * p)
                return false;
        }
        for (std::size_t i = 211; true;)
        {
            std::size_t q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 10;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 2;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 4;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 2;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 4;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 6;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 2;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 6;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 4;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 2;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 4;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 6;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 6;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 2;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 6;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 4;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 2;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 6;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 4;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 6;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 8;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 4;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 2;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 4;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 2;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 4;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 8;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 6;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 4;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 6;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 2;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 4;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 6;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 2;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 6;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 6;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 4;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 2;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 4;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 6;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 2;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 6;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 4;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 2;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 4;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 2;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            i += 10;
            q = n / i;
            if (q < i)
                return true;
            if (n == q * i)
                return false;

            // This will loop i to the next "plane" of potential primes
            i += 2;
        }
        return true;
    }

    size_t next_prime(size_t n)
    {
        const size_t L = 210;
        const size_t N = sizeof(small_primes) / sizeof(small_primes[0]);
        // If n is small enough, search in small_primes
        if (n <= small_primes[N - 1])
            return *std::lower_bound(small_primes, small_primes + N, n);

        // Start searching list of potential primes: L * k0 + indices[in]
        const size_t M = sizeof(indices) / sizeof(indices[0]);
        // Select first potential prime >= n
        //   Known a-priori n >= L
        size_t k0 = n / L;
        size_t in = static_cast<size_t>(std::lower_bound(indices, indices + M, n - k0 * L)
            - indices);
        n = L * k0 + indices[in];
        while (true)
        {
            // Divide n by all primes or potential primes (i) until:
            //    1.  The division is even, so try next potential prime.
            //    2.  The i > sqrt(n), in which case n is prime.
            // It is known a-priori that n is not divisible by 2, 3, 5 or 7,
            //    so don't test those (j == 5 ->  divide by 11 first).  And the
            //    potential primes start with 211, so don't test against the last
            //    small prime.
            for (size_t j = 5; j < N - 1; ++j)
            {
                const std::size_t p = small_primes[j];
                const std::size_t q = n / p;
                if (q < p)
                    return n;
                if (n == q * p)
                    goto next;
            }
            // n wasn't divisible by small primes, try potential primes
        {
            size_t i = 211;
            while (true)
            {
                std::size_t q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 10;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 2;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 4;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 2;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 4;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 6;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 2;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 6;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 4;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 2;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 4;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 6;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 6;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 2;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 6;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 4;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 2;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 6;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 4;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 6;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 8;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 4;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 2;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 4;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 2;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 4;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 8;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 6;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 4;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 6;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 2;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 4;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 6;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 2;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 6;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 6;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 4;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 2;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 4;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 6;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 2;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 6;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 4;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 2;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 4;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 2;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                i += 10;
                q = n / i;
                if (q < i)
                    return n;
                if (n == q * i)
                    break;

                // This will loop i to the next "plane" of potential primes
                i += 2;
            }
        }
    next:
        // n is not prime.  Increment n to next potential prime.
        if (++in == M)
        {
            ++k0;
            in = 0;
        }
        n = L * k0 + indices[in];
        }
    }

} // namespace zks;
