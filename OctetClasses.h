#ifndef BER_OCTETCLASSES_H
#define BER_OCTETCLASSES_H

namespace BER {
    class SubsequentIdOctet;

    class SubseqLengthOctet;

    struct IdentifierOctet : public Octet {
        using ClassTagType = OctetBits<2>;
        static constexpr ClassTagType Universal{0};
        static constexpr ClassTagType Application{1};
        static constexpr ClassTagType ContextSpecific{2};
        static constexpr ClassTagType Private{3};

        using Constructed = OctetBits<1>;

        using TagNumberType = OctetBits<5>;
        using SubOctet = SubsequentIdOctet;

        using Octet::Octet;

        IdentifierOctet(ClassTagType class_tag, Constructed primitive, TagNumberType tag_num) :
                Octet{PackOctet(class_tag, primitive, tag_num)} {}

        [[nodiscard]] constexpr ClassTagType ClassTag() const noexcept {
            return SubBits<7, 6>();
        }

        [[nodiscard]] constexpr Constructed IsPrimitive() const noexcept {
            return SubBits<5, 5>();
        }

        [[nodiscard]] constexpr TagNumberType TagNumber() const noexcept {
            return SubBits<4, 0>();
        }

        [[nodiscard]] constexpr bool IsLeadingOctet() const noexcept {
            return ~(this->TagNumber()) == 0;
        }
    };

    struct SubsequentIdOctet : public Octet {
        SubsequentIdOctet() = default;

        using Octet::Octet;

        using DataType = OctetBits<7>;

        [[nodiscard]] constexpr DataType Data() const noexcept {
            return SubBits<6, 0>();
        }

        [[nodiscard]] constexpr bool IsEnd() const noexcept {
            return SubBits<7, 7>() != 0;
        }
    };

    struct LengthOctet : public Octet {
        using DataType = OctetBits<7>;
        using SubOctet = SubseqLengthOctet;

        using Octet::Octet;

        [[nodiscard]] constexpr bool IsInDefinite() const noexcept {
            return SubBits<7, 7>() == 1 && SubBits<6, 1>() == 0;
        }

        [[nodiscard]] constexpr bool IsShort() const noexcept {
            return SubBits<7, 7>() == 0;
        }

        [[nodiscard]] constexpr DataType Data() const noexcept {
            return SubBits<6, 0>();
        }
    };

    struct SubseqLengthOctet : public Octet {
        using DataType = OctetBits<8>;

        using Octet::Octet;


        [[nodiscard]] constexpr DataType Data() const noexcept {
            return SubBits<7, 0>();
        }
    };

    struct LengthOctets {
        using value_type = std::uintmax_t;
        using ContentsOctetList = std::pmr::vector<Octet>;

        LengthOctet main_octet;
        ContentsOctetList sub_octets;

        [[nodiscard]] value_type size() const {
            if (sub_octets.size() > sizeof(value_type)) {
                throw std::logic_error{"Integer overflow"};
            }

            if (main_octet.IsShort()) {
                return main_octet.Data();
            }

            return DecodeIntegralImpl({&sub_octets[0], sub_octets.size()});
        }

        static value_type DecodeIntegralImpl(OctetView encoded) {
            value_type result = 0;
            bool neg = false;

            if (encoded[0].SubBits<7, 7>() == 1) {
                neg = true;
            }

            for (int i = 0; i < encoded.size() && (result <<= 8, 1); ++i) {
                result += (neg ? ~encoded[i].SubBits<7, 0>() : encoded[i].SubBits<7, 0>());
            }

            if (neg) {
                ++result;
                result *= -1;
            }

            return result;
        }

        template<class T>
        static ContentsOctetList IntegralContent(const T in) {
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

        static LengthOctets Encode(value_type length) {
            if (length < 127) {
                return {length, {}};
            }
            auto content = IntegralContent(length);
            if (content.size() >= 127) {
                throw std::logic_error{"Integer overflow"};
            }

            return {0x80 | content.size(), content};
        }
    };

    struct ContentOctet : public Octet {
        ContentOctet() = default;

        using Octet::Octet;

        [[nodiscard]] constexpr OctetBits<8> Content() const noexcept {
            return SubBits<7, 0>();
        }
    };

    struct EOCOctet : public Octet {
        EOCOctet() : Octet{0} {}

        [[nodiscard]] constexpr OctetBits<8> Content() const noexcept {
            return SubBits<7, 0>();
        }
    };
}

#endif //BER_OCTETCLASSES_H
