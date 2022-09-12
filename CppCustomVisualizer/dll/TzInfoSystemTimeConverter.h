#pragma once
//#include <Windows.h>
#include <optional>

extern std::optional<SYSTEMTIME> TzInfoSystemTimeToSystemTimeForYear( const SYSTEMTIME& stPartial, const WORD Year );
