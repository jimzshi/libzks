#ifndef _ZKS_ITERATOR_H
#define _ZKS_ITERATOR_H

#include <type_traits>
#include <iterator>

namespace zks
{

    using std::iterator_traits;
    using std::iterator;

    template<typename Iterator_, typename Container_>
    class normal_iterator_
    {
    protected:
        Iterator_ _M_current;

        typedef iterator_traits<Iterator_> __traits_type;

    public:
        typedef Iterator_ iterator_type;
        typedef typename __traits_type::iterator_category iterator_category;
        typedef typename __traits_type::value_type value_type;
        typedef typename __traits_type::difference_type difference_type;
        typedef typename __traits_type::reference reference;
        typedef typename __traits_type::pointer pointer;

        normal_iterator_() :
                _M_current(Iterator_())
        {
        }
        explicit normal_iterator_(const Iterator_& __i) :
                _M_current(__i)
        {
        }

        // Allow iterator to const_iterator conversion
        template<typename Iter_>
        normal_iterator_(const normal_iterator_<Iter_, std::enable_if_t<std::is_same<Iter_, typename Container_::pointer>::value, Container_> >& __i) :
                _M_current(__i.base())
        {
        }

        // Forward iterator requirements
        reference operator*() const
        {
            return *_M_current;
        }

        pointer operator->() const
        {
            return _M_current;
        }

        normal_iterator_& operator++()
        {
            ++_M_current;
            return *this;
        }

        normal_iterator_ operator++(int)
        {
            return normal_iterator_(_M_current++);
        }

        // Bidirectional iterator requirements
        normal_iterator_& operator--()
        {
            --_M_current;
            return *this;
        }

        normal_iterator_ operator--(int)
        {
            return normal_iterator_(_M_current--);
        }

        // Random access iterator requirements
        reference operator[](const difference_type& __n) const
        {
            return _M_current[__n];
        }

        normal_iterator_& operator+=(const difference_type& __n)
        {
            _M_current += __n;
            return *this;
        }

        normal_iterator_ operator+(const difference_type& __n) const
        {
            return normal_iterator_(_M_current + __n);
        }

        normal_iterator_& operator-=(const difference_type& __n)
        {
            _M_current -= __n;
            return *this;
        }

        normal_iterator_ operator-(const difference_type& __n) const
        {
            return normal_iterator_(_M_current - __n);
        }

        const Iterator_& base() const
        {
            return _M_current;
        }
    };

    // Forward iterator requirements
    template<typename IterLeft_, typename IterRight_, typename Container_>
    inline bool operator==(const normal_iterator_<IterLeft_, Container_>& __lhs, const normal_iterator_<IterRight_, Container_>& __rhs)
    {
        return __lhs.base() == __rhs.base();
    }

    template<typename Iterator_, typename Container_>
    inline bool operator==(const normal_iterator_<Iterator_, Container_>& __lhs, const normal_iterator_<Iterator_, Container_>& __rhs)
    {
        return __lhs.base() == __rhs.base();
    }

    template<typename IterLeft_, typename IterRight_, typename Container_>
    inline bool operator!=(const normal_iterator_<IterLeft_, Container_>& __lhs, const normal_iterator_<IterRight_, Container_>& __rhs)
    {
        return __lhs.base() != __rhs.base();
    }

    template<typename Iterator_, typename Container_>
    inline bool operator!=(const normal_iterator_<Iterator_, Container_>& __lhs, const normal_iterator_<Iterator_, Container_>& __rhs)
    {
        return __lhs.base() != __rhs.base();
    }

    // Random access iterator requirements
    template<typename IterLeft_, typename IterRight_, typename Container_>
    inline bool operator<(const normal_iterator_<IterLeft_, Container_>& __lhs, const normal_iterator_<IterRight_, Container_>& __rhs)
    {
        return __lhs.base() < __rhs.base();
    }

    template<typename Iterator_, typename Container_>
    inline bool operator<(const normal_iterator_<Iterator_, Container_>& __lhs, const normal_iterator_<Iterator_, Container_>& __rhs)
    {
        return __lhs.base() < __rhs.base();
    }

    template<typename IterLeft_, typename IterRight_, typename Container_>
    inline bool operator>(const normal_iterator_<IterLeft_, Container_>& __lhs, const normal_iterator_<IterRight_, Container_>& __rhs)
    {
        return __lhs.base() > __rhs.base();
    }

