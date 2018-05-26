cmake_minimum_required(VERSION 3.9)
project(SmallAnalysis)

set(CMAKE_CXX_STANDARD 11)

add_executable(SmallAnalysis main.cpp analysis.cpp analysis.h structinfo.h strutil.h source/test.h bitutil.h)