// Copyright (c) David Lowndes. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "stdafx.h"
#include <optional>
using std::optional;
#include "FileAndSystemTimeViz.h"

static CString FormatSystemTime( const SYSTEMTIME& st )
{
    TCHAR szDate[50];

    // Use long date format to allow the day name to show if that's in that format. Short date formats don't include the day.
    size_t NumCharsD = GetDateFormat( GetThreadLocale(), DATE_LONGDATE, &st, NULL, szDate, _countof( szDate ) );

    TCHAR szTime[50];

    auto NumCharsT = GetTimeFormat( GetThreadLocale(), TIME_FORCE24HOURFORMAT, &st, NULL, szTime, _countof( szTime ) );

    CString str;

    // If one of these isn't there, something's wrong, return an empty string
    if ( (NumCharsD != 0) || (NumCharsT != 0) )
    {
        str = szDate;
        str += _T( ' ' );
        str += szTime;
    }

    return str;
}

CString FileTimeToText( const FILETIME& ftUtc, UINT nBase )
{
    CString text;

    // convert to SystemTime
    SYSTEMTIME SysTime;

    if ( FileTimeToSystemTime( &ftUtc, &SysTime ) )
    {
        CString strUtc = FormatSystemTime( SysTime );

        SYSTEMTIME lst;

        if ( SystemTimeToTzSpecificLocalTime( NULL, &SysTime, &lst ) )
        {
            CString strLoc = FormatSystemTime( lst );

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
        }
        else
        {
            text.Format( _T( "utc: %s ???" ), static_cast<LPCTSTR>(strUtc) );
        }
    }
    else
    {
        // Prior to VS2017 RC2, VS only ever passed 10 for nBase, newer versions should support base values of 2, 8, 10, or 16
        char szHigh[33];	// 32 chars max for base 2 from a DWORD value
        char szLow[33];
        _ui64toa_s( ftUtc.dwHighDateTime, szHigh, _countof( szHigh ), nBase );
        _ui64toa_s( ftUtc.dwLowDateTime, szLow, _countof( szLow ), nBase );

        char* Prefix;
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
        //        sprintf_s( pResult, BufferMax, );
    }
    return text;
}

#pragma region TzInfoSystemTimeToSystemTimeForYear
// Convert the SYSTEMTIME to a FILETIME and back in order to get all members consistent - specifically the day of week value
static BOOL MakeSystemTimeConsistent( SYSTEMTIME & st )
{
    BOOL bRV = FALSE;
    FILETIME ft;
    if ( SystemTimeToFileTime( &st, &ft ) )
    {
        if ( FileTimeToSystemTime( &ft, &st ) )
        {
            bRV = TRUE;
        }
    }
    // I don't expect this to be called with some time that would fail
    _ASSERT( bRV );
    return bRV;
}

static void OffsetFileTime( FILETIME& ft, UINT64 offsetIn100nsUnits )
{
    ULARGE_INTEGER ulft;
    ulft.LowPart = ft.dwLowDateTime;
    ulft.HighPart = ft.dwHighDateTime;

    ulft.QuadPart += offsetIn100nsUnits;

    ft.dwLowDateTime = ulft.LowPart;
    ft.dwHighDateTime = ulft.HighPart;
}

constexpr INT64 OneDay = 24ull * 60 * 60 * 1000 * 1000 * 10;
constexpr UINT64 ThirtyOneDays = 31ull * OneDay;

static optional<SYSTEMTIME> TzInfoSystemTimeToSystemTimeForYear( const SYSTEMTIME & stPartial, const WORD Year )
{
    // Preserve these here for clarity of 
    const auto reqdWeekDay = stPartial.wDayOfWeek;
    const auto WeekNum = stPartial.wDay;

    // Take a copy, set the year & day to the start of the month
    SYSTEMTIME stFirstOfMonth{ stPartial };
    stFirstOfMonth.wYear = Year;
    stFirstOfMonth.wDay = 1;

    // Values 1...4 are week numbers, 5 means the last week in a month
    if ( WeekNum < 5 )
    {
        // Convert 1'st of month back and forth to get the weekday of the start of the month
        if ( MakeSystemTimeConsistent( stFirstOfMonth ) )
        {
            // Now have the start of the month weekday
            auto somwd = stFirstOfMonth.wDayOfWeek;

            WORD offsetDays;
            if ( reqdWeekDay < somwd )
            {
                offsetDays = reqdWeekDay - somwd + 7;
            }
            else
            {
                offsetDays = reqdWeekDay - somwd;
            }

            // offset by the weeks
            offsetDays += 7 * (WeekNum - 1);

            // The first day of the month for the desired weekday
            SYSTEMTIME st{ stFirstOfMonth };
            st.wDay += offsetDays;

            // Ensure the week day is correct for the return value
            if ( MakeSystemTimeConsistent( st ) )
            {
                return st;
            }
        }
    }
    else
    {
        // Get the last day of the month by going back 1 day from the 1'st day of the next month

        // First get the FILETIME for the 1'st of this month
        FILETIME ft;
        if ( SystemTimeToFileTime( &stFirstOfMonth, &ft ) )
        {
            // Increment the date by 31 days to ensure its in the next month
            OffsetFileTime( ft, ThirtyOneDays );

            // Convert back to SYSTEMTIME
            SYSTEMTIME st;
            if ( FileTimeToSystemTime( &ft, &st ) )
            {
                // We now want the start of this next month
                st.wDay = 1;

                if ( MakeSystemTimeConsistent( st ) )
                {
                    // Previous day is the last day of the month we want to go back 1 day
                    if ( SystemTimeToFileTime( &st, &ft ) )
                    {
                        OffsetFileTime( ft, static_cast<UINT64>(-OneDay) );

                        // Back to SYSTEMTIME
                        if ( FileTimeToSystemTime( &ft, &st ) )
                        {
                            // Now have the day of the week for the end of the month
                            const auto dowEndOfMonth = st.wDayOfWeek;

                            WORD offsetDays;
                            if ( reqdWeekDay <= dowEndOfMonth )
                            {
                                offsetDays = dowEndOfMonth - reqdWeekDay;
                            }
                            else
                            {
                                offsetDays = dowEndOfMonth - reqdWeekDay + 7;
                            }

                            st.wDay -= offsetDays;

                            // Ensure the week day is correct for the return value
                            if ( MakeSystemTimeConsistent( st ) )
                            {
                                return st;
                            }
                        }
                    }
                }
            }
        }
    }
    // Shouldn't get here
    _ASSERT( FALSE );
    return {};
}
#pragma endregion TzInfoSystemTimeToSystemTimeForYear

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
