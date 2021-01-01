#pragma once

extern CString CommonLFToVizString( const LOGFONT& lf );

template< class T>
CString LfToVizString( T value )
{
    CString strValue;

    // The weight is defined to be between 0...1000, so use that as an
    // indication that the structure is uninitialised.
    if ( (value.lfWeight >= 0) && (value.lfWeight <= 1000) )
    {
        // Ensure the face name is null terminated for this purpose.
        value.lfFaceName[_countof( value.lfFaceName ) - 1] = '\0';

        strValue = value.lfFaceName;
        strValue += _T( "; " );

        strValue += CommonLFToVizString( *reinterpret_cast<LOGFONT*>(&value) );
    }
    else
    {
        strValue = _T( "Not initialised" );
    }

    return strValue;
}
