/********************************************************************
Vireio Perception: Open-Source Stereoscopic 3D Driver
Copyright (C) 2012 Andres Hernandez

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************/

#include "D3DProxyDeviceSimultaneous.h"

D3DProxyDeviceSimultaneous::D3DProxyDeviceSimultaneous(IDirect3DDevice9* pDevice) : D3DProxyDevice(pDevice)
{
	// Proxy is created after actual Device (pDevice parameter) has been created with actual CreateDevice and before
	// the proxy CreateDevice has returned

	// Here we need to duplicate the default render target (the backbuffer)

}

D3DProxyDeviceSimultaneous::~D3DProxyDeviceSimultaneous()
{
}

void D3DProxyDeviceSimultaneous::Init(ProxyHelper::ProxyConfig& cfg)
{
	OutputDebugString("D3D ProxyDev Simultaneous Init\n");
	D3DProxyDevice::Init(cfg);
}

HRESULT WINAPI D3DProxyDeviceSimultaneous::BeginScene()
{
	// Notes:
	// Save currect state here and reapply before drawing the second eye. (state changes may occur before begin and during the scene so we need to make sure we have the correct initial state)
	// Move camera to first eye position - if we take the default position to be the center of the two eyes then offset each eye to either side we maintain the "correct" center
	// (could move eyes forward as well to provide very basic head model for pitch and yaw)
	// Have a look at how Viewports and Surfaces are used currently in StereoView




	HandleControls();
//	HandleTracking();
	ComputeViewTranslation();

	return D3DProxyDevice::BeginScene();
}

HRESULT WINAPI D3DProxyDeviceSimultaneous::EndScene()
{
	// Notes:
	// End scene, load saved initial state, change render targets/viewports/whatever to other eye, move camera to position of 2nd eye.



	// delay to avoid crashing on start
	static int initDelay = 120;
	initDelay--;

	if(!stereoView->initialized && initDelay < 0)
	{
		stereoView->Init(m_pDevice);
		SetupMatrices();
	}

	return D3DProxyDevice::EndScene();
}

HRESULT WINAPI D3DProxyDeviceSimultaneous::Present(CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion)
{
	HandleTracking();

	if(stereoView->initialized && stereoView->stereoEnabled)
	{
		if(eyeShutter > 0)
		{
			stereoView->UpdateEye(StereoView::LEFT_EYE);
		}
		else 
		{
			stereoView->UpdateEye(StereoView::RIGHT_EYE);
		}

		stereoView->Draw();

		eyeShutter *= -1;
	}

	if(eyeShutter > 0) return D3D_OK;
	HRESULT hr = D3DProxyDevice::Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
	return hr;
}

HRESULT WINAPI D3DProxyDeviceSimultaneous::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix)
{
	if(State == D3DTS_VIEW)
	{
		D3DXMATRIX sourceMatrix(*pMatrix);

		D3DXMATRIX transMatrix;
		D3DXMatrixIdentity(&transMatrix);

		if(trackerInitialized && tracker->isAvailable())
		{
			D3DXMATRIX rollMatrix;
			D3DXMatrixRotationZ(&rollMatrix, tracker->currentRoll);
			D3DXMatrixMultiply(&sourceMatrix, &sourceMatrix, &rollMatrix);
		}

		D3DXMatrixTranslation(&transMatrix, (separation*eyeShutter+offset)*6000.0f, 0, 0);
		D3DXMatrixMultiply(&sourceMatrix, &sourceMatrix, &transMatrix);

		return D3DProxyDevice::SetTransform(State, &sourceMatrix);
	}
	else if(State == D3DTS_PROJECTION)
	{
		D3DXMATRIX sourceMatrix(*pMatrix);

		D3DXMATRIX transMatrix;
		D3DXMatrixIdentity(&transMatrix);

		D3DXMatrixMultiply(&sourceMatrix, &sourceMatrix, &matProjectionInv);

		transMatrix[8] += convergence*eyeShutter*0.0075f;
		D3DXMatrixMultiply(&sourceMatrix, &sourceMatrix, &transMatrix);

		D3DXMatrixMultiply(&sourceMatrix, &sourceMatrix, &matProjection);

		return D3DProxyDevice::SetTransform(State, &sourceMatrix);
	}

	return D3DProxyDevice::SetTransform(State, pMatrix);
}