// Copyright (c) David Lowndes. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "stdafx.h"
#include "FontInfoViz.h"

static CString FontWeightAsNamedString( LONG weight )
{
    struct FWtoName
    {
        LPCTSTR Name;   //-V122
        LONG weight;
    };

    static const FWtoName fws[] =
    {
     _T( "FW_DONTCARE" ),   0
    ,_T( "FW_THIN" ),       100
    ,_T( "FW_EXTRALIGHT" ), 200
    ,_T( "FW_LIGHT" ),      300
    ,_T( "FW_NORMAL" ),     400
    ,_T( "FW_MEDIUM" ),     500
    ,_T( "FW_SEMIBOLD" ),   600
    ,_T( "FW_BOLD" ),       700
    ,_T( "FW_EXTRABOLD" ),  800
    ,_T( "FW_HEAVY" ),      900
    };

    if ( weight >= 0 )
    {
        for ( auto& fw : fws )
        {
            if ( weight <= fw.weight )
            {
                return fw.Name;
            }
        }
    }
    // An illegal value - return the number
    CString str;
    str.Format( _T( "Weight: %d" ), weight );
    return str;
}

static CString CharSetAsNamedString( BYTE csvalue )
{
    struct CharSetNameValue
    {
        LPCTSTR Name;   //-V122
        BYTE value;
    };
    static const CharSetNameValue csnvs[] =
    {
         _T( "ANSI_CHARSET" ),        0
        ,_T( "DEFAULT_CHARSET" ),    1
        ,_T( "SYMBOL_CHARSET" ),     2
        ,_T( "SHIFTJIS_CHARSET" ),   128
        ,_T( "HANGEUL_CHARSET" ),    129 //,_T("HANGUL_CHARSET" ),     129
        ,_T( "GB2312_CHARSET" ),     134
        ,_T( "CHINESEBIG5_CHARSET" ), 136
        ,_T( "OEM_CHARSET" ),        255
        ,_T( "JOHAB_CHARSET" ),      130
        ,_T( "HEBREW_CHARSET" ),     177
        ,_T( "ARABIC_CHARSET" ),     178
        ,_T( "GREEK_CHARSET" ),      161
        ,_T( "TURKISH_CHARSET" ),    162
        ,_T( "VIETNAMESE_CHARSET" ), 163
        ,_T( "THAI_CHARSET" ),       222
        ,_T( "EASTEUROPE_CHARSET" ), 238
        ,_T( "RUSSIAN_CHARSET" ),    204
        ,_T( "MAC_CHARSET" ),        77
        ,_T( "BALTIC_CHARSET" ),     186
    };

    for ( auto& item : csnvs )
    {
        if ( csvalue == item.value )
        {
            return item.Name;
        }
    }
    // An unknown value - return the number
    CString str;
    str.Format( _T( "CharSet: %d" ), csvalue );
    return str;

}

// This function only uses the LOGFONT members that don't vary between
// Unicode/MBS, so it's perfectly safe to pass either LOGFONTA or LOGFONTW
CString CommonLFToVizString( const LOGFONT& lf )
{
    CString str;
    str.Format( _T( "Height: %d %s %s %s %s %s" )
        , lf.lfHeight
        , static_cast<LPCTSTR>(FontWeightAsNamedString( lf.lfWeight ))
        , static_cast<LPCTSTR>(CharSetAsNamedString( lf.lfCharSet ))
        , lf.lfItalic ? _T( "Italic" ) : _T( "" )
        , lf.lfUnderline ? _T( "Underline" ) : _T( "" )
        , lf.lfStrikeOut ? _T( "StrikeOut" ) : _T( "" )
    );
    return str;
}

