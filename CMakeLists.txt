cmake_minimum_required(VERSION 3.19)
project(BER)

set(CMAKE_CXX_STANDARD 17)

add_executable(BER main.cpp DecodedBerObject.h Octet.h EncodedBerObject.h Constants.h Util.h OctetClasses.h)