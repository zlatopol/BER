#ifndef BER_CONSTANTS_H
#define BER_CONSTANTS_H

namespace BER {
    struct UniversalTagList {
        enum Type {
            RESERVED = 0,
            BOOLEAN = 1,
            INTEGER = 2,
            BIT_STRING = 3,
            OCTET_STRING = 4,
            NULL_TYPE = 5,
            OBJECT_IDENTIFIER = 6,
            ObjectDescriptor = 7,
            EXTERNAL = 8,
            REAL = 9,
            ENUMERATED = 10,
            EMBEDDED_PDV = 11,
            UTF8String = 12,
            RELATIVE_OID = 13,
            SEQUENCE = 16,
            SEQUENCE_OF = 16,
            SET = 17,
            SET_OF = 17,
            NumericString = 18,
            PrintableString = 19,
            T61String = 20,
            VideotexString = 21,
            IA5String = 22,
            UTCTime = 23,
            GeneralizedTime = 24,
            GraphicString = 25,
            VisibleString = 26,
            ISO646String = 26,
            GeneralString = 27,
            UniversalString = 28,
            CHARACTER_STRING = 29,
            BMPString = 30
        };
    };
}

#endif //BER_CONSTANTS_H
