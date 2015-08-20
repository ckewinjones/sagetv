/*
 * Copyright 2015 The SageTV Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __FILTERRIPERTIES_H__
#define __FILTERRIPERTIES_H__
#include "ITSSPlitter.h"

#define RETURN_FALSE_IF_FAILED(tsz,hr)          \
{   if( S_OK != hr)                             \
    {                                           \
        TCHAR dbgsup_tszDump[1024];             \
        wsprintf(dbgsup_tszDump, (tsz), (hr));  \
        OutputDebugString(dbgsup_tszDump);      \
        return FALSE;                           \
    }                                           \
}

#define SAFE_RELEASE(pObject) if(pObject){ pObject->Release(); pObject = NULL;}

class CFilterProperties : public CBasePropertyPage
{
public:

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);
    CFilterProperties(LPUNKNOWN lpunk, HRESULT *phr);
	
	void OnTimer( DWORD count );

private:
    BOOL OnReceiveMessage( HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam );
    HRESULT OnConnect(IUnknown *pUnknown);
    HRESULT OnDisconnect();
    HRESULT OnDeactivate();
    HRESULT OnApplyChanges();
	void OnBtnScanChannel( int nType  );
	void OnBtnChannelList( );
	void OnBtnScanDone( );
    void SetDirty();
    BOOL OnUpdate();

	void CFilterProperties::DisplayInfo();

	ITSParser2  *m_pTSSplitter;
	//IGraphBuilder * m_pGraphBuilder;

	void EnableATSCInput();
	void EnableDVBInput();
	HWND m_hwndDialog;

	BOOL m_bScanInProgress;
};

#endif
