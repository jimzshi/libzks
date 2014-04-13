#ifndef ZKS_BIT_HACK_H_
#define ZKS_BIT_HACK_H_

#include "utility.h"

#include <type_traits>

namespace zks {

    template<typename T_, T_ N_, unsigned MAX_ = sizeof(T_) * CHAR_BIT, unsigned CUR_ = 1,
        typename = typename std::enable_if<std::is_integral<T_>::value>::type>
    struct NextPowerOf2 {
        static const T_ value = (MAX_ <= CUR_) ? N_ :
            NextPowerOf2<T_, (((N_ - 1) | ((N_ - 1) >> CUR_)) + 1), MAX_, (CUR_ << 1)>::value;
    };

    template<typename T_, T_ N_, unsigned M_>
    struct NextPowerOf2 < T_, N_, M_, M_, void > {
        static const T_ value = N_ ? N_ : 1;
    };

    template<typename T_>
    inline T_ next_pow2(T_ x) {
        static unsigned maxb = sizeof(T_)*CHAR_BIT;
        --x;
        for (unsigned i = 1; i < maxb; ++i) {
            x |= x >> i;
        }
        return ++x;
    }

    template<typename T_, T_ N_, unsigned R_=0,
        typename = typename std::enable_if<std::is_integral<T_>::value>::type>
    struct LogBase2 {
        static const T_ value = N_ >> 1 ? LogBase2<T_, (N_ >> 1), (R_ + 1)>::value : R_;
    };

    template<unsigned R_> struct LogBase2 < int, 0, R_, void > {
        static const int value = 0;
    };

} // namespace zks;




#endif