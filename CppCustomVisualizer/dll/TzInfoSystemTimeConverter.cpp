#include "stdafx.h"
#include <Windows.h>
#include "TzInfoSystemTimeConverter.h"
#include <optional>

using std::optional;

// Convert the SYSTEMTIME to a FILETIME and back in order to get all members consistent - specifically the day of week value
static BOOL MakeSystemTimeConsistent( SYSTEMTIME& st )
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

optional<SYSTEMTIME> TzInfoSystemTimeToSystemTimeForYear( const SYSTEMTIME& stPartial, const WORD Year )
{
    // Copy these here for reuse and clarity of the code
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

//Pointless                if ( MakeSystemTimeConsistent( st ) )
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
