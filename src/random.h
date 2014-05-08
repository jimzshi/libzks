#ifndef ZKS_RANDOM_H_
#define ZKS_RANDOM_H_

#include "u8string.h"

#include <random>
#include <ctime>
#include <string>

namespace zks {
    template<typename EngT_ = std::default_random_engine>
    inline EngT_& global_rng() {
        static EngT_ rng{};
        return rng;
    }

    template<typename EngT_ = std::default_random_engine>
    inline void randomize() {
        static std::random_device rd{};
        global_rng<EngT_>().seed(rd());
    }

    template<
        typename NumT_ = int,
        typename EngT_ = std::default_random_engine,
        typename DistT_ = std::conditional_t<std::is_integral<NumT_>::value,
                                            std::uniform_int_distribution<NumT_>,
                                            std::uniform_real_distribution<NumT_> >
    > struct RNGen {
        DistT_ d;
        using param_t = typename DistT_::param_type;
        NumT_ operator()(NumT_ f, NumT_ t) {
            return d(global_rng<EngT_>(), param_t{ f, t });
        }
        NumT_ operator()() {
            return d(global_rng<EngT_>());
        }
    };

    template<
        typename NumT_ = int,
        typename EngT_ = std::default_random_engine,
        typename DistT_ = std::conditional_t<std::is_integral<NumT_>::value,
                                            std::uniform_int_distribution<NumT_>,
                                            std::uniform_real_distribution<NumT_> >
    > struct RangedRNGen {
        NumT_ from, to;
        DistT_ d;
        using param_t = typename DistT_::param_type;
        RangedRNGen(NumT_ from_ = 0, NumT_ to_ = 1) : from(from_), to(to_) {}
        NumT_ operator()(NumT_ f, NumT_ t) {
            return d(global_rng<EngT_>(), param_t{ f, t });
        }
        NumT_ operator()() {
            return d(global_rng<EngT_>(), param_t{ from, to });
        }
    };


    template<int F = 65, int T = 90>
    inline char rand_char() {
        static RangedRNGen<> rng{ F, T };
        return (char)rng();
    }

    template<int L = 13, int F = 65, int T = 90>
    inline std::string rand_string() {
        static RangedRNGen<size_t> lrng{ 1, L };
        static RangedRNGen<> rng{ F, T };
        std::string ret;
        ret.resize(lrng());
        for (auto& c : ret) {
            c = rand_char<F, T>();
        }
        return ret;
    }

    template<int L = 13, int F = 65, int T = 90>
    inline zks::u8string rand_u8string() {
        static RangedRNGen<size_t> lrng{ 1, L };
        static RangedRNGen<> rng{ F, T };
        zks::u8string ret;
        ret.resize(lrng());
        for (auto& c : ret) {
            c = rand_char<F, T>();
        }
        return ret;
    }

    template<typename ForwardIter_, typename RNG_ >
    inline void sample(ForwardIter_ beg, ForwardIter_ end, RNG_& rng) {
        for (; beg != end; ++beg) {
            *beg = rng();
        }
    }
}

#endif