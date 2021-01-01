#pragma once

#include <optional>

//extern CString FormatSystemTime( const SYSTEMTIME& st );
extern CString FileTimeToText( const FILETIME& ftUtc, UINT nBase );
std::optional<CString> SystemTimeToVisualizerFormattedString( const SYSTEMTIME& st, UINT Radix );
