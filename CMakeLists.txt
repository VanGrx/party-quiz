cmake_minimum_required(VERSION 3.5)

project(PartyQuiz LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


#Enable threads
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

#All dependencies we need
find_package(Boost 1.70.0)
find_package(CURL)
find_package(RapidJSON)


include_directories(${Boost_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS} src/server src/game)

set(server_include_files
    src/server/server.h
    src/server/listener.h
    src/server/session.h
    src/server/utils.h
    src/server/pages.h
    src/server/callbacks.h
    )

set(server_source_files
    src/main.cpp
    src/server/server.cpp
    src/server/listener.cpp
    src/server/session.cpp
    src/server/utils.cpp
    )


set(game_include_files
    src/game/randomgenerator.h
    src/game/game.h
    src/game/question.h
    src/game/player.h
    )

set(game_source_files
    src/main.cpp
    src/game/game.cpp
    src/game/question.cpp
    src/game/player.cpp
    src/game/randomgenerator.cpp
    )


add_executable(PartyQuiz
  src/main.cpp
  ${server_include_files}
  ${server_source_files}
  ${game_include_files}
  ${game_source_files}
  )

target_compile_options(PartyQuiz PRIVATE -Wall -Wextra -pedantic -Werror)

target_link_libraries(PartyQuiz ${Boost_LIBRARIES} ${CURL_LIBRARIES} Threads::Threads)