    template<typename Iterator_, typename Container_>
    inline bool operator>(const normal_iterator_<Iterator_, Container_>& __lhs, const normal_iterator_<Iterator_, Container_>& __rhs)
    {
        return __lhs.base() > __rhs.base();
    }

    template<typename IterLeft_, typename IterRight_, typename Container_>
    inline bool operator<=(const normal_iterator_<IterLeft_, Container_>& __lhs, const normal_iterator_<IterRight_, Container_>& __rhs)
    {
        return __lhs.base() <= __rhs.base();
    }

    template<typename Iterator_, typename Container_>
    inline bool operator<=(const normal_iterator_<Iterator_, Container_>& __lhs, const normal_iterator_<Iterator_, Container_>& __rhs)
    {
        return __lhs.base() <= __rhs.base();
    }

    template<typename IterLeft_, typename IterRight_, typename Container_>
    inline bool operator>=(const normal_iterator_<IterLeft_, Container_>& __lhs, const normal_iterator_<IterRight_, Container_>& __rhs)
    {
        return __lhs.base() >= __rhs.base();
    }

    template<typename Iterator_, typename Container_>
    inline bool operator>=(const normal_iterator_<Iterator_, Container_>& __lhs, const normal_iterator_<Iterator_, Container_>& __rhs)
    {
        return __lhs.base() >= __rhs.base();
    }

    // According to the resolution of DR179 not only the various comparison
    // operators but also operator- must accept mixed iterator/const_iterator
    // parameters.
    template<typename IterLeft_, typename IterRight_, typename Container_>
    inline typename normal_iterator_<IterLeft_, Container_>::difference_type operator-(const normal_iterator_<IterLeft_, Container_>& __lhs,
            const normal_iterator_<IterRight_, Container_>& __rhs)
    {
        return __lhs.base() - __rhs.base();
    }

    template<typename Iterator_, typename Container_>
    inline typename normal_iterator_<Iterator_, Container_>::difference_type operator-(const normal_iterator_<Iterator_, Container_>& __lhs,
            const normal_iterator_<Iterator_, Container_>& __rhs)
    {
        return __lhs.base() - __rhs.base();
    }

    template<typename Iterator_, typename Container_>
    inline normal_iterator_<Iterator_, Container_> operator+(typename normal_iterator_<Iterator_, Container_>::difference_type __n,
            const normal_iterator_<Iterator_, Container_>& __i)
    {
        return normal_iterator_<Iterator_, Container_>(__i.base() + __n);
    }

    template<typename Iterator_, typename Container_>
    class forward_iterator_
    {
    protected:
        Iterator_ _M_current;

        typedef iterator_traits<Iterator_> __traits_type;

    public:
        typedef Iterator_ iterator_type;
        typedef typename __traits_type::iterator_category iterator_category;
        typedef typename __traits_type::value_type value_type;
        typedef typename __traits_type::difference_type difference_type;
        typedef typename __traits_type::reference reference;
        typedef typename __traits_type::pointer pointer;

        forward_iterator_() :
                _M_current(Iterator_())
        {
        }
        explicit forward_iterator_(const Iterator_& __i) :
                _M_current(__i)
        {
        }

        // Allow iterator to const_iterator conversion
        template<typename Iter_>
        forward_iterator_(const forward_iterator_<Iter_, std::enable_if_t<std::is_same<Iter_, typename Container_::pointer>::value, Container_>>& __i) :
                _M_current(__i.base())
        {
        }

        // Forward iterator requirements
        reference operator*() const
        {
            return *_M_current;
        }

        pointer operator->() const
        {
            return _M_current;
        }

        forward_iterator_& operator++()
        {
            ++_M_current;
            return *this;
        }

        forward_iterator_ operator++(int)
        {
            return forward_iterator_(_M_current++);
        }

        forward_iterator_& operator+=(const difference_type& __n)
        {
            _M_current += __n;
            return *this;
        }

        forward_iterator_ operator+(const difference_type& __n) const
        {
            return forward_iterator_(_M_current + __n);
        }

        const Iterator_& base() const
        {
            return _M_current;
        }
    };

    template<typename IterLeft_, typename IterRight_, typename Container_>
    inline bool operator==(const forward_iterator_<IterLeft_, Container_>& __lhs, const forward_iterator_<IterRight_, Container_>& __rhs)
    {
        return __lhs.base() == __rhs.base();
    }

