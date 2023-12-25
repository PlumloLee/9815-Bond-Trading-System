cmake_minimum_required(VERSION 3.24)
project(tradingsystem)

# Set the C++ standard to C++14
set(CMAKE_CXX_STANDARD 14)

# Find the required Boost library components
set(BOOST_ROOT "/Users/liziyuan/boost_1_84_0")
find_package(Boost 1.84.0)

# Add the executable and list the source files
add_executable(tradingsystem
        main.cpp
        algoexecutionservice.hpp
        algostreamingservice.hpp
        executionservice.hpp
        functions.hpp
        guiservice.hpp
        historicaldataservice.hpp
        inquiryservice.hpp
        marketdataservice.hpp
        positionservice.hpp
        pricingservice.hpp
        products.hpp
        riskservice.hpp
        soa.hpp
        streamingservice.hpp
        tradebookingservice.hpp)

# Link the Boost libraries if found
if(Boost_FOUND)
    target_include_directories(tradingsystem PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(tradingsystem PRIVATE ${Boost_LIBRARIES})
endif()
