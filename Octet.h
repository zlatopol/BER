#ifndef BER_OCTET_H
#define BER_OCTET_H

#include <climits>
#include <numeric>
#include <memory_resource>
#include <cassert>
#include <string>

#include "Util.h"

namespace BER {
    struct Octet {
        using value_type = std::uint8_t;
        template<std::size_t S>
        using bits_type = Bits<S, int>;

        value_type octet_;

        Octet() = default;

        constexpr Octet(value_type v) : octet_{v} {}

        constexpr operator value_type() const noexcept {
            return octet_;
        }

        /**
         * Get bits' subset for octet.
         * For example, sub_bits<7, 6>() for octet with value 0b1011'0000 returns bits_type<2>{0b10}
         * @tparam Start bits' subset start position
         * @tparam End  bits' subset end position
         * @return bits_type<Start - End>
         */
        template<std::size_t Start, std::size_t End>
        [[nodiscard]] constexpr bits_type<Start - End + 1> SubBits() const noexcept {
            static_assert(End <= Start);
            static_assert(Start < CHAR_BIT * sizeof(value_type));
            static_assert(End < CHAR_BIT * sizeof(value_type));

            return {(octet_ << (CHAR_BIT * sizeof(value_type) - Start) >> (CHAR_BIT * sizeof(value_type) - Start))
                            >> End};
        }
    };

    using OctetView = std::basic_string_view<Octet>;
    using OctetString = std::basic_string<Octet>;

    template<std::size_t S>
    using OctetBits = Octet::bits_type<S>;

    template<std::size_t... Sizes>
    Octet PackOctet(OctetBits<Sizes>... args) {
        static_assert((Sizes + ...) <= CHAR_BIT * sizeof(Octet::value_type));

        auto shift = CHAR_BIT * sizeof(Octet::value_type);
        OctetBits<1>::value_type arr[sizeof...(args)] {
                (shift -= Sizes, args
                        << shift)...
        };

        return std::accumulate(std::begin(arr), std::end(arr), 0, [](auto &&lhs, auto &&rhs) {
            return lhs | rhs;
        });
    }

    OctetString FromString(std::string_view view) {
        OctetString result;

        for (auto &&el : view) {
            result.push_back(el);
        }

        return result;
    }
}

#endif //BER_OCTET_H
