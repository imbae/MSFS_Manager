#pragma once

constexpr auto PI = 3.14159265359;
constexpr auto RAD_TO_DEG = 180 / PI;
constexpr auto DEG_TO_RAD = PI / 180;

constexpr auto M_TO_FEET = 3.28084;
constexpr auto MS_KNOT = 1.94384;

constexpr auto DELTA_T = 1.0 / 50.0;		// unit : sec
constexpr auto MAX_SPEED_PAN = 60.0;		// unit : deg/sec
constexpr auto MAX_SPEED_TILT = 30.0;		// unit : deg/sec
constexpr auto MAX_RANGE_TILT = 90.0;		// unit : deg
constexpr auto MIN_RANGE_TILT = -30.0;		// unit : deg