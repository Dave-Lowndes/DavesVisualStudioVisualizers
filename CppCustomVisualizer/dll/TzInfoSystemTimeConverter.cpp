#include "stdafx.h"
#include <Windows.h>
#include "TzInfoSystemTimeConverter.h"
#include <optional>

using std::optional;

/// <summary>
/// Convert the SYSTEMTIME to a FILETIME and back in order to get all members consistent - specifically the day of week value
/// </summary>
/// <param name="st">The SYSTEMTIME value to make "consistent" (fully populated)</param>
/// <returns>true if all OK</returns>
static bool MakeSystemTimeConsistent( SYSTEMTIME& st )
{
    FILETIME ft;
    const bool bRV = SystemTimeToFileTime( &st, &ft ) && FileTimeToSystemTime( &ft, &st );
    // I don't expect this to be called with some time that would fail
    _ASSERT( bRV );
    return bRV;
}

constexpr static void OffsetFileTime( FILETIME& ft, UINT64 offsetIn100nsUnits )
{
    ULARGE_INTEGER FileTimeAsInteger{ .LowPart = ft.dwLowDateTime, .HighPart = ft.dwHighDateTime };

    FileTimeAsInteger.QuadPart += offsetIn100nsUnits;

    ft.dwLowDateTime = FileTimeAsInteger.LowPart;
    ft.dwHighDateTime = FileTimeAsInteger.HighPart;
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
            auto StartOfMonthWeekday = stFirstOfMonth.wDayOfWeek;

            WORD offsetDays = reqdWeekDay < StartOfMonthWeekday ?
                                reqdWeekDay - StartOfMonthWeekday + 7 :
                                reqdWeekDay - StartOfMonthWeekday;

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

				// Previous day is the last day of the month we want to go back 1 day
				if ( SystemTimeToFileTime( &st, &ft ) )
				{
					OffsetFileTime( ft, static_cast<UINT64>(-OneDay) );

					// Back to SYSTEMTIME
					if ( FileTimeToSystemTime( &ft, &st ) )
					{
						// Now have the day of the week for the end of the month
						const auto dowEndOfMonth = st.wDayOfWeek;

						const WORD offsetDays = reqdWeekDay <= dowEndOfMonth ?
							                        dowEndOfMonth - reqdWeekDay :
							                        dowEndOfMonth - reqdWeekDay + 7;

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
    // Shouldn't get here
    _ASSERT( FALSE );
    return {};
}
