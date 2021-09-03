
include(FetchContent)

FetchContent_Declare(kairos
  GIT_REPOSITORY https://github.com/chicchicforkfork/Kairos.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(kairos)

add_executable(search
    search.cpp
)
add_executable(update
    update.cpp
)
add_executable(loop
    loop.cpp
)
add_executable(insert
    insert.cpp
)
add_executable(expire
    expire.cpp
)
add_executable(expire2
    expire2.cpp
)
add_executable(clear
    clear.cpp
)
target_link_libraries(clear
  kairos
)
add_executable(live
    live.cpp
)