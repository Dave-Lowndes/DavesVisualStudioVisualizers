// Copyright (c) David Lowndes. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "stdafx.h"
#include <optional>
using std::optional;
#include "FileAndSystemTimeViz.h"

static CString FormatSystemTime( const SYSTEMTIME& st )
{
    TCHAR szDate[50];

    size_t NumChars = GetDateFormat( GetThreadLocale(), DATE_SHORTDATE, &st, NULL, szDate, _countof( szDate ) );

    if ( NumChars == 0 )
    {
        // Failed
        szDate[0] = '\0';
    }

    TCHAR szTime[50];

    NumChars = GetTimeFormat( GetThreadLocale(), TIME_FORCE24HOURFORMAT, &st, NULL, szTime, _countof( szTime ) );

    if ( NumChars == 0 )
    {
        // Failed
        szTime[0] = '\0';
    }

    CString str;
    str = szDate;
    str += _T( ' ' );
    str += szTime;

    // If the string isn't more than the separator something's wrong, return an empty string
    if ( str.GetLength() <= 1 )
    {
        str.Empty();
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
                diffInMins = diffInTicks / (10000000 * 60);
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

optional<CString> SystemTimeToVisualizerFormattedString( const SYSTEMTIME& st, UINT Radix )
{
    optional<CString> sRet;

    FILETIME ft;
    if ( SystemTimeToFileTime( &st, &ft ) )
    {
        sRet = FileTimeToText( ft, Radix );
    }

    return sRet;
}
