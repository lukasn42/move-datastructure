cmake_minimum_required(VERSION "3.16.3")

set(This Tests)
set(Sources
    move_datastructure_tests.cpp
    avl_tree_tests.cpp
    dl_list_tests.cpp
)
add_executable(${This} ${Sources})
target_link_libraries(${This} PUBLIC
    gtest_main
    move_datastructure
)
add_test(
    NAME ${This}
    COMMAND ${This}
)