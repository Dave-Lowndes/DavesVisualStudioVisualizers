// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// TargetApp.cpp : Example application which will be debuggeed

#include <iostream>
#include <windows.h>
#include <propkey.h>

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


int wmain(int argc, WCHAR* argv[])
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

	PROPERTYKEY key;

	// BREAK HERE TO SEE THE INVALID REPRESENTATION OF PROPERTYKEY
	key = PKEY_ApplicationName;

	MyClass myC(ft, key);

    FILETIME FTZero = {};

    __debugbreak(); // program will stop here. Evaluate 'myC', key, st, ft, lst in the debugtip, locals or watch windows.
    std::cout << "Test complete\n";

    return 0;
}

