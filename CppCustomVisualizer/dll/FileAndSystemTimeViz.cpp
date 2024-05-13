// Copyright (c) David Lowndes. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "stdafx.h"
#include <optional>
using std::optional;
#include "FileAndSystemTimeViz.h"
#include "TzInfoSystemTimeConverter.h"

static CString FormatSystemTime( const SYSTEMTIME& st )
{
    TCHAR szDate[50];

    // Use long date format to allow the day name to show if that's in that format. Short date formats don't include the day.
    const size_t NumCharsD = GetDateFormat( GetThreadLocale(), DATE_LONGDATE, &st, NULL, szDate, _countof( szDate ) );

    if ( NumCharsD == 0 )
	{
		// Normally GetLastError is ERROR_INVALID_PARAMETER indicating that the SYSTEMTIME invalid.

        // Date is invalid, so clear the buffer
        szDate[0] = _T('\0');
	}

    TCHAR szTime[50];

    const auto NumCharsT = GetTimeFormat( GetThreadLocale(), TIME_FORCE24HOURFORMAT, &st, NULL, szTime, _countof( szTime ) );

    if ( NumCharsT == 0 )
    {
        // Failed to get the time, so clear the buffer
        szTime[0] = _T('\0');
    }

    CString str;
    str.Format( _T( "%s %s" ), szDate, szTime );

    return str;
}

CString FileTimeToText( const FILETIME& ftUtc, UINT nBase )
{
    CString text;

    bool bValidFileTime{ false };

    // convert to SystemTime
    SYSTEMTIME SysTime;

    if ( FileTimeToSystemTime( &ftUtc, &SysTime ) )
    {
        const CString strUtc = FormatSystemTime( SysTime );

        SYSTEMTIME lst;

        // Even though FileTimeToSystemTime succeeds, and FormatSystemTime can
        // return something, SystemTimeToTzSpecificLocalTime can fail if the
        // time is invalid. So check the return value.
        if ( SystemTimeToTzSpecificLocalTime( NULL, &SysTime, &lst ) )
        {
            const CString strLoc = FormatSystemTime( lst );

            // Determine if there's a difference between local & utc so that I can display the most appropriate zone name
            LONGLONG diffInMins;
            {
                FILETIME lft;
                SystemTimeToFileTime( &lst, &lft );
                LONGLONG diffInTicks = reinterpret_cast<LARGE_INTEGER*>(&lft)->QuadPart - reinterpret_cast<const LARGE_INTEGER*>(&ftUtc)->QuadPart;
                diffInMins = diffInTicks / (10'000'000LL * 60);
            }

            TIME_ZONE_INFORMATION tzi;
            GetTimeZoneInformation( &tzi );

            // Remove the current bias from UTC and if it's now non-zero, its the DST zone
            diffInMins += tzi.Bias;
            LPCWSTR ZoneName = diffInMins != 0 ? tzi.DaylightName : tzi.StandardName;

			text.Format( _T( "[utc] %s [%ls] %s" ), static_cast<LPCTSTR>(strUtc), ZoneName, static_cast<LPCTSTR>(strLoc) );

            bValidFileTime = true;
        }
    }

    if ( !bValidFileTime )
    {
        // Prior to VS2017 RC2, VS only ever passed 10 for nBase, newer versions should support base values of 2, 8, 10, or 16
        char szHigh[33];	// 32 chars max for base 2 from a DWORD value
        char szLow[33];
        _ui64toa_s( ftUtc.dwHighDateTime, szHigh, _countof( szHigh ), nBase );
        _ui64toa_s( ftUtc.dwLowDateTime, szLow, _countof( szLow ), nBase );

        const char* Prefix;
        switch ( nBase )
        {
        case 2:
            Prefix = "0b";	// binary
            break;
        case 8:
            Prefix = "0";	// octal
            break;
        case 10:
            Prefix = "";	// decimal
            break;
        case 16:
            Prefix = "0x";	// hex
            break;
        default:
            Prefix = "?";	// unknown
            break;
        }

        text.Format( _T( "Invalid [%hs%hs:%hs%hs]" ), Prefix, szHigh, Prefix, szLow );
    }
    return text;
}

optional<CString> SystemTimeToVisualizerFormattedString( const SYSTEMTIME & st, UINT Radix )
{
    optional<CString> sRet;

    // If the SYSTEMTIME has year 0, it's probably of the form described here for the StandardDate and DaylightDate forms:
    // https://docs.microsoft.com/en-us/windows/win32/api/timezoneapi/ns-timezoneapi-time_zone_information
    // The other conditions preclude the following code from showing something daft
	if ( (st.wYear == 0) && (st.wDay != 0) && (st.wMonth >= 1) && (st.wMonth <= 12) )
    {
        // Show this like: "Wednesday, [Last] week [1] of March" along with a computed actual date of when that is for the current year.
        // See https://docs.microsoft.com/en-us/windows/win32/intl/locale-smonthname-constants for information on the max lengths of these
        TCHAR szMonthName[80];
        GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_SMONTHNAME1 + st.wMonth -1, szMonthName, _countof(szMonthName) );
        TCHAR szDayName[80];
        // Days are not in the same order as they are for wDayOfWeek! Mon = 1, Tue = 2, Sun = 7
        // Trick to re-order them
        auto ReorderedDayOfWeek = (st.wDayOfWeek + 6) % 7;
        GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_SDAYNAME1 + ReorderedDayOfWeek, szDayName, _countof( szDayName ) );

        CString str;

        // 1...4 are week numbers, 5 means last week of the month
        if ( st.wDay < 5 )
        {
            str.Format( _T( "%s, week %d of %s" ), szDayName, st.wDay, szMonthName );
        }
        else
        {
            str.Format( _T( "%s, Last week of %s" ), szDayName, szMonthName );
        }

        // Convert to a prospective real date time and if we can, show that in addition
        SYSTEMTIME stNow;
        GetSystemTime( &stNow );
        const auto stExample = TzInfoSystemTimeToSystemTimeForYear( st, stNow.wYear );

        if ( stExample.has_value() )
        {
            auto fmtd = SystemTimeToVisualizerFormattedString( *stExample, Radix );

            if ( fmtd.has_value() )
            {
                // Append the real example
                str += _T( "; e.g. " );
                str += *fmtd;
            }
        }

        sRet = str;
    }
    else
    {
        FILETIME ft;
        if ( SystemTimeToFileTime( &st, &ft ) )
        {
            sRet = FileTimeToText( ft, Radix );
        }
    }

    return sRet;
}
