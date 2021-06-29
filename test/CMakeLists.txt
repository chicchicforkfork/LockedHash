include(FetchContent)
FetchContent_Declare(gtest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.10.0
)
FetchContent_MakeAvailable(gtest)

add_executable(lockedhash_unit_test
    test_lockedhash.cpp
    test_lockedhash_keypair.cpp)

target_include_directories(lockedhash_unit_test
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${lockedhash_SOURCE_DIR}/include
)

target_link_libraries(lockedhash_unit_test
PRIVATE
    gtest_main
    pthread
)
