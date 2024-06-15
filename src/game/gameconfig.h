#ifndef UTILS_H
#define UTILS_H

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <cstdint>

// ---------------------------------------------------------------------------------------
// Game relative
constexpr unsigned int POINTS_FOR_CORRECT_ANSWER = 10;
constexpr unsigned int MAX_ID = 1000000;

constexpr uint8_t ROUND_TIME = 15;
constexpr uint8_t PAUSE_TIME = 5;

#endif // UTILS_H