    template<typename Iterator_, typename Container_>
    inline bool operator==(const forward_iterator_<Iterator_, Container_>& __lhs, const forward_iterator_<Iterator_, Container_>& __rhs)
    {
        return __lhs.base() == __rhs.base();
    }

    template<typename IterLeft_, typename IterRight_, typename Container_>
    inline bool operator!=(const forward_iterator_<IterLeft_, Container_>& __lhs, const forward_iterator_<IterRight_, Container_>& __rhs)
    {
        return __lhs.base() != __rhs.base();
    }

    template<typename Iterator_, typename Container_>
    inline bool operator!=(const forward_iterator_<Iterator_, Container_>& __lhs, const forward_iterator_<Iterator_, Container_>& __rhs)
    {
        return __lhs.base() != __rhs.base();
    }

    template<typename IterLeft_, typename IterRight_, typename Container_>
    inline bool operator<(const forward_iterator_<IterLeft_, Container_>& __lhs, const forward_iterator_<IterRight_, Container_>& __rhs)
    {
        return __lhs.base() < __rhs.base();
    }

    template<typename Iterator_, typename Container_>
    inline bool operator<(const forward_iterator_<Iterator_, Container_>& __lhs, const forward_iterator_<Iterator_, Container_>& __rhs)
    {
        return __lhs.base() < __rhs.base();
    }

    template<typename IterLeft_, typename IterRight_, typename Container_>
    inline bool operator>(const forward_iterator_<IterLeft_, Container_>& __lhs, const forward_iterator_<IterRight_, Container_>& __rhs)
    {
        return __lhs.base() > __rhs.base();
    }

    template<typename Iterator_, typename Container_>
    inline bool operator>(const forward_iterator_<Iterator_, Container_>& __lhs, const forward_iterator_<Iterator_, Container_>& __rhs)
    {
        return __lhs.base() > __rhs.base();
    }

    template<typename IterLeft_, typename IterRight_, typename Container_>
    inline bool operator<=(const forward_iterator_<IterLeft_, Container_>& __lhs, const forward_iterator_<IterRight_, Container_>& __rhs)
    {
        return __lhs.base() <= __rhs.base();
    }

    template<typename Iterator_, typename Container_>
    inline bool operator<=(const forward_iterator_<Iterator_, Container_>& __lhs, const forward_iterator_<Iterator_, Container_>& __rhs)
    {
        return __lhs.base() <= __rhs.base();
    }

    template<typename IterLeft_, typename IterRight_, typename Container_>
    inline bool operator>=(const forward_iterator_<IterLeft_, Container_>& __lhs, const forward_iterator_<IterRight_, Container_>& __rhs)
    {
        return __lhs.base() >= __rhs.base();
    }

    template<typename Iterator_, typename Container_>
    inline bool operator>=(const forward_iterator_<Iterator_, Container_>& __lhs, const forward_iterator_<Iterator_, Container_>& __rhs)
    {
        return __lhs.base() >= __rhs.base();
    }

    // According to the resolution of DR179 not only the various comparison
    // operators but also operator- must accept mixed iterator/const_iterator
    // parameters.
    template<typename IterLeft_, typename IterRight_, typename Container_>
    inline typename forward_iterator_<IterLeft_, Container_>::difference_type operator-(const forward_iterator_<IterLeft_, Container_>& __lhs,
            const forward_iterator_<IterRight_, Container_>& __rhs)
    {
        return __lhs.base() - __rhs.base();
    }

    template<typename Iterator_, typename Container_>
    inline typename forward_iterator_<Iterator_, Container_>::difference_type operator-(const forward_iterator_<Iterator_, Container_>& __lhs,
            const forward_iterator_<Iterator_, Container_>& __rhs)
    {
        return __lhs.base() - __rhs.base();
    }

    template<typename Iterator_, typename Container_>
    inline forward_iterator_<Iterator_, Container_> operator+(typename forward_iterator_<Iterator_, Container_>::difference_type __n,
            const forward_iterator_<Iterator_, Container_>& __i)
    {
        return forward_iterator_<Iterator_, Container_>(__i.base() + __n);
    }

} /* namespace zks */

#endif /*_ZKS_ITERATOR_H*/
