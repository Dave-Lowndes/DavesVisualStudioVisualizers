// Copyright (c) David Lowndes. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// TargetApp.cpp : Example application which will be debugged

#include <iostream>
#include <windows.h>
#include <propkey.h>
#include <tchar.h>
#include <ATLComTime.h>

class MyClass
{
    const FILETIME m_fileTime;
	const PROPERTYKEY m_key;

public:
    MyClass(const FILETIME& ft, const PROPERTYKEY & key) :
        m_fileTime(ft),
        m_key(key)
    {
    }
};

void DemoLOGFONTAW()
{
	HGDIOBJ hf = GetStockObject( DEFAULT_GUI_FONT );

	LOGFONTA lfa;
	LOGFONTW lfw;

	GetObjectA( hf, sizeof( lfa ), &lfa );
	GetObjectW( hf, sizeof( lfw ), &lfw );
}

void DemoLOGFONT()
{
	LOGFONT lf;

	// If you break here, the LOGFONT object will be uninitialized
	// (not guaranteed in a non-debug build)
	GetObject( GetStockObject( DEFAULT_GUI_FONT ), sizeof( lf ), &lf );

	// Here it's show the default values

	lf.lfCharSet = EASTEUROPE_CHARSET;
	lf.lfItalic = TRUE;
	lf.lfStrikeOut = TRUE;
	lf.lfUnderline = TRUE;
	lf.lfWeight = FW_SEMIBOLD;

	// And here you should see the amended characteristics
}

void DemoTimes2()
{
	// Break here and the COleDateTime will be uninitialized
	COleDateTime dt{ COleDateTime::GetCurrentTime() };

	// Here it will show the current date/time

	// Create one in summer time
	COleDateTime dtSummer( dt.GetYear(), 6, 16, 1, 2, 3 );

	// Another in winter time
	COleDateTime dtWinter( dt.GetYear(), 12, 25, 4, 5, 6 );

	CTimeSpan tspan( 1, 2, 3, 4 );
	CString str = dt.Format();

	FILETIME ft;
	GetSystemTimeAsFileTime( &ft );
	CTime tim( ft );
}

void DemoPropKey()
{
	PROPERTYKEY key;

	// BREAK HERE TO SEE THE INVALID REPRESENTATION OF PROPERTYKEY
	key = PKEY_ApplicationName;

	// Here you should see the populated key name
}

void DemoTimes1()
{
	FILETIME ft;
	SYSTEMTIME st;

	// BREAK HERE TO SEE THE INVALID REPRESENTATION OF FILETIME & SYSTEMTIME

	st.wYear = 2016;
	st.wMonth = 6;
	st.wDay = 16;
	st.wHour = 13;
	st.wMinute = 14;
	st.wSecond = 15;
	st.wMilliseconds = 0;

	SystemTimeToFileTime( &st, &ft );

	// Now get the local time
	SYSTEMTIME lst;
	SystemTimeToTzSpecificLocalTime( nullptr, &st, &lst );
	FILETIME lft;
	SystemTimeToFileTime( &lst, &lft );

	// Demonstrate the partial form of SYSTEMTIME
	SYSTEMTIME stPartial{};
	stPartial.wHour = 1;
	stPartial.wDayOfWeek = 0;	// Sunday
	stPartial.wDay = 5;			// Last week of the month
	stPartial.wMonth = 3;		// March

	// At this point the partial SYSTEMTIME is of the form obtained from the GetTimeZoneInformationForYear API.
	stPartial = stPartial;
}

int wmain(int argc, WCHAR* argv[])
{
	// Step into the following functions and examine the appropriate variables
	// to see the visualizers in action.
	__debugbreak();

	DemoLOGFONT();
	DemoLOGFONTAW();
	DemoTimes1();
	DemoTimes2();
	DemoPropKey();

	FILETIME FTZero = {};

	// Check things work when members of a class
	MyClass myC(FTZero, PKEY_ApplicationName );

    __debugbreak(); // program will stop here. Evaluate 'myC' in the debug tip, locals or watch windows.

    return 0;
}

