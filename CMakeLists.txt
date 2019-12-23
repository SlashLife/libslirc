cmake_minimum_required(VERSION 3.9)
project(libslirc)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS system)

set(CMAKE_CXX_STANDARD 17)

include_directories(${Boost_INCLUDE_DIRS})

add_library(libslirc SHARED src/entry.cpp include/slirc/entry.hpp src/irc.cpp include/slirc/irc.hpp src/module.cpp include/slirc/module.hpp src/event_id.cpp include/slirc/event_id.hpp src/event.cpp include/slirc/event.hpp src/util/component_map.cpp include/slirc/util/component_map.hpp src/util/spin_lock.cpp include/slirc/util/spin_lock.hpp src/apis/connection.cpp include/slirc/apis/connection.hpp src/modules/connection.cpp include/slirc/modules/connection.hpp src/network.cpp include/slirc/network.hpp src/packages/load_module.cpp include/slirc/packages/load_module.hpp)
target_link_libraries(libslirc ${Boost_LIBRARIES} wsock32)

add_executable(testslirc main.cpp)
target_link_libraries(testslirc libslirc)
target_link_libraries(testslirc ${Boost_LIBRARIES})
