
add_executable(search
    search.cpp
)

target_include_directories(search
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${lockedhash_SOURCE_DIR}/include
)

add_executable(update
    update.cpp
)