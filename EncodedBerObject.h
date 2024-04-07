#ifndef BER_ENCODEDBEROBJECT_H
#define BER_ENCODEDBEROBJECT_H

#include "Octet.h"
#include "OctetClasses.h"
#include "Constants.h"

#include <type_traits>
#include <memory_resource>
#include <cmath>
#include <algorithm>

namespace BER {
    using EncodedBerObject = std::pmr::vector<Octet>;
    using ContentsOctetList = std::pmr::vector<Octet>;

    namespace detail {
        template<class T>
        ContentsOctetList IntegralContent(const T in) {
            auto t = in;

            ContentsOctetList content;

            if constexpr(std::is_trivial_v<T>) {
                content.reserve(sizeof(T));
            }

            while (t != T{}) {
                content.push_back(t & (pow_2<8, T> - 1)); // t % pow_2<8>
                t /= pow_2<8, T>;
            }

            if (in == 0 || (in > 0 && (content.back().SubBits<7, 7>() == 1))) {
                content.push_back(0);
            }

            return content;
        }

        template<class T>
        EncodedBerObject EncodeIntegral(const T in) {
            static_assert(std::is_unsigned_v<T> ||
                          (T{} == ~T(-1)), "T shall be two's complement");

            auto t = in;
            EncodedBerObject result;
            result.reserve(6);

            IdentifierOctet id_octet{
                    IdentifierOctet::ClassTagType{IdentifierOctet::Universal},
                    IdentifierOctet::Constructed{false},
                    IdentifierOctet::TagNumberType{UniversalTagList::INTEGER}
            };

            result.push_back(id_octet);

            ContentsOctetList content;
            if constexpr(std::is_trivial_v<T>) {
                content.reserve(sizeof(T));
            }

            while (t != T{}) {
                content.push_back(t & (pow_2<8, T> - 1)); // t % pow_2<8>
                t >>= 8;
                if constexpr(std::is_signed_v<T>) {
                    if (t == T{-1}) {
                        content.push_back(~0);
                        break;
                    }
                }
            }

            if (in > 0 && (content.back().SubBits<7, 7>() == 1)) {
                content.push_back(0);
            }

            if ((std::is_trivial_v<T> && sizeof(T) < pow_2<8>) || content.size() < pow_2<8>) {
                result.reserve(1 + content.size());
                result.push_back(content.size());
            } else {
                double sublength_oct_cnt = std::log(content.size()) / std::log(pow_2<9>) + 1;
                if (sublength_oct_cnt >= pow_2<8> - 1) {
                    throw std::logic_error{"Integer overflow"};
                }
                result.reserve(sublength_oct_cnt + 1 + content.size());

                result.push_back(PackOctet(OctetBits<1>{1}, OctetBits<7>{static_cast<int>(sublength_oct_cnt)}));
                for (int i = sublength_oct_cnt - 1; i >= 0; --i) {
                    result.push_back((content.size() >> (i * 8)) & 0xFF);
                }
            }

            std::for_each(std::rbegin(content), std::rend(content), [&](auto &&el) {
                result.push_back(el);
            });

            return result;
        }

        template<class T>
        EncodedBerObject EncodeReal(const T in) {
            EncodedBerObject result;
            result.reserve(5);

            const IdentifierOctet id_octet{
                    IdentifierOctet::ClassTagType{IdentifierOctet::Universal},
                    IdentifierOctet::Constructed{false},
                    IdentifierOctet::TagNumberType{UniversalTagList::REAL}
            };

            result.push_back(id_octet);

            if (in == 0) {
                result.push_back(0);
                return result;
            }

            int exp;
            const auto norm_mant = std::frexp(in, &exp);
            exp -= std::numeric_limits<T>::digits;

            auto choose_exp_sz = [](auto sz) {
                if (sz == 1) {
                    return 0b00;
                } else if (sz == 2) {
                    return 0b01;
                } else if (sz == 3) {
                    return 0b10;
                } else {
                    return 0b11;
                }
            };

            std::uintmax_t mantissa =
                    std::abs(norm_mant) * std::pow(std::numeric_limits<T>::radix, std::numeric_limits<T>::digits);

            while ((mantissa & 1) == 0) {
                mantissa >>= 1;
                ++exp;
            }

            const auto mant_content = IntegralContent(mantissa);

            const auto exp_content = IntegralContent(exp);

            const Octet info_octet = PackOctet(
                    OctetBits<1>{1}, // binary encoding
                    OctetBits<1>{in < 0}, // mantissa's sign
                    OctetBits<2>{0}, // base 2
                    OctetBits<2>{0}, // F
                    OctetBits<2>{
                            choose_exp_sz(exp_content.size())
                    }
            );

            ContentsOctetList dop_content;

            if (choose_exp_sz(exp_content.size()) == 0b11) {
                dop_content = EncodeIntegral(exp_content.size());
            }

            result.push_back(
                    LengthOctet(1 /*info_octet*/ + mant_content.size() + exp_content.size() + dop_content.size())
            );

            result.push_back(info_octet);
            for (auto &&el : exp_content) {
                result.push_back(el);
            }

            for (auto &&el : mant_content) {
                result.push_back(el);
            }

            return result;
        }
    }

    EncodedBerObject Encode(bool b) {
        IdentifierOctet id_octet{
                IdentifierOctet::ClassTagType{IdentifierOctet::Universal},
                IdentifierOctet::Constructed{false},
                IdentifierOctet::TagNumberType{UniversalTagList::BOOLEAN}
        };

        LengthOctet length_octet{1};
        ContentOctet content{b};
        return {id_octet, length_octet, content};
    }

    template<class T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
    EncodedBerObject Encode(T t) {
        if constexpr(std::is_integral_v<T>) {
            return detail::EncodeIntegral(t);
        } else if constexpr(std::is_floating_point_v<T>) {
            return detail::EncodeReal(t);
        }
    }

    EncodedBerObject Encode(OctetView str) {
        EncodedBerObject result;

        IdentifierOctet identifierOctet {
            IdentifierOctet::Universal,
            IdentifierOctet::Constructed{false},
            IdentifierOctet::TagNumberType{UniversalTagList::OCTET_STRING}
        };

        const LengthOctets length = LengthOctets::Encode(str.size());

        result.reserve(sizeof(IdentifierOctet) + sizeof(LengthOctet) + length.sub_octets.size() + str.size());

        result.push_back(identifierOctet);
        result.push_back(length.main_octet);

        for (auto &&el : length.sub_octets) {
            result.push_back(el);
        }

        for (auto &&el : str) {
            result.push_back(el);
        }

        return result;
    }
}


#endif //BER_ENCODEDBEROBJECT_H
