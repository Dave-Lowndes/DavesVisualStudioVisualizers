// Copyright (c) David Lowndes. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// _CppCustomVisualizerService.cpp : Implementation of CCppCustomVisualizerService

#include "stdafx.h"
#include "_EntryPoint.h"
#include "FileAndSystemTimeViz.h"
#include "PropertyKeyViz.h"
//#include <ctime>
#include <ATLComTime.h>

#define INITGUID
#include <initguid.h>

// These GUID values are referenced in the .natvis file where the're associated with the named type
// {34A6191C-7D6D-4B36-9C3F-A4D051289DEB}
DEFINE_GUID( FILETIME_VIZ_ID, 0x34a6191c, 0x7d6d, 0x4b36, 0x9c, 0x3f, 0xa4, 0xd0, 0x51, 0x28, 0x9d, 0xeb );

// {1AC4A977-2954-4316-8CE6-A6251EEBA144}
DEFINE_GUID( SYSTEMTIME_VIZ_ID, 0x1ac4a977, 0x2954, 0x4316, 0x8c, 0xe6, 0xa6, 0x25, 0x1e, 0xeb, 0xa1, 0x44 );

// {47D01893-3FA1-4326-9ED2-5236246CF231}
DEFINE_GUID( PROPKEY_VIZ_ID, 0x47d01893, 0x3fa1, 0x4326, 0x9e, 0xd2, 0x52, 0x36, 0x24, 0x6c, 0xf2, 0x31 );

// {C7B69925-4654-434D-A657-631AAB40BB82}
//DEFINE_GUID( TIME_T_VIZ_ID, 0xc7b69925, 0x4654, 0x434d, 0xa6, 0x57, 0x63, 0x1a, 0xab, 0x40, 0xbb, 0x82 );

// {C27383FF-1889-4959-A0E4-99DB41295CB3}
DEFINE_GUID( COleDateTime_VIZ_ID,
    0xc27383ff, 0x1889, 0x4959, 0xa0, 0xe4, 0x99, 0xdb, 0x41, 0x29, 0x5c, 0xb3 );

