#ifndef BER_DECODEDBEROBJECT_H
#define BER_DECODEDBEROBJECT_H

#include <cstdint>
#include <type_traits>
#include <typeindex>
#include <memory_resource>
#include <stdexcept>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <functional>
#include <iterator>
#include <cassert>

#include "Octet.h"
#include "OctetClasses.h"

namespace BER {
    using IntType = std::intmax_t;

    class DecodedBerObject {
        std::type_index type_info_;
        std::pmr::memory_resource *resource_;
        int type_sz_{};
        void *data_{};

        template<class T>
        static T &cast_impl(void *d) noexcept {
            return *static_cast<T *>(d);
        }

        template<class T>
        static T &cast_impl(const void *d) noexcept {
            return *static_cast<const T *>(d);
        }

        struct RAIIPointer {
            void *data;
            std::size_t sz;
            std::pmr::memory_resource *resource;

            ~RAIIPointer() {
                if (data != nullptr) {
                    resource->deallocate(data, sz);
                }
            }
        };

    public:
        struct BerCastError : std::runtime_error {
            using std::runtime_error::runtime_error;
        };

        DecodedBerObject() = delete;

        template<class T>
        explicit
        DecodedBerObject(T &&t, std::pmr::memory_resource *resource = std::pmr::get_default_resource()) : resource_(
                resource),
                                                                                                          type_info_(
                                                                                                                  typeid(T)) {
            RAIIPointer p{resource_->allocate(sizeof(T), alignof(T)),
                          sizeof(T),
                          resource_};

            cast_impl<std::decay_t<T>>(p.data) = std::forward<T>(t);
            std::swap(data_, p.data);
            type_sz_ = sizeof(T);
        }

        template<class T>
        std::optional<T> cast() {
            if (std::type_index{typeid(T)} != type_info_) {
                return std::nullopt;
            } else {
                return std::optional<T>{cast_impl<T>(data_)};
            }
        }

        ~DecodedBerObject() {
            resource_->deallocate(data_, type_sz_);
        }
    };

    namespace detail {
        IntType DecodeIntegralImpl(OctetView encoded) {
            IntType result = 0;
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

        DecodedBerObject DecodeIntegral(OctetView encoded) {
            assert(!encoded.empty() && IdentifierOctet{encoded[0]}.TagNumber().value == UniversalTagList::INTEGER);

            IntType result = 0;
            bool neg = false;

            encoded.remove_prefix(1);
            LengthOctet length{encoded.front()};
            encoded.remove_prefix(1);
            assert(length.IsShort());
            if (length.Data() != encoded.size())  {
                throw std::logic_error{"Sizes' mismatch"};
            }

            if (encoded[0].SubBits<7, 7>() == 1) {
                neg = true;
            }

            for (int i = 0; i < length.Data() && (result <<= 8, 1); ++i) {
                result += (neg ? ~encoded[i].SubBits<7, 0>() : encoded[i].SubBits<7, 0>());
            }

            if (neg) {
                ++result;
                result *= -1;
            }
            return DecodedBerObject{result};
        }

        inline const std::unordered_map<UniversalTagList::Type, std::function<DecodedBerObject(
                OctetView)>> decoders_map = {
                {UniversalTagList::INTEGER, DecodeIntegral}
        };
    }

    DecodedBerObject Decode(OctetView view) {
        if (view.empty()) {
            throw std::logic_error{"Empty octet stream"};
        }

        IdentifierOctet id_octet{view[0]};

        if (id_octet.ClassTag().value != IdentifierOctet::Universal.value) {
            throw std::logic_error{"Universal is only supported"};
        }

        return detail::decoders_map.at(static_cast<UniversalTagList::Type>(id_octet.TagNumber().value))(view);
    }

    DecodedBerObject Decode(const EncodedBerObject &encoded) {
        return Decode(OctetView(&encoded.front(), encoded.size()));
    }
}


#endif //BER_DECODEDBEROBJECT_H
