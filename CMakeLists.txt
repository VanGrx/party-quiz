cmake_minimum_required(VERSION 3.5)

project(PartyQuiz LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#All dependencies we need
find_package(Boost 1.83.0)
find_package(CURL)
find_package(RapidJSON)

include_directories(${Boost_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS} src/server src/game)

set(server_include_files
    src/server/server.h
    src/server/http_session.h
    src/server/websocket_session.h
    src/server/utils.h
    src/server/pages.h
    src/server/callbacks.h
    )

set(server_source_files
    src/main.cpp
    src/server/server.cpp
    src/server/http_session.cpp
    src/server/websocket_session.cpp
    src/server/utils.cpp
    )

set(game_include_files
    src/game/randomgenerator.h
    src/game/game.h
    src/game/question.h
    src/game/player.h
    src/game/gameconfig.h
    src/game/questionsgenerator.h
    )

set(game_source_files
    src/game/game.cpp
    src/game/question.cpp
    src/game/player.cpp
    src/game/randomgenerator.cpp
    src/game/questionsgenerator.cpp
    )

add_executable(PartyQuiz
  src/main.cpp
  ${server_include_files}
  ${server_source_files}
  ${game_include_files}
  ${game_source_files}
  )


if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  set(WARNING_FLAGS "-W4")
else()
  set(WARNING_FLAGS "-Wall" "-pedantic" "-Werror" "-Wextra")
endif()

target_compile_options(PartyQuiz PRIVATE ${WARNING_FLAGS})

target_link_libraries(PartyQuiz ${Boost_LIBRARIES} ${CURL_LIBRARIES})
