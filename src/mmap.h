#ifndef ZKS_MMAP_H_
#define ZKS_MMAP_H_

/*from
http://cpp.indi.frih.net/blog/2014/09/how-to-read-an-entire-file-into-memory-in-cpp/
*/

#include <type_traits>
#include <ostream>
#include <sstream>
#include <limits>
#include <array>
#include <vector>
#include <deque>

namespace zks {

    /* go to method for small files (<100K)
    auto ss = std::ostringstream{};
    ss << in.rdbuf();
    auto s = ss.str();
    Problems: has to copy data from the ostringstream into a string and for large
    data this could be an issue due to having two copies of large data in memory.
    */
    template <typename CharT,
        typename Traits = std::char_traits<CharT>,
        typename Allocator = std::allocator<CharT>
    >
        std::basic_string<CharT, Traits, Allocator>
        mmap(std::basic_istream<CharT, Traits>& in, Allocator alloc = {})
    {
        std::basic_ostringstream<CharT, Traits, Allocator>
            ss(std::basic_string<CharT, Traits, Allocator>(std::move(alloc)));

        if (!(ss << in.rdbuf()))
            throw std::ios_base::failure{ "error" };

        return ss.str();
    }

    /* reading straight into a container:
    If you are dealing with files it can be faster to count all the
    characters first, then do one big allocation and one big whopper of a read:
    auto const start_pos = in.tellg();
    in.ignore(std::numeric_limits<std::streamsize>::max());
    auto const char_count = in.gcount();
    in.seekg(start_pos);
    auto s = std::string(char_count, char{});
    in.read(&s[0], s.size());
    in.ignore() is a safe way to count the bytes in a file but means this method
    requires reading the file twice, once to count bytes and once to read them in.
    */
    template <typename Container = std::string,
        typename CharT = char,
        typename Traits = std::char_traits<char>
    >
        Container mmap(std::basic_istream<CharT, Traits>& in,
            typename Container::allocator_type alloc = {})
    {
        static_assert(
            // Allow only strings...
            std::is_same<Container,
            std::basic_string<CharT, Traits, typename Container::allocator_type>
            >::value ||
            // ... and vectors of the plain, signed, and
            // unsigned flavours of CharT.
            std::is_same<Container,
            std::vector<CharT, typename Container::allocator_type>
            >::value ||
            std::is_same<Container,
            std::vector<std::make_unsigned<CharT>, typename Container::allocator_type>
            >::value ||
            std::is_same<Container,
            std::vector<std::make_signed<CharT>, typename Container::allocator_type>
            >::value,
            "only strings and vectors of ((un)signed) CharT allowed");

        auto const start_pos = in.tellg();
        if (std::streamsize(-1) == start_pos)
            throw std::ios_base::failure{ "error" };

        if (!in.ignore(std::numeric_limits<std::streamsize>::max()))
            throw std::ios_base::failure{ "error" };

        auto const char_count = in.gcount();

        if (!in.seekg(start_pos))
            throw std::ios_base::failure{ "error" };

        auto container = Container(std::move(alloc));
        container.resize(char_count);

        if (0 != container.size()) {
            if (!in.read(reinterpret_cast<CharT*>(&container[0]), container.size()))
                throw std::ios_base::failure{ "error" };
        }

        return container;
    }

    /* read chunks into a deque:
    If you’re expecting enormous files (at least several hundreds of megabytes,
    on average) and you don’t want to seek on stream, read the file in chunks into
    a deque. Advantage is no copy unless you can't work with the dequeu and end up
    copying the data out of it.
    */
    template <typename CharT,
        typename Traits = std::char_traits<CharT>,
        typename CharO = CharT,
        typename Allocator = std::allocator<CharO>
    >
        std::deque<CharO, Allocator>
        mmap(std::basic_istream<CharT, Traits>& in, Allocator alloc = {})
    {
        static_assert(std::is_same<CharT, CharO>::value ||
            std::is_same<std::make_unsigned<CharT>, CharO>::value ||
            std::is_same<std::make_signed<CharT>, CharO>::value,
            "char type of deque must be same "
            "as stream char type "
            "(possibly signed or unsigned)");

        using std::begin;
        using std::end;

        auto const chunk_size = std::size_t{ BUFSIZ };

        auto container = std::deque<CharO, Allocator>(std::move(alloc));

        auto chunk = std::array<CharO, chunk_size>{};

        while (in.read(reinterpret_cast<CharT*>(chunk.data()), chunk.size())
            || in.gcount()
            ) {
            container.insert(end(container), begin(chunk), begin(chunk) + in.gcount());
        }

        return container;
    }

} //namespace zks

#endif