HRESULT STDMETHODCALLTYPE CCppCustomVisualizerService::EvaluateVisualizedExpression(
    _In_ Evaluation::DkmVisualizedExpression* pVisualizedExpression,
    _Deref_out_opt_ Evaluation::DkmEvaluationResult** ppResultObject
    )
{
    HRESULT hr;

    // This method is called to visualize one of the variable types this extension handles. Its basic job is to create
    // a DkmEvaluationResult object. A DkmEvaluationResult is the data that backs a row in the
    // watch window -- a name, value, and type, a flag indicating if the item can be expanded, and
    // lots of other additional properties.

    Evaluation::DkmPointerValueHome* pPointerValueHome = Evaluation::DkmPointerValueHome::TryCast(pVisualizedExpression->ValueHome());
    if (pPointerValueHome == nullptr)
    {
        // This sample only handles visualizing in-memory structures
        return E_NOTIMPL;
    }

    DkmRootVisualizedExpression* pRootVisualizedExpression = DkmRootVisualizedExpression::TryCast(pVisualizedExpression);
    if (pRootVisualizedExpression == nullptr)
    {
        // This sample doesn't provide child evaluation results, so only root expressions are expected
        return E_NOTIMPL;
    }

    DkmProcess* pTargetProcess = pVisualizedExpression->RuntimeInstance()->Process();
    CString strValue;

    const auto & vizId = pVisualizedExpression->VisualizerId();

    if ( vizId == FILETIME_VIZ_ID )
    {
        // Read the FILETIME value from the target process
        FILETIME value;
        hr = pTargetProcess->ReadMemory( pPointerValueHome->Address(), DkmReadMemoryFlags::None, &value, sizeof( value ), nullptr );
        if ( FAILED( hr ) )
        {
            // If the bytes of the value cannot be read from the target process, just fall back to the default visualization
            return E_NOTIMPL;
        }

        // Format this FILETIME as a string
        strValue = FileTimeToText( value, pRootVisualizedExpression->InspectionContext()->Radix() );
    }
    else if ( vizId == SYSTEMTIME_VIZ_ID )
    {
        SYSTEMTIME value;
        hr = pTargetProcess->ReadMemory( pPointerValueHome->Address(), DkmReadMemoryFlags::None, &value, sizeof( value ), nullptr );
        if ( FAILED( hr ) )
        {
            // If the bytes of the value cannot be read from the target process, just fall back to the default visualization
            return E_NOTIMPL;
        }

        // Convert to FILETIME (UTC) to pass to the string conversion that shows both UTC & local
        FILETIME ft;
        if ( SystemTimeToFileTime( &value, &ft ) )
        {
            strValue = FileTimeToText( ft, pRootVisualizedExpression->InspectionContext()->Radix() );
        }

        // An empty returned string indicates an invalid SYSTEMTIME
        if ( strValue.IsEmpty() )
        {
            strValue = _T( "Invalid" );
        }
    }
    else if ( vizId == PROPKEY_VIZ_ID )
    {
        // Read the PROPERTYKEY value from the target process
        PROPERTYKEY value;
        hr = pTargetProcess->ReadMemory( pPointerValueHome->Address(), DkmReadMemoryFlags::None, &value, sizeof( value ), nullptr );
        if ( FAILED( hr ) )
        {
            // If the bytes of the value cannot be read from the target process, just fall back to the default visualization
            return E_NOTIMPL;
        }

        // Format as a string
        strValue = GetKeyName( value );
    }
#if 0 //Ineffective, no unique struct definition
    else if ( vizId == TIME_T_VIZ_ID)
    {
        // Read the time_t value from the target process
        time_t value;
        hr = pTargetProcess->ReadMemory( pPointerValueHome->Address(), DkmReadMemoryFlags::None, &value, sizeof( value ), nullptr );
        if ( FAILED( hr ) )
        {
            // If the bytes of the value cannot be read from the target process, just fall back to the default visualization
            return E_NOTIMPL;
        }

        // Format as a string
        {
            tm tmm;
            gmtime_s( &tmm, &value );
            char buffer[100];
            asctime_s( buffer, &tmm );
            strValue = buffer;
        }
    }
#endif
    else if ( vizId == COleDateTime_VIZ_ID )
    {
        /*ATL::*/COleDateTime value;

        hr = pTargetProcess->ReadMemory( pPointerValueHome->Address(), DkmReadMemoryFlags::None, &value, sizeof( value ), nullptr );
        if ( FAILED( hr ) )
        {
            // If the bytes of the value cannot be read from the target process, just fall back to the default visualization
            return E_NOTIMPL;
        }

        // Convert -> SYSTEMTIME -> FILETIME in order to use the FILETIME string output function
        SYSTEMTIME st;
        if ( value.GetAsSystemTime( st ) )
        {
            // Convert to FILETIME (UTC) to pass to the string conversion that shows both UTC & local
            FILETIME ft;
            if ( SystemTimeToFileTime( &st, &ft ) )
            {
                strValue = FileTimeToText( ft, pRootVisualizedExpression->InspectionContext()->Radix() );
            }
        }

        // An empty returned string indicates an invalid SYSTEMTIME
        if ( strValue.IsEmpty() )
        {
            // Note: If the COleDateTime is invalid, this returns appropriate text
            strValue = value.Format();
        }
    }
    else
    {
        // Don't know what it's for
        return E_NOTIMPL;
    }

    CString strEditableValue;

    auto pType = pRootVisualizedExpression->Type();

    // If we are formatting a pointer, we want to also show the address of the pointer
    if ( ( pType != nullptr ) && ( wcschr( pType->Value(), '*') != nullptr ) )
    {
        // Make the editable value just the pointer string
        UINT64 address = pPointerValueHome->Address();
        if ((pTargetProcess->SystemInformation()->Flags()& DefaultPort::DkmSystemInformationFlags::Is64Bit) != 0)
        {
            strEditableValue.Format(L"0x%08x%08x", static_cast<DWORD>(address >> 32), static_cast<DWORD>(address));
        }
        else
        {
            strEditableValue.Format(L"0x%08x", static_cast<DWORD>(address));
        }

        // Prefix the value with the address
        CString strValueWithAddress;
        strValueWithAddress.Format(L"%s {%s}", static_cast<LPCWSTR>(strEditableValue), static_cast<LPCWSTR>(strValue));
        strValue = strValueWithAddress;
    }

    CComPtr<DkmString> pValue;
    hr = DkmString::Create(DkmSourceString(strValue), &pValue);
    if (FAILED(hr))
    {
        return hr;
    }

    CComPtr<DkmString> pEditableValue;
    hr = DkmString::Create(strEditableValue, &pEditableValue);
    if (FAILED(hr))
    {
        return hr;
    }

    CComPtr<DkmDataAddress> pAddress;
    hr = DkmDataAddress::Create(pVisualizedExpression->RuntimeInstance(), pPointerValueHome->Address(), nullptr, &pAddress);
    if (FAILED(hr))
    {
        return hr;
    }

    DkmEvaluationResultFlags_t resultFlags = DkmEvaluationResultFlags::Expandable;
    if (strEditableValue.IsEmpty())
    {
        // We only allow editting pointers, so mark non-pointers as read-only
        resultFlags |= DkmEvaluationResultFlags::ReadOnly;
    }

    CComPtr<DkmSuccessEvaluationResult> pSuccessEvaluationResult;
    hr = DkmSuccessEvaluationResult::Create(
        pVisualizedExpression->InspectionContext(),
        pVisualizedExpression->StackFrame(),
        pRootVisualizedExpression->Name(),
        pRootVisualizedExpression->FullName(),
        resultFlags,
        pValue,
        pEditableValue,
        pRootVisualizedExpression->Type(),
        DkmEvaluationResultCategory::Class,
        DkmEvaluationResultAccessType::None,
        DkmEvaluationResultStorageType::None,
        DkmEvaluationResultTypeModifierFlags::None,
        pAddress,
        nullptr,
        (DkmReadOnlyCollection<DkmModuleInstance*>*)nullptr,
        // This sample doesn't need to store any state associated with this evaluation result, so we
        // pass `DkmDataItem::Null()` here. A more complicated extension which had associated
        // state such as an extension which took over expansion of evaluation results would likely
        // create an instance of the extension's data item class and pass the instance here.
        // More information: https://github.com/Microsoft/ConcordExtensibilitySamples/wiki/Data-Container-API
        DkmDataItem::Null(),
        &pSuccessEvaluationResult
        );
    if (FAILED(hr))
    {
        return hr;
    }

    *ppResultObject = pSuccessEvaluationResult.Detach();
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CCppCustomVisualizerService::UseDefaultEvaluationBehavior(
    _In_ Evaluation::DkmVisualizedExpression* pVisualizedExpression,
    _Out_ bool* pUseDefaultEvaluationBehavior,
    _Deref_out_opt_ Evaluation::DkmEvaluationResult** ppDefaultEvaluationResult
    )
{
    HRESULT hr;

    // This method is called by the expression evaluator when a visualized expression's children are
    // being expanded, or the value is being set. We just want to delegate this back to the C++ EE.
    // So we need to set `*pUseDefaultEvaluationBehavior` to true and return the evaluation result which would
    // be created if this custom visualizer didn't exist.
    //
    // NOTE: If this custom visualizer supported underlying strings (no DkmEvaluationResultFlags::RawString),
    // this method would also be called when that is requested.

    DkmRootVisualizedExpression* pRootVisualizedExpression = DkmRootVisualizedExpression::TryCast(pVisualizedExpression);
    if (pRootVisualizedExpression == nullptr)
    {
        // This sample doesn't provide child evaluation results, so only root expressions are expected
        return E_NOTIMPL;
    }

    DkmInspectionContext* pParentInspectionContext = pVisualizedExpression->InspectionContext();

    CAutoDkmClosePtr<DkmLanguageExpression> pLanguageExpression;
    hr = DkmLanguageExpression::Create(
        pParentInspectionContext->Language(),
        DkmEvaluationFlags::TreatAsExpression,
        pRootVisualizedExpression->FullName(),
        DkmDataItem::Null(),
        &pLanguageExpression
        );
    if (FAILED(hr))
    {
        return hr;
    }

    // Create a new inspection context with 'DkmEvaluationFlags::ShowValueRaw' set. This is important because
    // the result of the expression is a FILETIME, and we don't want our visualizer to be invoked again. This
    // step would be unnecessary if we were evaluating a different expression that resulted in a type which
    // we didn't visualize.
    CComPtr<DkmInspectionContext> pInspectionContext;
    if (DkmComponentManager::IsApiVersionSupported(DkmApiVersion::VS16RTMPreview))
    {
        // If we are running in VS 16 or newer, use this overload...
        hr = DkmInspectionContext::Create(
            pParentInspectionContext->InspectionSession(),
            pParentInspectionContext->RuntimeInstance(),
            pParentInspectionContext->Thread(),
            pParentInspectionContext->Timeout(),
            DkmEvaluationFlags::TreatAsExpression |
            DkmEvaluationFlags::ShowValueRaw,
            pParentInspectionContext->FuncEvalFlags(),
            pParentInspectionContext->Radix(),
            pParentInspectionContext->Language(),
            pParentInspectionContext->ReturnValue(),
            (Evaluation::DkmCompiledVisualizationData*)nullptr,
            Evaluation::DkmCompiledVisualizationDataPriority::None,
            pParentInspectionContext->ReturnValues(),
            pParentInspectionContext->SymbolsConnection(),
            &pInspectionContext
            );
    }
    else
    {
        // Otherwise fall back to the Visual Studio 14 version
        hr = DkmInspectionContext::Create(
            pParentInspectionContext->InspectionSession(),
            pParentInspectionContext->RuntimeInstance(),
            pParentInspectionContext->Thread(),
            pParentInspectionContext->Timeout(),
            DkmEvaluationFlags::TreatAsExpression |
            DkmEvaluationFlags::ShowValueRaw,
            pParentInspectionContext->FuncEvalFlags(),
            pParentInspectionContext->Radix(),
            pParentInspectionContext->Language(),
            pParentInspectionContext->ReturnValue(),
            (Evaluation::DkmCompiledVisualizationData*)nullptr,
            Evaluation::DkmCompiledVisualizationDataPriority::None,
            pParentInspectionContext->ReturnValues(),
            &pInspectionContext
            );
    }
    if (FAILED(hr))
    {
        return hr;
    }

    CComPtr<DkmEvaluationResult> pEEEvaluationResult;
    hr = pVisualizedExpression->EvaluateExpressionCallback(
                                    pInspectionContext,
                                    pLanguageExpression,
                                    pVisualizedExpression->StackFrame(),
                                    &pEEEvaluationResult
                                    );
    if (FAILED(hr))
    {
        return hr;
    }

    *ppDefaultEvaluationResult = pEEEvaluationResult.Detach();
    *pUseDefaultEvaluationBehavior = true;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CCppCustomVisualizerService::GetChildren(
    _In_ Evaluation::DkmVisualizedExpression* /*pVisualizedExpression*/,
    _In_ UINT32 /*InitialRequestSize*/,
    _In_ Evaluation::DkmInspectionContext* /*pInspectionContext*/,
    _Out_ DkmArray<Evaluation::DkmChildVisualizedExpression*>* /*pInitialChildren*/,
    _Deref_out_ Evaluation::DkmEvaluationResultEnumContext** /*ppEnumContext*/
    )
{
    // This sample delegates expansion to the C++ EE, so this method doesn't need to be implemented
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CCppCustomVisualizerService::GetItems(
    _In_ Evaluation::DkmVisualizedExpression* /*pVisualizedExpression*/,
    _In_ Evaluation::DkmEvaluationResultEnumContext* /*pEnumContext*/,
    _In_ UINT32 /*StartIndex*/,
    _In_ UINT32 /*Count*/,
    _Out_ DkmArray<Evaluation::DkmChildVisualizedExpression*>* /*pItems*/
    )
{
    // This sample delegates expansion to the C++ EE, so this method doesn't need to be implemented
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CCppCustomVisualizerService::SetValueAsString(
    _In_ Evaluation::DkmVisualizedExpression* /*pVisualizedExpression*/,
    _In_ DkmString* /*pValue*/,
    _In_ UINT32 /*Timeout*/,
    _Deref_out_opt_ DkmString** /*ppErrorText*/
    )
{
    // This sample delegates setting values to the C++ EE, so this method doesn't need to be implemented
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CCppCustomVisualizerService::GetUnderlyingString(
    _In_ Evaluation::DkmVisualizedExpression* /*pVisualizedExpression*/,
    _Deref_out_opt_ DkmString** /*ppStringValue*/
    )
{
    // FILETIME doesn't have an underlying string (no DkmEvaluationResultFlags::RawString), so this method
    // doesn't need to be implemented
    return E_NOTIMPL;
}

