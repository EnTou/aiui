cmake_minimum_required(VERSION 2.8)

project(aiui)

add_definitions("-Wall -std=c++11")

add_executable(aiui.cpp base64.cpp base64.h md5.cpp md5.h)