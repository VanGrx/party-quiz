cmake_minimum_required(VERSION 3.5)

project(PartyQuiz LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)


find_package(Boost 1.70.0)
find_package(CURL)
find_package(RapidJSON)


include_directories(${Boost_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS} src/)

set(include_files
    src/server.h
    src/listener.h
    src/session.h
    src/utils.h
    src/pages.h
    src/game.h
    src/question.h
    src/player.h
    src/callbacks.h
    src/randomgenerator.h
    )

set(source_files
    src/main.cpp
    src/server.cpp
    src/listener.cpp
    src/session.cpp
    src/utils.cpp
    src/game.cpp
    src/question.cpp
    src/player.cpp
    src/randomgenerator.cpp
    )



add_executable(PartyQuiz ${source_files} ${include_files})

target_compile_options(PartyQuiz PRIVATE -Wall -Wextra -pedantic -Werror)

target_link_libraries(PartyQuiz ${Boost_LIBRARIES} ${CURL_LIBRARIES} Threads::Threads)
