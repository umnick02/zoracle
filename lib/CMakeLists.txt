# SPDX-License-Identifier: Apache-2.0

add_subdirectory_ifdef(CONFIG_CUSTOM_LIB custom_lib)
add_library(sbus STATIC sbus/sbus.cpp)
target_link_libraries(sbus PRIVATE app)
