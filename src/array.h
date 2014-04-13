#ifndef ZKS_ARRAY_H_
#define ZKS_ARRAY_H_

#include "bit_hack.h"
#include <type_traits>

namespace zks {

    template<typename T_, 
            class = std::enable_if_t<!std::is_pointer<T_>::value && !std::is_reference<T_>::value> >
    struct Chunk_type_traits_ {
        typedef typename std::remove_cv<T_>::type Type_;
        typedef Type_* pointer;
        typedef Type_& reference;
        typedef Type_ const* const_ptr;
        typedef Type_ const& const_ref;
    };

    template<typename T_, int ChunkSize_, int BlockSize_>
    struct Chunk_size_traits_ {
        typedef typename Chunk_type_traits_<T_>::Type_ Ty_;
            static const int m_type_bytes_ = sizeof(Ty_);
            static const int m_chunk_bytes_ = zks::NextPowerOf2<int, ChunkBytes_>::value;
            static const int m_objs_in_chunk_ = m_chunk_bytes_ / m_type_bytes_;
            static const int m_mask_ = m_chunk_bytes_ - 1;
            static const int m_mask_bits_ = zks::LogBase2 <int, m_chunk_bytes_> ;
            int m_size_;
            int m_chunk_size_;
            int m_capacity_;
        };


    };
}

#endif
