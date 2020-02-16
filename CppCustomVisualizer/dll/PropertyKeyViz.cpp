#include "stdafx.h"
#include "PropertyKeyViz.h"
#include <shobjidl.h>   // SHGetPropertyStoreFromParsingName, etc
#pragma comment( lib, "propsys.lib" )

CStringW GetKeyName( const PROPERTYKEY& Key )
{
    CStringW sRet;

    CComPtr<IPropertyDescription> pPropDesc;

    HRESULT hr = PSGetPropertyDescription( Key, IID_PPV_ARGS( &pPropDesc ) );
    if ( SUCCEEDED( hr ) )
    {
        CComHeapPtr<wchar_t> pCName;
        hr = pPropDesc->GetCanonicalName( &pCName );
        if ( SUCCEEDED( hr ) )
        {
            sRet = pCName;

            CComHeapPtr<wchar_t> pDName;
            hr = pPropDesc->GetDisplayName( &pDName );
            if ( SUCCEEDED( hr ) )
            {
                sRet += L" \"";
                sRet += pDName;
                sRet += L"\"";
            }
            else
            {
                sRet += L" ***No Display Name***";
            }
        }
        else
        {
            sRet = L"Failed to get name info";
        }
    }
    else
    {
        sRet = L"Invalid";
    }

    return sRet;
}
