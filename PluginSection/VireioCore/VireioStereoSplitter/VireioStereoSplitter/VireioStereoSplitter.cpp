/********************************************************************
Vireio Perception: Open-Source Stereoscopic 3D Driver
Copyright (C) 2012 Andres Hernandez

Vireio Stereo Splitter - Vireio Perception Render Target Handler
Copyright (C) 2015 Denis Reischl

File <VireioStereoSplitter.cpp> and
Class <VireioStereoSplitter> :
Copyright (C) 2015 Denis Reischl

Parts of this class directly derive from Vireio source code originally
authored by Chris Drain (v1.1.x 2013).

The stub class <AQU_Nodus> is the only public class from the Aquilinus
repository and permitted to be used for open source plugins of any kind.
Read the Aquilinus documentation for further information.

Vireio Perception Version History:
v1.0.0 2012 by Andres Hernandez
v1.0.X 2013 by John Hicks, Neil Schneider
v1.1.x 2013 by Primary Coding Author: Chris Drain
Team Support: John Hicks, Phil Larkson, Neil Schneider
v2.0.x 2013 by Denis Reischl, Neil Schneider, Joshua Brown
v2.0.4 onwards 2014 by Grant Bagwell, Simon Brown and Neil Schneider

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

#define DEBUG_UINT(a) { wchar_t buf[128]; wsprintf(buf, L"%u", a); OutputDebugString(buf); }
#define DEBUG_HEX(a) { wchar_t buf[128]; wsprintf(buf, L"%x", a); OutputDebugString(buf); }
#define IS_RENDER_TARGET(d3dusage) ((d3dusage & D3DUSAGE_RENDERTARGET) > 0 ? true : false)

#include"VireioStereoSplitter.h"

#define INTERFACE_IDIRECT3DDEVICE9                           8
#define INTERFACE_IDIRECT3DSTATEBLOCK9                      13
#define INTERFACE_IDIRECT3DSWAPCHAIN9                       15
#define METHOD_IDIRECT3DDEVICE9_RESET                       16
#define	METHOD_IDIRECT3DDEVICE9_PRESENT                     17
#define METHOD_IDIRECT3DDEVICE9_GETBACKBUFFER               18 
#define METHOD_IDIRECT3DDEVICE9_CREATETEXTURE               23
#define METHOD_IDIRECT3DDEVICE9_CREATEVOLUMETEXTURE         24
#define METHOD_IDIRECT3DDEVICE9_CREATECUBETEXTURE           25
#define METHOD_IDIRECT3DDEVICE9_CREATEVERTEXBUFFER          26
#define METHOD_IDIRECT3DDEVICE9_CREATEINDEXBUFFER           27
#define METHOD_IDIRECT3DDEVICE9_CREATERENDERTARGET          28
#define METHOD_IDIRECT3DDEVICE9_CREATEDEPTHSTENCILSURFACE   29
#define METHOD_IDIRECT3DDEVICE9_UPDATESURFACE               30 
#define METHOD_IDIRECT3DDEVICE9_UPDATETEXTURE               31 
#define METHOD_IDIRECT3DDEVICE9_GETRENDERTARGETDATA         32
#define METHOD_IDIRECT3DDEVICE9_GETFRONTBUFFERDATA          33
#define METHOD_IDIRECT3DDEVICE9_STRETCHRECT                 34 
#define METHOD_IDIRECT3DDEVICE9_COLORFILL                   35 
#define METHOD_IDIRECT3DDEVICE9_SETRENDERTARGET             37
#define METHOD_IDIRECT3DDEVICE9_GETRENDERTARGET             38 
#define METHOD_IDIRECT3DDEVICE9_SETDEPTHSTENCILSURFACE      39
#define METHOD_IDIRECT3DDEVICE9_GETDEPTHSTENCILSURFACE      40 
#define METHOD_IDIRECT3DDEVICE9_BEGINSCENE                  41
#define METHOD_IDIRECT3DDEVICE9_ENDSCENE                    42
#define METHOD_IDIRECT3DDEVICE9_CLEAR                       43
#define METHOD_IDIRECT3DDEVICE9_SETVIEWPORT                 47
#define METHOD_IDIRECT3DDEVICE9_SETRENDERSTATE              57
#define METHOD_IDIRECT3DDEVICE9_GETTEXTURE                  64 
#define METHOD_IDIRECT3DDEVICE9_SETTEXTURE                  65
#define METHOD_IDIRECT3DDEVICE9_DRAWPRIMITIVE               81
#define METHOD_IDIRECT3DDEVICE9_DRAWINDEXEDPRIMITIVE        82 
#define METHOD_IDIRECT3DDEVICE9_DRAWPRIMITIVEUP             83 
#define METHOD_IDIRECT3DDEVICE9_DRAWINDEXEDPRIMITIVEUP      84
#define METHOD_IDIRECT3DDEVICE9_DRAWRECTPATCH              115 
#define METHOD_IDIRECT3DDEVICE9_DRAWTRIPATCH               116
#define	METHOD_IDIRECT3DSWAPCHAIN9_PRESENT                   3
#define	METHOD_IDIRECT3DSWAPCHAIN9_GETFRONTBUFFERDATA        4
#define	METHOD_IDIRECT3DSWAPCHAIN9_GETBACKBUFFER             5
#define METHOD_IDIRECT3DSTATEBLOCK9_APPLY                    5

/**
* Constructor.
***/
StereoSplitter::StereoSplitter() :AQU_Nodus(),
m_apcActiveRenderTargets(D3D9_SIMULTANEOUS_RENDER_TARGET_COUNT * 2, nullptr),
m_apcActiveTextures(D3D9_SIMULTANEAOUS_TEXTURE_COUNT * 2, nullptr),
m_hBitmapControl(nullptr),
m_bControlUpdate(false),
m_hFont(nullptr),
m_unTextureNumber(0),
m_unRenderTargetNumber(0),
m_bPresent(false),
m_bApply(true),
m_punRenderTargetIndex(nullptr),
m_ppcRenderTarget(nullptr),
m_ppcNewZStencil(nullptr),
m_punSampler(nullptr),
m_ppcTexture(nullptr),
m_ppSourceSurface(nullptr),
m_ppcSourceRect(nullptr),
m_ppcDestinationSurface(nullptr),
m_ppsDestPoint(nullptr),
m_ppcSourceTexture(nullptr),
m_ppcDestinationTexture(nullptr),
m_ppcSurface(nullptr),
m_ppsRect(nullptr),
m_punColor(nullptr),
m_ppcSourceSurface_StretchRect(nullptr),
m_ppcSourceRect_StretchRect(nullptr),
m_ppcDestSurface_StretchRect(nullptr),
m_ppcDestRect_StretchRect(nullptr),
m_peFilter_StretchRect(nullptr),
m_peDrawingSide(nullptr),
m_ppasVSConstantRuleIndices(nullptr),
m_ppasPSConstantRuleIndices(nullptr),
m_peState(nullptr),
m_punValue(nullptr),
m_ppsViewport(nullptr),
m_ppcRenderTargetGetData(nullptr),
m_punISwapChain(nullptr),
m_ppcDestSurface(nullptr),
m_punWidth(nullptr),
m_punHeight(nullptr),
m_punEdgeLength(nullptr),
m_punLevels(nullptr),
m_punDepth(nullptr),
m_punUsage(nullptr),
m_peFormat(nullptr),
m_peMultiSample(nullptr),
m_punMultisampleQuality(nullptr),
m_pnDiscard(nullptr),
m_pnLockable(nullptr),
m_pePool(nullptr),
m_ppcTextureCreate(nullptr),
m_ppcVolumeTexture(nullptr),
m_ppcCubeTexture(nullptr),
m_ppcSurfaceCreate(nullptr),
m_ppvSharedHandle(nullptr)
{
	m_pcActiveDepthStencilSurface[0] = nullptr;
	m_pcActiveDepthStencilSurface[1] = nullptr;
	m_pcActiveBackBufferSurface[0] = nullptr;
	m_pcActiveBackBufferSurface[1] = nullptr;
	m_pcStereoBuffer[0] = nullptr;
	m_pcStereoBuffer[1] = nullptr;

	// set to "LEFT" !!
	m_eCurrentRenderingSide = RenderPosition::Left;

	// first viewport set is default
	m_bActiveViewportIsDefault = true;

	// set default behaviour
	m_sGameSettings.nDuplicateRenderTarget = DetermineDuplicateRToDS::DT_TRUE;
	m_sGameSettings.nDuplicateDepthStencil = DetermineDuplicateRToDS::DT_WIDTH_UNEQUAL_HEIGHT;
	m_sGameSettings.nDuplicateTexture = DetermineDuplicateTexture::DT_IF_RENDER_TARGET;
	m_sGameSettings.nDuplicateCubeTexture = DetermineDuplicateCubeTexture::DT_IF_RENDER_TARGET;
	m_sGameSettings.nSaveRenderStatesID = DetermineSaveRenderStates::DT_STATE_BLOCK;

	// Vireio GUI is always null at begin... since in a compiled profile it is never used
	m_pcVireioGUI = nullptr;

	// use a D3D9Ex device ? only available by setting, false by constructor
	m_bUseD3D9Ex = false;
}

/**
* Destructor.
***/
StereoSplitter::~StereoSplitter()
{
	if (m_pcStereoBufferSurface[0])
	{
		m_pcStereoBufferSurface[0]->Release();
		m_pcStereoBufferSurface[0] = nullptr;
	}
	if (m_pcStereoBufferSurface[1])
	{
		m_pcStereoBufferSurface[1]->Release();
		m_pcStereoBufferSurface[1] = nullptr;
	}
	if (m_pcStereoBuffer[0])
	{
		m_pcStereoBuffer[0]->Release();
		m_pcStereoBuffer[0] = nullptr;
	}
	if (m_pcStereoBuffer[1])
	{
		m_pcStereoBuffer[1]->Release();
		m_pcStereoBuffer[1] = nullptr;
	}
}

/**
* Return the name of the  Stereo Splitter node.
***/
const char* StereoSplitter::GetNodeType()
{
	return "Stereo Splitter";
}

/**
* Returns a global unique identifier for the Stereo Splitter node.
***/
UINT StereoSplitter::GetNodeTypeId()
{
#define DEVELOPER_IDENTIFIER 2006
#define MY_PLUGIN_IDENTIFIER 64
	return ((DEVELOPER_IDENTIFIER << 16) + MY_PLUGIN_IDENTIFIER);
}

/**
* Returns the name of the category for the Stereo Splitter node.
***/
LPWSTR StereoSplitter::GetCategory()
{
	return L"Vireio Core";
}

/**
* Returns a logo to be used for the Stereo Splitter node.
***/
HBITMAP StereoSplitter::GetLogo()
{
	HMODULE hModule = GetModuleHandle(L"VireioStereoSplitter.dll");
	HBITMAP hBitmap = LoadBitmap(hModule, MAKEINTRESOURCE(IMG_LOGO01));
	return hBitmap;
}

/**
* Returns the updated control for the Stereo Splitter node.
* Allways return >nullptr< if there is no update for the control !!
***/
HBITMAP StereoSplitter::GetControl()
{
	// here we create the Vireio GUI.... if this runs under a compiled profile this method is never called
	if (!m_pcVireioGUI)
		CreateGUI();
	else
		return m_pcVireioGUI->GetGUI(false, true, false, false);

	return nullptr;
}

/**
* Node data size.
* 1) sizeof(GameSettings)
***/
DWORD StereoSplitter::GetSaveDataSize()
{
	DWORD dwSizeofData = sizeof(GameSettings);

	return dwSizeofData;
}

/**
* Save the data.
***/
char* StereoSplitter::GetSaveData(UINT* pdwSizeOfData)
{
	static std::stringstream acStream;
	acStream = std::stringstream();

	// write game config
	acStream.write((char*)&m_sGameSettings, sizeof(GameSettings));

	// set data size
	UINT unDataSize = (UINT)acStream.str().size();
	if (unDataSize > MAX_DATA_SIZE) unDataSize = 0;
	*pdwSizeOfData = unDataSize;

	// copy data
	memcpy(&m_acData[0], (void*)&acStream.str()[0], (size_t)unDataSize);

	return (char*)&m_acData[0];
}

/**
* Get node data from the profile file.
***/
void StereoSplitter::InitNodeData(char* pData, UINT dwSizeOfData)
{
	UINT dwDataOffset = 0;
	if (dwSizeOfData >= sizeof(GameSettings))
	{
		// copy the game settings data
		memcpy(&m_sGameSettings, pData, sizeof(GameSettings));
		dwDataOffset += sizeof(GameSettings);
	}
}

/**
* Provides the name of the requested commander.
***/
LPWSTR StereoSplitter::GetCommanderName(DWORD unCommanderIndex)
{
	switch ((STS_Commanders)unCommanderIndex)
	{
		case STS_Commanders::StereoTextureLeft:
			return L"Stereo Output Texture Left";
		case STS_Commanders::StereoTextureRight:
			return L"Stereo Output Texture Right";
	}

	return L"";
}

/**
* Provides the name of the requested decommander.
***/
LPWSTR StereoSplitter::GetDecommanderName(DWORD unDecommanderIndex)
{
	switch ((STS_Decommanders)unDecommanderIndex)
	{
		case STS_Decommanders::RenderTargetIndex:             /**< ->SetRenderTarget() render target index ***/
			return L"RenderTargetIndex";
		case STS_Decommanders::pRenderTarget:                 /**< ->SetRenderTarget() render target ***/
			return L"pRenderTarget";
		case STS_Decommanders::pNewZStencil:                  /**< ->SetDepthStencilSurface() stencil surface ***/
			return L"pNewZStencil";
		case STS_Decommanders::Sampler:                       /**< ->SetTexture() sampler index **/
			return L"Sampler";
		case STS_Decommanders::pTexture:                      /**< ->SetTexture() texture pointer ***/
			return L"pTexture";
		case pSourceSurface:
			return L"pSourceSurface";
		case pSourceRect:
			return L"pSourceRect";
		case pDestinationSurface:
			return L"pDestinationSurface";
		case pDestPoint:
			return L"pDestPoint";
		case pSourceTexture:
			return L"pSourceTexture";
		case pDestinationTexture:
			return L"pDestinationTexture";
		case pSurface:
			return L"pSurface";
		case pRect:
			return L"pRect";
		case color:
			return L"color";
		case pSourceSurface_StretchRect:
			return L"pSourceSurface_StretchRect";
		case pSourceRect_StretchRect:
			return L"pSourceRect_StretchRect";
		case pDestSurface_StretchRect:
			return L"pDestSurface_StretchRect";
		case pDestRect_StretchRect:
			return L"pDestRect_StretchRect";
		case Filter_StretchRect:
			return L"Filter_StretchRect";
		case peDrawingSide:
			return L"peDrawingSide";
		case pasVShaderConstantIndices:
			return L"pasVShaderConstantIndices";
		case pasPShaderConstantIndices:
			return L"pasPShaderConstantIndices";
		case State:
			return L"State";
		case Value:
			return L"Value";
		case pViewport:
			return L"pViewport";
		case pRenderTargetGetData:
			return L"pRenderTargetGetData";
		case iSwapChain:
			return L"iSwapChain";
		case pDestSurface:
			return L"pDestSurface";
		case Width:
			return L"Width";
		case Height:
			return L"Height";
		case EdgeLength:
			return L"EdgeLength";
		case Levels:
			return L"Levels";
		case Depth:
			return L"Depth";
		case Usage:
			return L"Usage";
		case Format:
			return L"Format";
		case MultiSample:
			return L"MultiSample";
		case MultisampleQuality:
			return L"MultisampleQuality";
		case Discard:
			return L"Discard";
		case Lockable:
			return L"Lockable";
		case Pool:
			return L"Pool";
		case ppTexture:
			return L"ppTexture";
		case ppVolumeTexture:
			return L"ppVolumeTexture";
		case ppCubeTexture:
			return L"ppCubeTexture";
		case ppSurface:
			return L"ppSurface";
		case pSharedHandle:
			return L"pSharedHandle";
		default:
			break;
	}

	return L"";
}

/**
* Returns the plug type for the requested commander.
***/
DWORD StereoSplitter::GetCommanderType(DWORD unCommanderIndex)
{
	return NOD_Plugtype::AQU_PNT_IDIRECT3DTEXTURE9;
}

/**
* Returns the plug type for the requested decommander.
***/
DWORD StereoSplitter::GetDecommanderType(DWORD unDecommanderIndex)
{
	switch ((STS_Decommanders)unDecommanderIndex)
	{
		case STS_Decommanders::RenderTargetIndex:             /**< ->SetRenderTarget() render target index ***/
			return NOD_Plugtype::AQU_UINT;
		case STS_Decommanders::pRenderTarget:                 /**< ->SetRenderTarget() render target ***/
			return NOD_Plugtype::AQU_PNT_IDIRECT3DSURFACE9;
		case STS_Decommanders::pNewZStencil:                  /**< ->SetDepthStencilSurface() stencil surface ***/
			return NOD_Plugtype::AQU_PNT_IDIRECT3DSURFACE9;
		case STS_Decommanders::Sampler:                       /**< ->SetTexture() sampler index **/
			return NOD_Plugtype::AQU_UINT;
		case STS_Decommanders::pTexture:                      /**< ->SetTexture() texture pointer ***/
			return NOD_Plugtype::AQU_PNT_IDIRECT3DBASETEXTURE9;
		case pSourceSurface:
			return NOD_Plugtype::AQU_PNT_IDIRECT3DSURFACE9;
		case pSourceRect:
			return NOD_Plugtype::AQU_PNT_RECT;
		case pDestinationSurface:
			return NOD_Plugtype::AQU_PNT_IDIRECT3DSURFACE9;
		case pDestPoint:
			return NOD_Plugtype::AQU_PNT_POINT;
		case pSourceTexture:
			return NOD_Plugtype::AQU_PNT_IDIRECT3DBASETEXTURE9;
		case pDestinationTexture:
			return NOD_Plugtype::AQU_PNT_IDIRECT3DBASETEXTURE9;
		case pSurface:
			return NOD_Plugtype::AQU_PNT_IDIRECT3DSURFACE9;
		case pRect:
			return NOD_Plugtype::AQU_PNT_RECT;
		case color:
			return NOD_Plugtype::AQU_D3DCOLOR;
		case pSourceSurface_StretchRect:
			return NOD_Plugtype::AQU_PNT_IDIRECT3DSURFACE9;
		case pSourceRect_StretchRect:
			return NOD_Plugtype::AQU_PNT_RECT;
		case pDestSurface_StretchRect:
			return NOD_Plugtype::AQU_PNT_IDIRECT3DSURFACE9;
		case pDestRect_StretchRect:
			return NOD_Plugtype::AQU_PNT_RECT;
		case Filter_StretchRect:
			return NOD_Plugtype::AQU_D3DTEXTUREFILTERTYPE;
		case peDrawingSide:
			return NOD_Plugtype::AQU_INT;
		case pasVShaderConstantIndices:
			return NOD_Plugtype::AQU_VOID;
		case pasPShaderConstantIndices:
			return NOD_Plugtype::AQU_VOID;
		case State:
			return NOD_Plugtype::AQU_D3DRENDERSTATETYPE;
		case Value:
			return NOD_Plugtype::AQU_UINT;
		case pViewport:
			return NOD_Plugtype::AQU_PNT_D3DVIEWPORT9;
		case pRenderTargetGetData:
			return NOD_Plugtype::AQU_PNT_IDIRECT3DSURFACE9;
		case iSwapChain:
			return NOD_Plugtype::AQU_UINT;
		case pDestSurface:
			return NOD_Plugtype::AQU_PNT_IDIRECT3DSURFACE9;
		case Width:
			return NOD_Plugtype::AQU_UINT;
		case Height:
			return NOD_Plugtype::AQU_UINT;
		case EdgeLength:
			return NOD_Plugtype::AQU_UINT;
		case Levels:
			return NOD_Plugtype::AQU_UINT;
		case Depth:
			return NOD_Plugtype::AQU_UINT;
		case Usage:
			return NOD_Plugtype::AQU_UINT;
		case Format:
			return NOD_Plugtype::AQU_UINT;
		case MultiSample:
			return NOD_Plugtype::AQU_D3DMULTISAMPLE_TYPE;
		case MultisampleQuality:
			return NOD_Plugtype::AQU_UINT;
		case Discard:
			return NOD_Plugtype::AQU_BOOL;
		case Lockable:
			return NOD_Plugtype::AQU_BOOL;
		case Pool:
			return NOD_Plugtype::AQU_D3DPOOL;
		case ppTexture:
			return NOD_Plugtype::AQU_PNT_IDIRECT3DTEXTURE9;
		case ppVolumeTexture:
			return NOD_Plugtype::AQU_PNT_IDIRECT3DVOLUMETEXTURE9;
		case ppCubeTexture:
			return NOD_Plugtype::AQU_PNT_IDIRECT3DCUBETEXTURE9;
		case ppSurface:
			return NOD_Plugtype::AQU_PNT_IDIRECT3DSURFACE9;
		case pSharedHandle:
			return NOD_Plugtype::AQU_PNT_HANDLE;
	}

	return 0;
}

/**
* Provides the output pointer for the requested commander.
***/
void* StereoSplitter::GetOutputPointer(DWORD unCommanderIndex)
{
	switch ((STS_Commanders)unCommanderIndex)
	{
		case STS_Commanders::StereoTextureLeft:
			return (void*)&m_pcStereoBuffer[0];
		case STS_Commanders::StereoTextureRight:
			return (void*)&m_pcStereoBuffer[1];
	}

	return nullptr;
}

/**
* Sets the input pointer for the requested decommander.
***/
void StereoSplitter::SetInputPointer(DWORD unDecommanderIndex, void* pData)
{
	switch ((STS_Decommanders)unDecommanderIndex)
	{
		case STS_Decommanders::RenderTargetIndex:                        /**< ->SetRenderTarget() render target index ***/
			m_punRenderTargetIndex = (DWORD*)pData;
			break;
		case STS_Decommanders::pRenderTarget:                            /**< ->SetRenderTarget() render target ***/
			m_ppcRenderTarget = (IDirect3DSurface9**)pData;
			break;
		case STS_Decommanders::pNewZStencil:                             /**< ->SetDepthStencilSurface() stencil surface ***/
			m_ppcNewZStencil = (IDirect3DSurface9**)pData;
			break;
		case STS_Decommanders::Sampler:                                  /**< ->SetTexture() sampler index **/
			m_punSampler = (DWORD*)pData;
			break;
		case STS_Decommanders::pTexture:                                 /**< ->SetTexture() texture pointer ***/
			m_ppcTexture = (IDirect3DTexture9**)pData;
			break;
		case pSourceSurface:
			m_ppSourceSurface = (IDirect3DSurface9**)pData;              /**< ->UpdateSurface() source surface ***/
			break;
		case pSourceRect:
			m_ppcSourceRect = (RECT**)pData;                             /**< ->UpdateSurface() source rectangle ***/
			break;
		case pDestinationSurface:
			m_ppcDestinationSurface = (IDirect3DSurface9**)pData;        /**< ->UpdateSurface() destination surface ***/
			break;
		case pDestPoint:
			m_ppsDestPoint = (POINT**)pData;                             /**< ->UpdateSurface() destination point ***/
			break;
		case pSourceTexture:
			m_ppcSourceTexture = (IDirect3DBaseTexture9**)pData;         /**< ->UpdateTexture() source texture ***/
			break;
		case pDestinationTexture:
			m_ppcDestinationTexture = (IDirect3DBaseTexture9**)pData;    /**< ->UpdateTexture() destination texture ***/
			break;
		case pSurface:
			m_ppcSurface = (IDirect3DSurface9**)pData;                   /**< ->ColorFill() surface pointer ***/
			break;
		case pRect:
			m_ppsRect = (RECT**)pData;                                   /**< ->ColorFill() destination rectangle ***/
			break;
		case color:
			m_punColor = (D3DCOLOR*)pData;                               /**< ->ColorFill() destination color ***/
			break;
		case pSourceSurface_StretchRect:
			m_ppcSourceSurface_StretchRect = (IDirect3DSurface9**)pData; /**< ->StretchRect() source surface ***/
			break;
		case pSourceRect_StretchRect:
			m_ppcSourceRect_StretchRect = (RECT**)pData;                 /**< ->StretchRect() source rectangle ***/
			break;
		case pDestSurface_StretchRect:
			m_ppcDestSurface_StretchRect = (IDirect3DSurface9**)pData;   /**< ->StretchRect() destination surface ***/
			break;
		case pDestRect_StretchRect:
			m_ppcDestRect_StretchRect = (RECT**)pData;                   /**< ->StretchRect() destination rectangle ***/
			break;
		case Filter_StretchRect:
			m_peFilter_StretchRect = (D3DTEXTUREFILTERTYPE*)pData;       /**< ->StretchRect() filter ***/
			break;
		case peDrawingSide:
			m_peDrawingSide = (RenderPosition*)pData;
			break;
		case pasVShaderConstantIndices:
			m_ppasVSConstantRuleIndices = (std::vector<Vireio_Constant_Rule_Index_DX9>**)pData;
			break;
		case pasPShaderConstantIndices:
			m_ppasPSConstantRuleIndices = (std::vector<Vireio_Constant_Rule_Index_DX9>**)pData;
			break;
		case State:
			m_peState = (D3DRENDERSTATETYPE*)pData;
			break;
		case Value:
			m_punValue = (DWORD*)pData;
			break;
		case pViewport:
			m_ppsViewport = (D3DVIEWPORT9**)pData;
			break;
		case pRenderTargetGetData:
			m_ppcRenderTargetGetData = (IDirect3DSurface9**)pData; /**< ->GetRenderTargetData() */
			break;
		case iSwapChain:
			m_punISwapChain = (UINT*)pData;                        /**< ->GetFrontBufferData() */
			break;
		case pDestSurface:
			m_ppcDestSurface = (IDirect3DSurface9**)pData;         /**< ->GetRenderTargetData() + GetFrontBufferData() */
			break;
		case Width:
			m_punWidth = (UINT*)pData;                             /**< ->CreateTexture(), CreateVolumeTexture(), CreateRenderTarget(), CreateDepthStencilSurface(), CreateOffscreenPlainSurface() **/
			break;
		case Height:
			m_punHeight = (UINT*)pData;                            /**< ->CreateTexture(), CreateVolumeTexture(), CreateRenderTarget(), CreateDepthStencilSurface(), CreateOffscreenPlainSurface() **/
			break;
		case EdgeLength:
			m_punEdgeLength = (UINT*)pData;                        /**< ->CreateCubeTexture() **/
			break;
		case Levels:
			m_punLevels = (UINT*)pData;                            /**< ->CreateTexture(), CreateVolumeTexture(), CreateCubeTexture() **/
			break;
		case Depth:
			m_punDepth = (UINT*)pData;                             /**< ->CreateVolumeTexture() **/
			break;
		case Usage:
			m_punUsage = (DWORD*)pData;                            /**< ->CreateTexture(), CreateVolumeTexture(), CreateCubeTexture() **/
			break;
		case Format:
			m_peFormat = (D3DFORMAT*)pData;                        /**< ->CreateTexture(), CreateVolumeTexture(), CreateCubeTexture(), CreateRenderTarget(), CreateDepthStencilSurface(), CreateOffscreenPlainSurface() **/
			break;
		case MultiSample:
			m_peMultiSample = (D3DMULTISAMPLE_TYPE*)pData;         /**< ->CreateRenderTarget(), CreateDepthStencilSurface() **/
			break;
		case MultisampleQuality:
			m_punMultisampleQuality = (DWORD*)pData;               /**< ->CreateRenderTarget(), CreateDepthStencilSurface() **/
			break;
		case Discard:
			m_pnDiscard = (BOOL*)pData;                            /**< ->CreateDepthStencilSurface() **/
			break;
		case Lockable:
			m_pnLockable = (BOOL*)pData;                           /**< ->CreateRenderTarget() **/
			break;
		case Pool:
			m_pePool = (D3DPOOL*)pData;                            /**< ->CreateTexture(), CreateVolumeTexture(), CreateCubeTexture(), CreateOffscreenPlainSurface() **/
			break;
		case ppTexture:
			m_ppcTextureCreate = (IDirect3DTexture9**)pData;       /**< ->CreateTexture() **/
			break;
		case ppVolumeTexture:
			m_ppcVolumeTexture = (IDirect3DVolumeTexture9**)pData; /**< ->CreateVolumeTexture() **/
			break;
		case ppCubeTexture:
			m_ppcCubeTexture = (IDirect3DCubeTexture9**)pData;     /**< ->CreateCubeTexture() **/
			break;
		case ppSurface:
			m_ppcSurfaceCreate = (IDirect3DSurface9**)pData;       /**< ->CreateRenderTarget(), CreateDepthStencilSurface(), CreateOffscreenPlainSurface() **/
			break;
		case pSharedHandle:
			m_ppvSharedHandle = (HANDLE**)pData;
			break;
		default:
			break;
	}
}

/**
* Stereo Splitter supports various D3D9 calls.
***/
bool StereoSplitter::SupportsD3DMethod(int nD3DVersion, int nD3DInterface, int nD3DMethod)
{
	if ((nD3DVersion >= (int)AQU_DirectXVersion::DirectX_9_0) &&
		(nD3DVersion <= (int)AQU_DirectXVersion::DirectX_9_29))
	{
		if (nD3DInterface == INTERFACE_IDIRECT3DDEVICE9)
		{
			if ((nD3DMethod == METHOD_IDIRECT3DDEVICE9_PRESENT) ||
				(nD3DMethod == METHOD_IDIRECT3DDEVICE9_SETRENDERTARGET) ||
				(nD3DMethod == METHOD_IDIRECT3DDEVICE9_SETDEPTHSTENCILSURFACE) ||
				(nD3DMethod == METHOD_IDIRECT3DDEVICE9_CLEAR) ||
				(nD3DMethod == METHOD_IDIRECT3DDEVICE9_SETTEXTURE) ||
				(nD3DMethod == METHOD_IDIRECT3DDEVICE9_DRAWPRIMITIVE) ||
				(nD3DMethod == METHOD_IDIRECT3DDEVICE9_DRAWINDEXEDPRIMITIVE) ||
				(nD3DMethod == METHOD_IDIRECT3DDEVICE9_DRAWPRIMITIVEUP) ||
				(nD3DMethod == METHOD_IDIRECT3DDEVICE9_DRAWINDEXEDPRIMITIVEUP) ||
				(nD3DMethod == METHOD_IDIRECT3DDEVICE9_GETBACKBUFFER) ||
				(nD3DMethod == METHOD_IDIRECT3DDEVICE9_UPDATESURFACE) ||
				(nD3DMethod == METHOD_IDIRECT3DDEVICE9_UPDATETEXTURE) ||
				(nD3DMethod == METHOD_IDIRECT3DDEVICE9_STRETCHRECT) ||
				(nD3DMethod == METHOD_IDIRECT3DDEVICE9_COLORFILL) ||
				(nD3DMethod == METHOD_IDIRECT3DDEVICE9_GETRENDERTARGET) ||
				(nD3DMethod == METHOD_IDIRECT3DDEVICE9_GETDEPTHSTENCILSURFACE) ||
				(nD3DMethod == METHOD_IDIRECT3DDEVICE9_GETTEXTURE) ||
				(nD3DMethod == METHOD_IDIRECT3DDEVICE9_RESET) ||
				(nD3DMethod == METHOD_IDIRECT3DDEVICE9_BEGINSCENE) ||
				(nD3DMethod == METHOD_IDIRECT3DDEVICE9_ENDSCENE) ||
				(nD3DMethod == METHOD_IDIRECT3DDEVICE9_DRAWRECTPATCH) ||
				(nD3DMethod == METHOD_IDIRECT3DDEVICE9_DRAWTRIPATCH) ||
				(nD3DMethod == METHOD_IDIRECT3DDEVICE9_SETRENDERSTATE) ||
				(nD3DMethod == METHOD_IDIRECT3DDEVICE9_SETVIEWPORT) ||
				(nD3DMethod == METHOD_IDIRECT3DDEVICE9_CREATETEXTURE) ||
				(nD3DMethod == METHOD_IDIRECT3DDEVICE9_CREATEVOLUMETEXTURE) ||
				(nD3DMethod == METHOD_IDIRECT3DDEVICE9_CREATECUBETEXTURE) ||
				(nD3DMethod == METHOD_IDIRECT3DDEVICE9_CREATEVERTEXBUFFER) ||
				(nD3DMethod == METHOD_IDIRECT3DDEVICE9_CREATEINDEXBUFFER) ||
				(nD3DMethod == METHOD_IDIRECT3DDEVICE9_CREATERENDERTARGET) ||
				(nD3DMethod == METHOD_IDIRECT3DDEVICE9_CREATEDEPTHSTENCILSURFACE) ||
				(nD3DMethod == METHOD_IDIRECT3DDEVICE9_GETRENDERTARGETDATA) ||
				(nD3DMethod == METHOD_IDIRECT3DDEVICE9_GETFRONTBUFFERDATA))
				return true;
		}
		else if (nD3DInterface == INTERFACE_IDIRECT3DSWAPCHAIN9)
		{
			if ((nD3DMethod == METHOD_IDIRECT3DSWAPCHAIN9_PRESENT) ||
				(nD3DMethod == METHOD_IDIRECT3DSWAPCHAIN9_GETBACKBUFFER) ||
				(nD3DMethod == METHOD_IDIRECT3DSWAPCHAIN9_GETFRONTBUFFERDATA)) return true;
		}
		else if (nD3DInterface == INTERFACE_IDIRECT3DSTATEBLOCK9)
		{
			if (nD3DMethod == METHOD_IDIRECT3DSTATEBLOCK9_APPLY) return true;
		}
	}
	return false;
}

/**
* Handle Stereo Render Targets (+Depth Buffers).
***/
void* StereoSplitter::Provoke(void* pThis, int eD3D, int eD3DInterface, int eD3DMethod, DWORD unNumberConnected, int& nProvokerIndex)
{

#ifdef _DEBUG_STEREO_SPLITTER
	{ wchar_t buf[128]; wsprintf(buf, L"[STS] if %u mt %u", eD3DInterface, eD3DMethod); OutputDebugString(buf); }
#endif

	static HRESULT nHr = S_OK;

	if (m_bPresent)
		nProvokerIndex |= AQU_PluginFlags::DoubleCallFlag;

	switch (eD3DInterface)
	{
#pragma region IDirect3DDevice9
		case INTERFACE_IDIRECT3DDEVICE9:
		{
										   switch (eD3DMethod)
										   {
#pragma region Present
											   case METHOD_IDIRECT3DDEVICE9_PRESENT:
												   Present((LPDIRECT3DDEVICE9)pThis);
												   return nullptr;
#pragma endregion
#pragma region BeginScene
											   case METHOD_IDIRECT3DDEVICE9_BEGINSCENE:
												   // ensure left drawing side here
												   SetDrawingSide((LPDIRECT3DDEVICE9)pThis, RenderPosition::Left);
												   return nullptr;
#pragma endregion 
#pragma region EndScene
											   case METHOD_IDIRECT3DDEVICE9_ENDSCENE:
												   // ensure left drawing side here
												   SetDrawingSide((LPDIRECT3DDEVICE9)pThis, RenderPosition::Left);
												   nProvokerIndex |= AQU_PluginFlags::ImmediateReturnFlag;
												   nHr = ((IDirect3DDevice9*)pThis)->EndScene();
												   return (void*)&nHr;
#pragma endregion 
#pragma region SetRenderTarget
											   case METHOD_IDIRECT3DDEVICE9_SETRENDERTARGET:
												   if (m_bPresent)
												   {
													   if (!m_punRenderTargetIndex) return nullptr;
													   if (!m_ppcRenderTarget) return nullptr;

													   // call method
													   SetRenderTarget((LPDIRECT3DDEVICE9)pThis, *m_punRenderTargetIndex, *m_ppcRenderTarget);

													   // index too high ?
													   if ((*m_punRenderTargetIndex) >= D3D9_SIMULTANEOUS_RENDER_TARGET_COUNT) return nullptr;

													   // replace the call
													   if (m_eCurrentRenderingSide == RenderPosition::Left)
													   {
														   nHr = ((IDirect3DDevice9*)pThis)->SetRenderTarget(*m_punRenderTargetIndex, m_apcActiveRenderTargets[*m_punRenderTargetIndex]);
													   }
													   else
													   {
														   nHr = ((IDirect3DDevice9*)pThis)->SetRenderTarget(*m_punRenderTargetIndex, m_apcActiveRenderTargets[*m_punRenderTargetIndex + D3D9_SIMULTANEOUS_RENDER_TARGET_COUNT]);
													   }

													   // changing rendertarget resets viewport to fullsurface
													   if (nHr == D3D_OK) m_bActiveViewportIsDefault = true;

													   // method replaced, immediately return
													   nProvokerIndex |= AQU_PluginFlags::ImmediateReturnFlag;
													   return (void*)&nHr;
												   }
												   return nullptr;
#pragma endregion
#pragma region SetDepthStencilSurface
											   case METHOD_IDIRECT3DDEVICE9_SETDEPTHSTENCILSURFACE:
												   if (m_bPresent)
												   {
													   if (!m_ppcNewZStencil) return nullptr;

													   // call method
													   SetDepthStencilSurface((LPDIRECT3DDEVICE9)pThis, *m_ppcNewZStencil);

													   // replace the call
													   if (m_eCurrentRenderingSide == RenderPosition::Left)
													   {
														   nHr = ((IDirect3DDevice9*)pThis)->SetDepthStencilSurface(m_pcActiveDepthStencilSurface[0]);
													   }
													   else
													   {
														   nHr = ((IDirect3DDevice9*)pThis)->SetDepthStencilSurface(m_pcActiveDepthStencilSurface[1]);
													   }

													   // method replaced, immediately return
													   nProvokerIndex |= AQU_PluginFlags::ImmediateReturnFlag;
													   return (void*)&nHr;
												   }
												   return nullptr;
#pragma endregion
#pragma region SetTexture
											   case METHOD_IDIRECT3DDEVICE9_SETTEXTURE:
												   if (m_bPresent)
												   {
													   // get data
													   if (!m_punSampler) return nullptr;
													   if (!m_ppcTexture) return nullptr;

													   // call method
													   SetTexture((LPDIRECT3DDEVICE9)pThis, *m_punSampler, *m_ppcTexture);

													   // index too high ?
													   if ((*m_punSampler) >= D3D9_SIMULTANEAOUS_TEXTURE_COUNT) return nullptr;

													   // replace the call
													   if ((m_eCurrentRenderingSide == RenderPosition::Left) || (!(m_apcActiveTextures[*m_punSampler + D3D9_SIMULTANEAOUS_TEXTURE_COUNT])))
													   {
														   nHr = ((IDirect3DDevice9*)pThis)->SetTexture(*m_punSampler, m_apcActiveTextures[*m_punSampler]);
													   }
													   else
													   {
														   nHr = ((IDirect3DDevice9*)pThis)->SetTexture(*m_punSampler, m_apcActiveTextures[*m_punSampler + D3D9_SIMULTANEAOUS_TEXTURE_COUNT]);
													   }

													   // method replaced, immediately return
													   nProvokerIndex |= AQU_PluginFlags::ImmediateReturnFlag;
													   return (void*)&nHr;
												   }
												   return nullptr;
#pragma endregion
#pragma region Clear
											   case METHOD_IDIRECT3DDEVICE9_CLEAR:
												   if (m_bPresent)
												   {
													   bool bSwitched = true;

													   // switch the drawing side before the second draw call is done
													   if (m_eCurrentRenderingSide == RenderPosition::Left)
														   bSwitched = SetDrawingSide((LPDIRECT3DDEVICE9)pThis, RenderPosition::Right);
													   else if (m_eCurrentRenderingSide == RenderPosition::Right)
														   bSwitched = SetDrawingSide((LPDIRECT3DDEVICE9)pThis, RenderPosition::Left);

													   // no second call if not switched -> immediate return
													   if (!bSwitched)
													   {
														   nProvokerIndex |= AQU_PluginFlags::ImmediateReturnFlag;
														   nHr = S_OK;
														   return (void*)&nHr;
													   }
												   }
												   return nullptr;
#pragma endregion
#pragma region DrawPrimitive
											   case METHOD_IDIRECT3DDEVICE9_DRAWPRIMITIVE:
												   if (m_bPresent)
												   {
													   bool bSwitched = true;

													   // switch the drawing side before the second draw call is done
													   if (m_eCurrentRenderingSide == RenderPosition::Left)
														   bSwitched = SetDrawingSide((LPDIRECT3DDEVICE9)pThis, RenderPosition::Right);
													   else if (m_eCurrentRenderingSide == RenderPosition::Right)
														   bSwitched = SetDrawingSide((LPDIRECT3DDEVICE9)pThis, RenderPosition::Left);

													   // no second call if not switched -> immediate return
													   if (!bSwitched)
													   {
														   nProvokerIndex |= AQU_PluginFlags::ImmediateReturnFlag;
														   nHr = S_OK;
														   return (void*)&nHr;
													   }
												   }
												   return nullptr;
#pragma endregion
#pragma region DrawIndexedPrimitive
											   case METHOD_IDIRECT3DDEVICE9_DRAWINDEXEDPRIMITIVE:
												   if (m_bPresent)
												   {
													   bool bSwitched = true;

													   // switch the drawing side before the second draw call is done
													   if (m_eCurrentRenderingSide == RenderPosition::Left)
														   bSwitched = SetDrawingSide((LPDIRECT3DDEVICE9)pThis, RenderPosition::Right);
													   else if (m_eCurrentRenderingSide == RenderPosition::Right)
														   bSwitched = SetDrawingSide((LPDIRECT3DDEVICE9)pThis, RenderPosition::Left);

													   // no second call if not switched -> immediate return
													   if (!bSwitched)
													   {
														   nProvokerIndex |= AQU_PluginFlags::ImmediateReturnFlag;
														   nHr = S_OK;
														   return (void*)&nHr;
													   }
												   }
												   return nullptr;
#pragma endregion
#pragma region DrawPrimitiveUP
											   case METHOD_IDIRECT3DDEVICE9_DRAWPRIMITIVEUP:
												   if (m_bPresent)
												   {
													   bool bSwitched = true;

													   // switch the drawing side before the second draw call is done
													   if (m_eCurrentRenderingSide == RenderPosition::Left)
														   bSwitched = SetDrawingSide((LPDIRECT3DDEVICE9)pThis, RenderPosition::Right);
													   else if (m_eCurrentRenderingSide == RenderPosition::Right)
														   bSwitched = SetDrawingSide((LPDIRECT3DDEVICE9)pThis, RenderPosition::Left);

													   // no second call if not switched -> immediate return
													   if (!bSwitched)
													   {
														   nProvokerIndex |= AQU_PluginFlags::ImmediateReturnFlag;
														   nHr = S_OK;
														   return (void*)&nHr;
													   }
												   }
												   return nullptr;
#pragma endregion
#pragma region DrawIndexedPrimitiveUP
											   case METHOD_IDIRECT3DDEVICE9_DRAWINDEXEDPRIMITIVEUP:
												   if (m_bPresent)
												   {
													   bool bSwitched = true;

													   // switch the drawing side before the second draw call is done
													   if (m_eCurrentRenderingSide == RenderPosition::Left)
														   bSwitched = SetDrawingSide((LPDIRECT3DDEVICE9)pThis, RenderPosition::Right);
													   else if (m_eCurrentRenderingSide == RenderPosition::Right)
														   bSwitched = SetDrawingSide((LPDIRECT3DDEVICE9)pThis, RenderPosition::Left);

													   // no second call if not switched -> immediate return
													   if (!bSwitched)
													   {
														   nProvokerIndex |= AQU_PluginFlags::ImmediateReturnFlag;
														   nHr = S_OK;
														   return (void*)&nHr;
													   }
												   }
												   return nullptr;
#pragma endregion
#pragma region UpdateSurface
											   case METHOD_IDIRECT3DDEVICE9_UPDATESURFACE:
												   if (!m_ppSourceSurface) return nullptr;
												   if (!m_ppcSourceRect) return nullptr;
												   if (!m_ppcDestinationSurface) return nullptr;
												   if (!m_ppsDestPoint) return nullptr;
												   if (!(*m_ppSourceSurface)) return nullptr;
												   if (!(*m_ppcDestinationSurface)) return nullptr;
												   {
													   // get twins
													   IDirect3DSurface9* pcSurfaceDst = VerifyPrivateDataInterfaces((IDirect3DDevice9*)pThis, *m_ppcDestinationSurface);
													   IDirect3DSurface9* pcSurfaceSrc = VerifyPrivateDataInterfaces((IDirect3DDevice9*)pThis, *m_ppSourceSurface);

													   if (pcSurfaceDst)
													   {
														   if (pcSurfaceSrc)
														   {
															   // Source is stereo, destination is stereo. Copying both.
															   ((IDirect3DDevice9*)pThis)->UpdateSurface(pcSurfaceSrc, *m_ppcSourceRect, pcSurfaceDst, *m_ppsDestPoint);
														   }
														   else
														   {
															   // Source is not stereo, destination is stereo. Copying source to both sides of destination
															   ((IDirect3DDevice9*)pThis)->UpdateSurface(*m_ppSourceSurface, *m_ppcSourceRect, pcSurfaceDst, *m_ppsDestPoint);
														   }
													   }
													   // else Source is maybe stereo, destination is not stereo. Copied left side only.
												   }
												   return nullptr;
#pragma endregion  
#pragma region UpdateTexture
											   case METHOD_IDIRECT3DDEVICE9_UPDATETEXTURE:
												   if (!m_ppcSourceTexture) return nullptr;
												   if (!m_ppcDestinationTexture) return nullptr;
												   if (!(*m_ppcSourceTexture)) return nullptr;
												   if (!(*m_ppcDestinationTexture)) return nullptr;
												   {
													   // get twins
													   IDirect3DBaseTexture9* pcTextureSrc = VerifyPrivateDataInterfaces((IDirect3DDevice9*)pThis, *m_ppcSourceTexture, false);
													   IDirect3DBaseTexture9* pcTextureDst = VerifyPrivateDataInterfaces((IDirect3DDevice9*)pThis, *m_ppcDestinationTexture, (pcTextureSrc != nullptr));

													   if (pcTextureDst)
													   {
														   if (pcTextureSrc)
														   {
															   // Source is stereo, destination is stereo. Copying both.
															   ((IDirect3DDevice9*)pThis)->UpdateTexture(pcTextureSrc, pcTextureDst);
														   }
														   else
														   {
															   // Source is not stereo, destination is stereo. Copying source to both sides of destination
															   ((IDirect3DDevice9*)pThis)->UpdateTexture(*m_ppcSourceTexture, pcTextureDst);
														   }
													   }
													   // else Source is maybe stereo, destination is not stereo. Copied left side only.
												   }
												   return nullptr;
#pragma endregion   
#pragma region StretchRect 
											   case METHOD_IDIRECT3DDEVICE9_STRETCHRECT:
												   if (!m_ppcSourceSurface_StretchRect) return nullptr;
												   if (!m_ppcSourceRect_StretchRect) return nullptr;
												   if (!m_ppcDestSurface_StretchRect) return nullptr;
												   if (!m_ppcDestRect_StretchRect) return nullptr;
												   if (!m_peFilter_StretchRect) return nullptr;
												   if (!(*m_ppcSourceSurface_StretchRect)) return nullptr;
												   if (!(*m_ppcSourceRect_StretchRect)) return nullptr;
												   {
													   // get twins
													   IDirect3DSurface9* pcSurfaceDst = VerifyPrivateDataInterfaces((IDirect3DDevice9*)pThis, *m_ppcDestSurface_StretchRect);
													   IDirect3DSurface9* pcSurfaceSrc = VerifyPrivateDataInterfaces((IDirect3DDevice9*)pThis, *m_ppcSourceSurface_StretchRect);

													   if (pcSurfaceDst)
													   {
														   if (pcSurfaceSrc)
														   {
															   // Source is stereo, destination is stereo. Copying both.
															   ((IDirect3DDevice9*)pThis)->StretchRect(pcSurfaceSrc, *m_ppcSourceRect_StretchRect, pcSurfaceDst, *m_ppcDestRect_StretchRect, *m_peFilter_StretchRect);
														   }
														   else
														   {
															   // Source is not stereo, destination is stereo. Copying source to both sides of destination
															   ((IDirect3DDevice9*)pThis)->StretchRect(*m_ppcSourceSurface_StretchRect, *m_ppcSourceRect_StretchRect, pcSurfaceDst, *m_ppcDestRect_StretchRect, *m_peFilter_StretchRect);
														   }
													   }
													   // else Source is maybe stereo, destination is not stereo. Copied left side only.
												   }
												   return nullptr;
#pragma endregion  
#pragma region ColorFill
											   case METHOD_IDIRECT3DDEVICE9_COLORFILL:
												   if (!m_ppcSurface) return nullptr;
												   if (!m_ppsRect) return nullptr;
												   if (!m_punColor) return nullptr;
												   if (!(*m_ppcSurface)) return nullptr;
												   {
													   // get twin
													   IDirect3DSurface9* pcSurfaceTwin = VerifyPrivateDataInterfaces((IDirect3DDevice9*)pThis, *m_ppcSurface);

													   if (pcSurfaceTwin)
													   {
														   // call method for twin surface
														   ((IDirect3DDevice9*)pThis)->ColorFill(pcSurfaceTwin, *m_ppsRect, *m_punColor);
													   }
												   }
												   return nullptr;
#pragma endregion 
#pragma region GetBackBuffer
											   case METHOD_IDIRECT3DDEVICE9_GETBACKBUFFER:
												   // ensure left drawing side here
												   SetDrawingSide((LPDIRECT3DDEVICE9)pThis, RenderPosition::Left);
												   return nullptr;
#pragma endregion 
#pragma region GetRenderTarget
											   case METHOD_IDIRECT3DDEVICE9_GETRENDERTARGET:
												   // ensure left drawing side here
												   SetDrawingSide((LPDIRECT3DDEVICE9)pThis, RenderPosition::Left);
												   return nullptr;
#pragma endregion 
												   //#pragma region GetRenderTargetData
												   //											   case METHOD_IDIRECT3DDEVICE9_GETRENDERTARGETDATA:
												   //												   // ensure left drawing side here
												   //												   SetDrawingSide((LPDIRECT3DDEVICE9)pThis, RenderPosition::Left);
												   //												   return nullptr;
												   //#pragma endregion
#pragma region GetDepthStencilSurface
											   case METHOD_IDIRECT3DDEVICE9_GETDEPTHSTENCILSURFACE:
												   // ensure left drawing side here
												   SetDrawingSide((LPDIRECT3DDEVICE9)pThis, RenderPosition::Left);
												   return nullptr;
#pragma endregion  
#pragma region GetTexture 
											   case METHOD_IDIRECT3DDEVICE9_GETTEXTURE:
												   // ensure left drawing side here
												   SetDrawingSide((LPDIRECT3DDEVICE9)pThis, RenderPosition::Left);
												   return nullptr;
#pragma endregion  
#pragma region Reset
											   case METHOD_IDIRECT3DDEVICE9_RESET:
												   SetDrawingSide((LPDIRECT3DDEVICE9)pThis, RenderPosition::Left);
												   return nullptr;
#pragma endregion 
#pragma region DrawRectPatch
											   case METHOD_IDIRECT3DDEVICE9_DRAWRECTPATCH:
												   // TODO !!
												   return nullptr;
#pragma endregion  
#pragma region DrawTriPatch
											   case METHOD_IDIRECT3DDEVICE9_DRAWTRIPATCH:
												   // TODO !!
												   return nullptr;
#pragma endregion 
#pragma region SetRenderState
											   case METHOD_IDIRECT3DDEVICE9_SETRENDERSTATE:
												   if (!m_peState) return nullptr;
												   if (!m_punValue) return nullptr;
												   else
												   {
													   // catch D3DRS_POINT for "Resolve Z-Buffer" operation... read "Advanced-DX9-Capabilities-for-ATI-Radeon-Cards_v2.pdf" for more info
													   if (*m_peState == D3DRENDERSTATETYPE::D3DRS_POINTSIZE)
													   {
														   // the value 0x3f800000 seems to be also a resolve z buffer request
														   // - it is send by Skyrim on AMD cards
														   // however, the following code works to do it
														   DWORD unValue = *m_punValue;
														   if ((unValue == 0x3f800000) || (unValue == RESZ_CODE))
														   {
															   // Perform a dummy draw call to ensure texture sampler 0 is set before the
															   // resolve is triggered
															   // Vertex declaration and shaders may need to me adjusted to ensure no debug
															   // error message is produced
															   D3DXVECTOR3 vDummyPoint(0.0f, 0.0f, 0.0f);
															   ((IDirect3DDevice9*)pThis)->SetRenderState(D3DRS_ZENABLE, FALSE);
															   ((IDirect3DDevice9*)pThis)->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
															   ((IDirect3DDevice9*)pThis)->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
															   ((IDirect3DDevice9*)pThis)->DrawPrimitiveUP(D3DPT_POINTLIST, 1, vDummyPoint, sizeof(D3DXVECTOR3));
															   ((IDirect3DDevice9*)pThis)->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
															   ((IDirect3DDevice9*)pThis)->SetRenderState(D3DRS_ZENABLE, TRUE);
															   ((IDirect3DDevice9*)pThis)->SetRenderState(D3DRS_COLORWRITEENABLE, 0x0F);

															   ((IDirect3DDevice9*)pThis)->SetRenderState(D3DRS_POINTSIZE, unValue);
															   ((IDirect3DDevice9*)pThis)->SetRenderState(D3DRS_POINTSIZE, 0);

															   // switch the drawing side before the second resolve is done
															   if (m_eCurrentRenderingSide == RenderPosition::Left)
																   SetDrawingSide((LPDIRECT3DDEVICE9)pThis, RenderPosition::Right);
															   else if (m_eCurrentRenderingSide == RenderPosition::Right)
																   SetDrawingSide((LPDIRECT3DDEVICE9)pThis, RenderPosition::Left);

															   // do the same procedure for the second target
															   ((IDirect3DDevice9*)pThis)->SetRenderState(D3DRS_ZENABLE, FALSE);
															   ((IDirect3DDevice9*)pThis)->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
															   ((IDirect3DDevice9*)pThis)->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
															   ((IDirect3DDevice9*)pThis)->DrawPrimitiveUP(D3DPT_POINTLIST, 1, vDummyPoint, sizeof(D3DXVECTOR3));
															   ((IDirect3DDevice9*)pThis)->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
															   ((IDirect3DDevice9*)pThis)->SetRenderState(D3DRS_ZENABLE, TRUE);
															   ((IDirect3DDevice9*)pThis)->SetRenderState(D3DRS_COLORWRITEENABLE, 0x0F);
														   }
													   }
												   }
												   return nullptr;
#pragma endregion
#pragma region SetViewport
											   case METHOD_IDIRECT3DDEVICE9_SETVIEWPORT:
												   if (m_ppsViewport)
												   {
													   nHr = SetViewport((IDirect3DDevice9*)pThis, *m_ppsViewport);

													   // method replaced, immediately return
													   nProvokerIndex |= AQU_PluginFlags::ImmediateReturnFlag;
													   return (void*)&nHr;
												   }
												   return nullptr;
#pragma endregion
#pragma region CreateTexture
											   case METHOD_IDIRECT3DDEVICE9_CREATETEXTURE:
												   return nullptr; // NOT_IMPLEMENTED
												   if (!m_punWidth) return nullptr;
												   if (!m_punHeight) return nullptr;
												   if (!m_punLevels) return nullptr;
												   if (!m_punUsage) return nullptr;
												   if (!m_peFormat) return nullptr;
												   if (!m_pePool) return nullptr;
												   if (!m_ppcTextureCreate) return nullptr;
												   if (!m_ppvSharedHandle) return nullptr;
												   if (m_bUseD3D9Ex)
												   {
													   IDirect3DTexture9* pLeftTexture = NULL;
													   IDirect3DTexture9* pRightTexture = NULL;

													   D3DPOOL newPool = *m_pePool;

													   HRESULT hr = S_OK;
													   IDirect3DDevice9Ex *pDirect3DDevice9Ex = NULL;
													   if (SUCCEEDED(((IDirect3DDevice9*)pThis)->QueryInterface(IID_IDirect3DDevice9Ex, reinterpret_cast<void**>(&pDirect3DDevice9Ex))) &&
														   Pool == D3DPOOL_MANAGED)
													   {
														   newPool = D3DPOOL_DEFAULT;
														   pDirect3DDevice9Ex->Release();
													   }

													   // try and create left
													   if (SUCCEEDED(nHr = ((IDirect3DDevice9*)pThis)->CreateTexture(*m_punWidth, *m_punHeight, *m_punLevels, *m_punUsage, *m_peFormat, newPool, &pLeftTexture, *m_ppvSharedHandle)))
													   {
														   // Does this Texture need duplicating?
														   if (ShouldDuplicateTexture(*m_punWidth, *m_punHeight, *m_punLevels, *m_punUsage, *m_peFormat, *m_pePool))
														   {
															   if (FAILED(((IDirect3DDevice9*)pThis)->CreateTexture(*m_punWidth, *m_punHeight, *m_punLevels, *m_punUsage, *m_peFormat, newPool, &pRightTexture, *m_ppvSharedHandle)))
															   {
																   OutputDebugString(L"[STS] Failed to create right eye texture while attempting to create stereo pair, falling back to mono\n");
																   pRightTexture = NULL;
															   }
														   }
													   }
													   else
													   {
														   OutputDebugString(L"[STS] Failed to create texture\n");
													   }

													   if (SUCCEEDED(nHr))
													   {
														   *m_ppcTextureCreate = new IDirect3DStereoTexture9(pLeftTexture, pRightTexture, ((IDirect3DDevice9*)pThis));
													   }


													   // method replaced, immediately return
													   nProvokerIndex |= AQU_PluginFlags::ImmediateReturnFlag;
													   return (void*)&nHr;
												   }
												   return nullptr;
#pragma endregion
#pragma region CreateVolumeTexture
											   case  METHOD_IDIRECT3DDEVICE9_CREATEVOLUMETEXTURE:
												   return nullptr; // NOT_IMPLEMENTED
												   if (m_bUseD3D9Ex)
												   {

													   // method replaced, immediately return
													   nProvokerIndex |= AQU_PluginFlags::ImmediateReturnFlag;
													   return (void*)&nHr;
												   }
												   return nullptr;
#pragma endregion
#pragma region CreateCubeTexture
											   case  METHOD_IDIRECT3DDEVICE9_CREATECUBETEXTURE:
												   return nullptr; // NOT_IMPLEMENTED
												   if (m_bUseD3D9Ex)
												   {

													   // method replaced, immediately return
													   nProvokerIndex |= AQU_PluginFlags::ImmediateReturnFlag;
													   return (void*)&nHr;
												   }
												   return nullptr;
#pragma endregion
#pragma region CreateVertexBuffer
											   case  METHOD_IDIRECT3DDEVICE9_CREATEVERTEXBUFFER:
												   return nullptr; // NOT_IMPLEMENTED
												   if (m_bUseD3D9Ex)
												   {

													   // method replaced, immediately return
													   nProvokerIndex |= AQU_PluginFlags::ImmediateReturnFlag;
													   return (void*)&nHr;
												   }
												   return nullptr;
#pragma endregion
#pragma region CreateIndexBuffer
											   case  METHOD_IDIRECT3DDEVICE9_CREATEINDEXBUFFER:
												   return nullptr; // NOT_IMPLEMENTED
												   if (m_bUseD3D9Ex)
												   {

													   // method replaced, immediately return
													   nProvokerIndex |= AQU_PluginFlags::ImmediateReturnFlag;
													   return (void*)&nHr;
												   }
												   return nullptr;
#pragma endregion
#pragma region CreateRenderTarget
											   case  METHOD_IDIRECT3DDEVICE9_CREATERENDERTARGET:
												   return nullptr; // NOT_IMPLEMENTED
												   if (m_bUseD3D9Ex)
												   {

													   // method replaced, immediately return
													   nProvokerIndex |= AQU_PluginFlags::ImmediateReturnFlag;
													   return (void*)&nHr;
												   }
												   return nullptr;
#pragma endregion
#pragma region CreateDepthStencilSurface
											   case  METHOD_IDIRECT3DDEVICE9_CREATEDEPTHSTENCILSURFACE:
												   return nullptr; // NOT_IMPLEMENTED
												   if (m_bUseD3D9Ex)
												   {

													   // method replaced, immediately return
													   nProvokerIndex |= AQU_PluginFlags::ImmediateReturnFlag;
													   return (void*)&nHr;
												   }
												   return nullptr;
#pragma endregion
#pragma region GetRendertargetData
											   case  METHOD_IDIRECT3DDEVICE9_GETRENDERTARGETDATA:
												   return nullptr; // NOT_IMPLEMENTED
												   if (m_bUseD3D9Ex)
												   {

													   // method replaced, immediately return
													   nProvokerIndex |= AQU_PluginFlags::ImmediateReturnFlag;
													   return (void*)&nHr;
												   }
												   return nullptr;
#pragma endregion
#pragma region GetFrontBufferData
											   case METHOD_IDIRECT3DDEVICE9_GETFRONTBUFFERDATA:
												   return nullptr; // NOT_IMPLEMENTED
												   if (m_bUseD3D9Ex)
												   {

													   // method replaced, immediately return
													   nProvokerIndex |= AQU_PluginFlags::ImmediateReturnFlag;
													   return (void*)&nHr;
												   }
												   return nullptr;
#pragma endregion
										   }
										   return nullptr;
		}
#pragma endregion
#pragma region IDirect3DSwapChain9
		case INTERFACE_IDIRECT3DSWAPCHAIN9:
			switch (eD3DMethod)
			{
				case METHOD_IDIRECT3DSWAPCHAIN9_PRESENT:
				{
														   // get the device and call present
														   IDirect3DDevice9* pcDevice = nullptr;
														   if (pcDevice)
														   {
															   ((LPDIRECT3DSWAPCHAIN9)pThis)->GetDevice(&pcDevice);
															   Present(pcDevice);
															   pcDevice->Release();
														   }
				}
					return nullptr;
				case METHOD_IDIRECT3DSWAPCHAIN9_GETBACKBUFFER:
					// ensure left drawing side here
					SetDrawingSide((LPDIRECT3DDEVICE9)pThis, RenderPosition::Left);
					return nullptr;
				case METHOD_IDIRECT3DSWAPCHAIN9_GETFRONTBUFFERDATA:
					// ensure left drawing side here
					SetDrawingSide((LPDIRECT3DDEVICE9)pThis, RenderPosition::Left);
					return nullptr;
			}
			return nullptr;
#pragma endregion
#pragma region IDirect3DStateBlock9
		case INTERFACE_IDIRECT3DSTATEBLOCK9:
			switch (eD3DMethod)
			{
				case METHOD_IDIRECT3DSTATEBLOCK9_APPLY:
					Apply();
					return nullptr;
			}
#pragma endregion
	}
	return nullptr;
}

/**
* There's some windows event on our node.
***/
void StereoSplitter::WindowsEvent(UINT msg, WPARAM wParam, LPARAM lParam)
{
	// multiply mouse coords by 4 due to Aquilinus workspace architecture
	Vireio_GUI_Event sEvent = m_pcVireioGUI->WindowsEvent(msg, wParam, lParam, 4);
	switch (sEvent.eType)
	{
		case NoEvent:
			break;
		case ChangedToNext:
		case ChangedToPrevious:
			// handle game settings page
			if (sEvent.dwIndexOfPage == m_aunPageIDs[GUI_Pages::GameSettingsPage])
			{
				if (sEvent.dwIndexOfControl == m_sPageGameSettings.unDuplicateRenderTargetID)
					m_sGameSettings.nDuplicateRenderTarget = sEvent.nNewValue;
				if (sEvent.dwIndexOfControl == m_sPageGameSettings.unDuplicateDepthStencilID)
					m_sGameSettings.nDuplicateDepthStencil = sEvent.nNewValue;
				if (sEvent.dwIndexOfControl == m_sPageGameSettings.unDuplicateTextureID)
					m_sGameSettings.nDuplicateTexture = sEvent.nNewValue;
				if (sEvent.dwIndexOfControl == m_sPageGameSettings.unDuplicateCubeTextureID)
					m_sGameSettings.nDuplicateCubeTexture = sEvent.nNewValue;
				if (sEvent.dwIndexOfControl == m_sPageGameSettings.unSaveRenderStatesID)
					m_sGameSettings.nSaveRenderStatesID = sEvent.nNewValue;
			}
			break;
	}
}

/**
* Incoming Present() call.
* Handle the check time counter.
***/
void StereoSplitter::Present(IDirect3DDevice9* pcDevice)
{
	// ensure to be on left drawing side
	if (m_eCurrentRenderingSide == RenderPosition::Right)
		SetDrawingSide(pcDevice, RenderPosition::Left);

	// initialize ?
	if (!m_bPresent)
	{
		// get active render targets
		for (DWORD unI = 0; unI < D3D9_SIMULTANEOUS_RENDER_TARGET_COUNT; unI++)
		{
			// get render target index
			IDirect3DSurface9* pcTarget = nullptr;
			pcDevice->GetRenderTarget(unI, &pcTarget);

			// set active and release
			if (pcTarget)
			{
				m_apcActiveRenderTargets[unI] = pcTarget;
				m_apcActiveRenderTargets[D3D9_SIMULTANEOUS_RENDER_TARGET_COUNT + unI] = VerifyPrivateDataInterfaces(pcDevice, pcTarget);
				pcTarget->Release();
			}
		}

		// set number of render targets
		m_unRenderTargetNumber = 0;
		for (std::vector<IDirect3DSurface9*>::size_type i = 0; i < D3D9_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
		if (m_apcActiveRenderTargets[i] != NULL) m_unRenderTargetNumber = (DWORD)i + 1;

		// get depth stencil
		IDirect3DSurface9* pcDepthStencil = nullptr;
		pcDevice->GetDepthStencilSurface(&pcDepthStencil);

		// set active and release
		if (pcDepthStencil)
		{
			m_pcActiveDepthStencilSurface[0] = pcDepthStencil;
			m_pcActiveDepthStencilSurface[1] = VerifyPrivateDataInterfaces(pcDevice, pcDepthStencil);
			pcDepthStencil->Release();
		}

		// get viewport
		pcDevice->GetViewport(&m_sLastViewportSet);

		// TODO !! INIT ACTIVE TEXTURES
	}

	// finally, provide pointers to the left and right render target - get back buffer... TODO !! HANDLE CHANGED
	if (!m_pcActiveBackBufferSurface[0])
		pcDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &m_pcActiveBackBufferSurface[0]);

	if (m_pcActiveBackBufferSurface[0])
	{
		// set twin by verifying
		if (!m_pcActiveBackBufferSurface[1])
			m_pcActiveBackBufferSurface[1] = VerifyPrivateDataInterfaces(pcDevice, m_pcActiveBackBufferSurface[0]);

		if (m_pcActiveBackBufferSurface[1])
		{
			// create textures... (if not created)
			if ((!m_pcStereoBuffer[0]) || (!m_pcStereoBuffer[1]))
			{
				D3DSURFACE_DESC sDesc = D3DSURFACE_DESC();
				m_pcActiveBackBufferSurface[0]->GetDesc(&sDesc);

				// got the backbuffer ?
				if (sDesc.Width > 0)
				{
					// query for the IDirect3DDevice9Ex interface
					/*IDirect3DDevice9Ex *pcDirect3DDevice9Ex = NULL;
					if (SUCCEEDED(pcDevice->QueryInterface(IID_IDirect3DDevice9Ex, reinterpret_cast<void**>(&pcDirect3DDevice9Ex))))
					{
					HANDLE pHandleLeft = nullptr;
					if (!m_pcStereoBuffer[0])
					pcDirect3DDevice9Ex->CreateTexture(sDesc.Width, sDesc.Height, 1, D3DUSAGE_RENDERTARGET, sDesc.Format, D3DPOOL_DEFAULT, &m_pcStereoBuffer[0], &pHandleLeft);

					HANDLE pHandleRight = nullptr;
					if (!m_pcStereoBuffer[1])
					pcDirect3DDevice9Ex->CreateTexture(sDesc.Width, sDesc.Height, 1, D3DUSAGE_RENDERTARGET, sDesc.Format, D3DPOOL_DEFAULT, &m_pcStereoBuffer[1], &pHandleRight);

					// set shared handles as private interfaces
					if (m_pcStereoBuffer[0])
					m_pcStereoBuffer[0]->SetPrivateData(PDIID_Shared_Handle, (void*)pHandleLeft, sizeof(IUnknown*), NULL);
					if (m_pcStereoBuffer[1])
					m_pcStereoBuffer[1]->SetPrivateData(PDIID_Shared_Handle, (void*)pHandleRight, sizeof(IUnknown*), NULL);

					pcDirect3DDevice9Ex->Release();
					}
					else OutputDebugString(L"[STS] Failed to query IDirect3DDevice9Ex interface.");*/

					if (!m_pcStereoBuffer[0])
						pcDevice->CreateTexture(sDesc.Width, sDesc.Height, 1, D3DUSAGE_DYNAMIC, sDesc.Format, D3DPOOL_DEFAULT, &m_pcStereoBuffer[0], NULL);
					if (!m_pcStereoBuffer[1])
						pcDevice->CreateTexture(sDesc.Width, sDesc.Height, 1, D3DUSAGE_DYNAMIC, sDesc.Format, D3DPOOL_DEFAULT, &m_pcStereoBuffer[1], NULL);

					if ((!m_pcStereoBuffer[0]) || (!m_pcStereoBuffer[1])) OutputDebugString(L"[STS] Failed to create texture default/dynamic.");
				}
			}

			// first stereo output test
			if ((m_pcStereoBuffer[0]) && (m_pcStereoBuffer[1]))
			{
				m_pcStereoBuffer[0]->GetSurfaceLevel(0, &m_pcStereoBufferSurface[0]);
				m_pcStereoBuffer[1]->GetSurfaceLevel(0, &m_pcStereoBufferSurface[1]);
				if (m_pcStereoBufferSurface[0])
				{
					pcDevice->StretchRect(m_pcActiveBackBufferSurface[0], NULL, m_pcStereoBufferSurface[0], NULL, D3DTEXF_NONE);
					m_pcStereoBufferSurface[0]->Release();
				}
				if (m_pcStereoBufferSurface[1])
				{
					pcDevice->StretchRect(m_pcActiveBackBufferSurface[1], NULL, m_pcStereoBufferSurface[1], NULL, D3DTEXF_NONE);
					m_pcStereoBufferSurface[1]->Release();
				}
			}
		}
	}
	else
		OutputDebugString(L"Vireio Stereo Splitter : No primary swap chain found.");

	// set present() bool to true
	m_bPresent = true;
}

/**
* Incoming SetRenderTarget() call.
* Set render targets internally, handle render target monitoring.
***/
void StereoSplitter::SetRenderTarget(IDirect3DDevice9* pcDevice, DWORD unRenderTargetIndex, IDirect3DSurface9* pcRenderTarget)
{
	// set the render target internally
	if (unRenderTargetIndex < D3D9_SIMULTANEOUS_RENDER_TARGET_COUNT)
	{
		// set NULL manually, otherwise just set the render target :
		if (!pcRenderTarget)
		{
			m_apcActiveRenderTargets[unRenderTargetIndex] = NULL;
			m_apcActiveRenderTargets[unRenderTargetIndex + D3D9_SIMULTANEOUS_RENDER_TARGET_COUNT] = NULL;

			// set number of render targets
			m_unRenderTargetNumber = 0;
			for (std::vector<IDirect3DSurface9*>::size_type i = 0; i < D3D9_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
			if (m_apcActiveRenderTargets[i] != NULL) m_unRenderTargetNumber = (DWORD)i + 1;

			// and return
			return;
		}
		else
		{
			m_apcActiveRenderTargets[unRenderTargetIndex] = pcRenderTarget;

			// set number of render targets
			if (unRenderTargetIndex >= m_unRenderTargetNumber) m_unRenderTargetNumber = unRenderTargetIndex + 1;
		}
	}
	else return;

	// set second render target
	m_apcActiveRenderTargets[unRenderTargetIndex + D3D9_SIMULTANEOUS_RENDER_TARGET_COUNT] = VerifyPrivateDataInterfaces(pcDevice, pcRenderTarget);
	if ((unRenderTargetIndex == 0) && (m_apcActiveRenderTargets[D3D9_SIMULTANEOUS_RENDER_TARGET_COUNT] == nullptr))
		m_apcActiveRenderTargets[D3D9_SIMULTANEOUS_RENDER_TARGET_COUNT] = m_apcActiveRenderTargets[0];
}

/**
* Incoming SetDepthStencilSurface() call.
***/
void StereoSplitter::SetDepthStencilSurface(IDirect3DDevice9* pcDevice, IDirect3DSurface9* pNewZStencil)
{
	// set NULL manually, otherwise just set the render target :
	if (!pNewZStencil)
		m_pcActiveDepthStencilSurface[0] = NULL;
	else
		m_pcActiveDepthStencilSurface[0] = pNewZStencil;

	// set NULL twin if NULL
	if (!pNewZStencil)
		m_pcActiveDepthStencilSurface[1] = NULL;
	else
	{
		// get surface twin
		m_pcActiveDepthStencilSurface[1] = VerifyPrivateDataInterfaces(pcDevice, pNewZStencil);
	}

	m_bControlUpdate = true;
}

/**
* Incoming SetTexture() call.
***/
void StereoSplitter::SetTexture(IDirect3DDevice9* pcDevice, DWORD Stage, IDirect3DBaseTexture9* pcTexture)
{
	// set the texture internally
	if (Stage < D3D9_SIMULTANEAOUS_TEXTURE_COUNT)
	{
		// set NULL manually, otherwise just set the render target :
		if (!pcTexture)
		{
			m_apcActiveTextures[Stage] = NULL;

			// set number of textures
			m_unTextureNumber = 0;
			for (std::vector<IDirect3DSurface9*>::size_type i = 0; i < D3D9_SIMULTANEAOUS_TEXTURE_COUNT; i++)
			if (m_apcActiveTextures[i] != NULL) m_unTextureNumber = (DWORD)i + 1;
		}
		else
		{
			m_apcActiveTextures[Stage] = pcTexture;

			// set number of textures
			if (Stage >= m_unTextureNumber) m_unTextureNumber = Stage + 1;
		}
	}
	else return;

	// set NULL twin if NULL
	if (!pcTexture)
		m_apcActiveTextures[Stage + D3D9_SIMULTANEAOUS_TEXTURE_COUNT] = NULL;
	else
	{
		// get texture twin
		m_apcActiveTextures[Stage + D3D9_SIMULTANEAOUS_TEXTURE_COUNT] = VerifyPrivateDataInterfaces(pcDevice, pcTexture, false);
	}

	m_bControlUpdate = true;
}

/**
* Try and set, if success save viewport.
* If viewport width and height match primary render target size and zmin is 0 and zmax 1 set
* m_bActiveViewportIsDefault flag true.
* @see m_bActiveViewportIsDefault
***/
HRESULT StereoSplitter::SetViewport(IDirect3DDevice9* pcDevice, CONST D3DVIEWPORT9* psViewport)
{
	HRESULT nHR = pcDevice->SetViewport(psViewport);

	if (SUCCEEDED(nHR))
	{
		m_bActiveViewportIsDefault = IsViewportDefaultForMainRT(psViewport);
		m_sLastViewportSet = *psViewport;
	}

	return nHR;
}

/**
* IDirect3DStateBlock9->Apply()
***/
void StereoSplitter::Apply()
{
	// state block to be applied, will be handled in SetDrawingSide()
	m_bApply = true;
}

/**
* Verify private data interfaces for the surface.
***/
IDirect3DSurface9* StereoSplitter::VerifyPrivateDataInterfaces(IDirect3DDevice9* pcDevice, IDirect3DSurface9* pcSurface)
{
	if (!pcSurface) return nullptr;

	// does this render target view have a stereo twin view ?
	IDirect3DSurface9* pcSurfaceTwin = nullptr;
	DWORD unSize = sizeof(pcSurfaceTwin);
	pcSurface->GetPrivateData(PDIID_IDirect3DSurface9_Stereo_Twin, (void*)&pcSurfaceTwin, &unSize);

	if (pcSurfaceTwin)
	{
		// interface already verified ? return nullptr
		if (pcSurfaceTwin == NO_PRIVATE_INTERFACE)
			return nullptr;

		// no release here, DX9 does not increment the counter
		return pcSurfaceTwin;
	}
	else
	{
		// try to get the container
		IDirect3DBaseTexture9* pcTexture = nullptr;
		pcSurface->GetContainer(__uuidof(IDirect3DTexture9), (void**)&pcTexture);
		if (!pcTexture)
			pcSurface->GetContainer(__uuidof(IDirect3DCubeTexture9), (void**)&pcTexture);

		// is this a texture level surface ?
		if (pcTexture)
		{
			// create the stereo texture
			IDirect3DBaseTexture9* pcTextureTwin = nullptr;
			CreateStereoTexture(pcDevice, pcTexture, &pcTextureTwin);

			// no twin ?
			if (!pcTextureTwin)
				return nullptr;

			// try to get the twin after creating the stereo texture
			DWORD unSize = sizeof(pcSurfaceTwin);
			pcSurface->GetPrivateData(PDIID_IDirect3DSurface9_Stereo_Twin, (void*)&pcSurfaceTwin, &unSize);

			// interface already verified ? return nullptr
			if (pcSurfaceTwin == NO_PRIVATE_INTERFACE)
				return nullptr;

			// and return the twin
			if (pcSurfaceTwin) return pcSurfaceTwin; else return nullptr;
		}
		else
		{
			// get description
			D3DSURFACE_DESC sDesc;
			pcSurface->GetDesc(&sDesc);

			// should we duplicate ?
			if ((sDesc.Usage & D3DUSAGE_DEPTHSTENCIL) == D3DUSAGE_DEPTHSTENCIL)
			{
				// no duplicate ? set "verified" indicator
				if (!ShouldDuplicateDepthStencilSurface(sDesc.Width, sDesc.Height, sDesc.Format, sDesc.MultiSampleType, sDesc.MultiSampleQuality, false /*TODO!!*/))
				{
					IUnknown* pcIndicator = NO_PRIVATE_INTERFACE;
					pcSurface->SetPrivateData(PDIID_IDirect3DSurface9_Stereo_Twin, (void*)&pcIndicator, sizeof(IUnknown*), 0);
					return nullptr;
				}
			}
			else
			{
				// no duplicate ? set "verified" indicator
				if (!ShouldDuplicateRenderTarget(sDesc.Width, sDesc.Height, sDesc.Format, sDesc.MultiSampleType, sDesc.MultiSampleQuality, false /*TODO!!*/, (pcSurface == m_apcActiveRenderTargets[0])))
				{
					IUnknown* pcIndicator = NO_PRIVATE_INTERFACE;
					pcSurface->SetPrivateData(PDIID_IDirect3DSurface9_Stereo_Twin, (void*)&pcIndicator, sizeof(IUnknown*), 0);
					return nullptr;
				}
			}

			// get target pointers, both tex + surface
			LPDIRECT3DTEXTURE9 pcStereoTwinTexture = nullptr;
			LPDIRECT3DSURFACE9 pcStereoTwinSurface = nullptr;

			// depth stencil ??
			if ((sDesc.Usage & D3DUSAGE_DEPTHSTENCIL) == D3DUSAGE_DEPTHSTENCIL)
			{
				// discard must be false since we switch the stencil surfaces for every draw call
				if (FAILED(pcDevice->CreateDepthStencilSurface(sDesc.Width, sDesc.Height, sDesc.Format, sDesc.MultiSampleType, sDesc.MultiSampleQuality, false, &pcStereoTwinSurface, NULL)))
				{
					OutputDebugString(L"VireioStereoSplitter : Failed to create depth stencil surface.");
#ifdef _DEBUGTHIS
					wchar_t buf[32];
					wsprintf(buf, L"desc.Width %u", sDesc.Width); OutputDebugString(buf);
					wsprintf(buf, L"sDesc.Height %u", sDesc.Height); OutputDebugString(buf);
					wsprintf(buf, L"sDesc.Format %u", sDesc.Format); OutputDebugString(buf);
					wsprintf(buf, L"sDesc.MultiSampleType %u", sDesc.MultiSampleType); OutputDebugString(buf);
					wsprintf(buf, L"sDesc.MultiSampleQuality %u", sDesc.MultiSampleQuality); OutputDebugString(buf);
#endif
				}
			}
			else
			if (FAILED(pcDevice->CreateRenderTarget(sDesc.Width, sDesc.Height, sDesc.Format, sDesc.MultiSampleType, sDesc.MultiSampleQuality, false, &pcStereoTwinSurface, NULL)))
			{
				if (FAILED(pcDevice->CreateRenderTarget(sDesc.Width, sDesc.Height, sDesc.Format, sDesc.MultiSampleType, sDesc.MultiSampleQuality, true, &pcStereoTwinSurface, NULL)))
				{
					OutputDebugString(L"VireioStereoSplitter : Failed to create render target.");
#ifdef _DEBUGTHIS
					wchar_t buf[32];
					wsprintf(buf, L"sDesc.Width %u", sDesc.Width); OutputDebugString(buf);
					wsprintf(buf, L"sDesc.Height %u", sDesc.Height); OutputDebugString(buf);
					wsprintf(buf, L"sDesc.Format %u", sDesc.Format); OutputDebugString(buf);
					wsprintf(buf, L"sDesc.MultiSampleType %u", sDesc.MultiSampleType); OutputDebugString(buf);
					wsprintf(buf, L"sDesc.MultiSampleQuality %u", sDesc.MultiSampleQuality); OutputDebugString(buf);
#endif
					pcStereoTwinSurface = nullptr;
				}
			}

			// update resource and set private data
			if (pcStereoTwinSurface)
			{
				// update the surface
				pcDevice->UpdateSurface(pcSurface, nullptr, pcStereoTwinSurface, nullptr);

				// set twin as private data
				pcSurface->SetPrivateData(PDIID_IDirect3DSurface9_Stereo_Twin, (void*)pcStereoTwinSurface, sizeof(IUnknown*), D3DSPD_IUNKNOWN);

				// and return the twin
				return pcStereoTwinSurface;
			}
			else OutputDebugString(L"[STS] No surface !!");
		}
	}

	return nullptr;
}

/**
* Verify private data interfaces for the surface.
***/
IDirect3DBaseTexture9* StereoSplitter::VerifyPrivateDataInterfaces(IDirect3DDevice9* pcDevice, IDirect3DBaseTexture9* pcTexture, bool bDuplicate)
{
	if (!pcTexture) return nullptr;

	// get private data texture twin
	IDirect3DBaseTexture9* pcTextureTwin = nullptr;
	DWORD unSize = sizeof(pcTextureTwin);
	pcTexture->GetPrivateData(PDIID_IDirect3DBaseTexture9_Stereo_Twin, (void*)&pcTextureTwin, &unSize);

	// has a twin already ?
	if (pcTextureTwin)
	{
		// interface already verified ? return nullptr
		if (pcTextureTwin == NO_PRIVATE_INTERFACE)
			return nullptr;

		// no release here, DX9 does not increment the counter
		return pcTextureTwin;
	}
	else
	{
		if (bDuplicate)
		{
			// create the twin and return
			CreateStereoTexture(pcDevice, pcTexture, &pcTextureTwin);
			if (pcTextureTwin) return pcTextureTwin; else return nullptr;
		}
		else
		{
			// duplicate if depth stencil or render target
			if (pcTexture->GetType() == D3DRESOURCETYPE::D3DRTYPE_TEXTURE)
			{
				// get surface level 0
				IDirect3DSurface9* pcSurface = nullptr;
				((IDirect3DTexture9*)pcTexture)->GetSurfaceLevel(0, &pcSurface);
				if (pcSurface)
				{
					// get description
					D3DSURFACE_DESC sDesc = {};
					pcSurface->GetDesc(&sDesc);
					pcSurface->Release();

					// is depth buffer or render target ?
					if ((sDesc.Usage & D3DUSAGE_DEPTHSTENCIL) || (sDesc.Usage & D3DUSAGE_RENDERTARGET))
					{
						// create the twin and return
						CreateStereoTexture(pcDevice, pcTexture, &pcTextureTwin);
						if (pcTextureTwin) return pcTextureTwin; else return nullptr;
					}
				}
			}

		}
	}

	return nullptr;
}

/**
* Switches rendering to which ever side is specified by side.
* Use to specify the side that you want to draw to.
*
* Derives D3DProxyDevice::setDrawingSide() from the Vireio 2.x.x driver code.
*
* @return True if change succeeded, false if it fails.
* Attempting to switch to a side when that side is already the active side will return true without making any changes.
***/
bool StereoSplitter::SetDrawingSide(IDirect3DDevice9* pcDevice, RenderPosition eSide)
{
	// Already on the correct eye
	if (eSide == m_eCurrentRenderingSide)
		return true;

	// should never try and render for the right eye if there is no render target for the main render targets right side
	if ((m_apcActiveRenderTargets[0] == m_apcActiveRenderTargets[D3D9_SIMULTANEOUS_RENDER_TARGET_COUNT]) && (eSide == RenderPosition::Left))
	{
		return false;
	}

	// Everything hasn't changed yet but we set this first so we don't accidentally use the member instead of the local and break
	// things, as I have already managed twice.
	SetDrawingSideField(eSide);

	// state block was applied ?
	if (m_bApply)
	{
		// verify the render targets
		for (std::vector<IDirect3DSurface9*>::size_type i = 0; i < D3D9_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
		{
			// get the render target for this index
			LPDIRECT3DSURFACE9 pcRenderTarget = nullptr;
			pcDevice->GetRenderTarget((DWORD)i, &pcRenderTarget);

			// is this render target stored ?
			if (pcRenderTarget != m_apcActiveRenderTargets[i])
			{
				if (pcRenderTarget)
					SetRenderTarget(pcDevice, (DWORD)i, pcRenderTarget);
				else
					SetRenderTarget(pcDevice, (DWORD)i, nullptr);
			}

			// ..and release
			if (pcRenderTarget) pcRenderTarget->Release();
		}

		// TODO !! TEXTURES

		// state block is handled
		m_bApply = false;
	}

	// switch render targets to new eSide
	bool bRenderTargetChanged = false;
	HRESULT nHr = D3D_OK;
	for (std::vector<IDirect3DSurface9*>::size_type i = 0; i < m_unRenderTargetNumber; i++)
	{
		if (eSide == RenderPosition::Left)
		{
			nHr = pcDevice->SetRenderTarget((DWORD)i, m_apcActiveRenderTargets[i]);
		}
		else
		{
			nHr = pcDevice->SetRenderTarget((DWORD)i, m_apcActiveRenderTargets[i + D3D9_SIMULTANEOUS_RENDER_TARGET_COUNT]);
		}

		if (nHr)
		{
			OutputDebugString(L"Error trying to set one of the Render Targets while switching between active eyes for drawing.\n");
		}
		else
		{
			bRenderTargetChanged = true;
		}
	}

	// if a non-fullsurface viewport is active and a rendertarget changed we need to reapply the viewport
	if (bRenderTargetChanged && !m_bActiveViewportIsDefault)
	{
		pcDevice->SetViewport(&m_sLastViewportSet);
	}

	//#define DRAW_INDICATORS
#ifdef DRAW_INDICATORS
	if (eSide == RenderPosition::Left)
	{
		RECT rc;
		rc.top = 10;
		rc.bottom = 60;
		rc.left = 10;
		rc.right = 60;
		pcDevice->ColorFill(m_apcActiveRenderTargets[0], &rc, D3DCOLOR_ARGB(255, 100, 10, 200));
	}
	else
	{
		RECT rc;
		rc.top = 400;
		rc.bottom = 450;
		rc.left = 10;
		rc.right = 60;
		pcDevice->ColorFill(m_apcActiveRenderTargets[D3D9_SIMULTANEOUS_RENDER_TARGET_COUNT], &rc, D3DCOLOR_ARGB(255, 100, 200, 10));
	}
#endif

	// switch depth stencil to new side
	if (m_pcActiveDepthStencilSurface[0] != NULL)
	{
		if (eSide == RenderPosition::Left)
			nHr = pcDevice->SetDepthStencilSurface(m_pcActiveDepthStencilSurface[0]);
		else
		{
			nHr = pcDevice->SetDepthStencilSurface(m_pcActiveDepthStencilSurface[1]);
		}
	}

	// switch textures to new side
	for (std::vector<IDirect3DSurface9*>::size_type i = 0; i < m_unTextureNumber; i++)
	{
		if ((eSide == RenderPosition::Left) || (!(m_apcActiveTextures[i + D3D9_SIMULTANEAOUS_TEXTURE_COUNT])))
		{
			nHr = pcDevice->SetTexture((DWORD)i, m_apcActiveTextures[i]);
		}
		else
			nHr = pcDevice->SetTexture((DWORD)i, m_apcActiveTextures[i + D3D9_SIMULTANEAOUS_TEXTURE_COUNT]);

		if (nHr)
			OutputDebugString(L"[STS] Error trying to set one of the textures while switching between active eyes for drawing.\n");
	}

	// set shader constants to new side, first vertex shader
	if (m_ppasVSConstantRuleIndices)
	{
		if (*m_ppasVSConstantRuleIndices)
		{
			std::vector<Vireio_Constant_Rule_Index_DX9>* pasIndices = *m_ppasVSConstantRuleIndices;
			if (eSide == RenderPosition::Left)
			{
				for (std::vector<Vireio_Constant_Rule_Index_DX9>::size_type nI = 0; nI < pasIndices->size(); nI++)
					pcDevice->SetVertexShaderConstantF((*pasIndices)[nI].m_dwConstantRuleRegister, (*pasIndices)[nI].m_afConstantDataLeft, (*pasIndices)[nI].m_dwConstantRuleRegisterCount);
			}
			else
			{
				for (std::vector<Vireio_Constant_Rule_Index_DX9>::size_type nI = 0; nI < pasIndices->size(); nI++)
					pcDevice->SetVertexShaderConstantF((*pasIndices)[nI].m_dwConstantRuleRegister, (*pasIndices)[nI].m_afConstantDataRight, (*pasIndices)[nI].m_dwConstantRuleRegisterCount);
			}
		}
	}
	// now pixel shader
	if (m_ppasPSConstantRuleIndices)
	{
		if (*m_ppasPSConstantRuleIndices)
		{
			std::vector<Vireio_Constant_Rule_Index_DX9>* pasIndices = *m_ppasPSConstantRuleIndices;
			if (eSide == RenderPosition::Left)
			{
				for (std::vector<Vireio_Constant_Rule_Index_DX9>::size_type nI = 0; nI < pasIndices->size(); nI++)
					pcDevice->SetPixelShaderConstantF((*pasIndices)[nI].m_dwConstantRuleRegister, (*pasIndices)[nI].m_afConstantDataLeft, (*pasIndices)[nI].m_dwConstantRuleRegisterCount);
			}
			else
			{
				for (std::vector<Vireio_Constant_Rule_Index_DX9>::size_type nI = 0; nI < pasIndices->size(); nI++)
					pcDevice->SetPixelShaderConstantF((*pasIndices)[nI].m_dwConstantRuleRegister, (*pasIndices)[nI].m_afConstantDataRight, (*pasIndices)[nI].m_dwConstantRuleRegisterCount);
			}
		}
	}

	return true;
}

/**
* Creates a texture by surface description and returns both texture and surface level zero.
***/
void StereoSplitter::CreateStereoTexture(IDirect3DDevice9* pcDevice, IDirect3DBaseTexture9* pcTexture, IDirect3DBaseTexture9** ppcStereoTwinTexture)
{
	// get the resource type
	D3DRESOURCETYPE eType = pcTexture->GetType();
	switch (eType)
	{
		case D3DRTYPE_TEXTURE:
		{
								 // get first level
								 IDirect3DSurface9* pcSurface = nullptr;
								 ((IDirect3DTexture9*)pcTexture)->GetSurfaceLevel(0, &pcSurface);
								 if (pcSurface)
								 {
									 // get description
									 D3DSURFACE_DESC sDesc = {};
									 pcSurface->GetDesc(&sDesc);
									 pcSurface->Release();

									 // should we duplicate ?
									 if (!ShouldDuplicateTexture(sDesc.Width, sDesc.Height, pcTexture->GetLevelCount(), sDesc.Usage, sDesc.Format, sDesc.Pool))
									 {
										 // set no interface indicator
										 IUnknown* pcIndicator = NO_PRIVATE_INTERFACE;
										 pcTexture->SetPrivateData(PDIID_IDirect3DBaseTexture9_Stereo_Twin, (void*)&pcIndicator, sizeof(IUnknown*), 0);

										 // loop throug all levels, set no interface indicator
										 for (DWORD unI = 0; unI < pcTexture->GetLevelCount(); unI++)
										 {
											 // get current level
											 pcSurface = nullptr;
											 ((IDirect3DTexture9*)pcTexture)->GetSurfaceLevel(unI, &pcSurface);
											 if (pcSurface)
											 {
												 // set no interface indicator
												 pcSurface->SetPrivateData(PDIID_IDirect3DSurface9_Stereo_Twin, (void*)&pcIndicator, sizeof(IUnknown*), 0);
												 pcSurface->Release();
											 }

										 }

										 return;
									 }

									 // create the texture
									 if (FAILED(pcDevice->CreateTexture((UINT)sDesc.Width, (UINT)sDesc.Height, pcTexture->GetLevelCount(), sDesc.Usage, sDesc.Format, sDesc.Pool, (IDirect3DTexture9**)ppcStereoTwinTexture, NULL)))
									 {
										 OutputDebugString(L"VireioStereoSplitter : Failed to create render target texture.");
#ifdef _DEBUGTHIS							
										 wchar_t buf[32];
										 wsprintf(buf, L"sDesc.Width %u", sDesc.Width); OutputDebugString(buf);
										 wsprintf(buf, L"sDesc.Height %u", sDesc.Height); OutputDebugString(buf);
										 wsprintf(buf, L"sDesc.Usage %u", sDesc.Usage); OutputDebugString(buf);
										 wsprintf(buf, L"sDesc.Format %u", sDesc.Format); OutputDebugString(buf);
#endif
										 *ppcStereoTwinTexture = nullptr;
									 }
									 else
									 {
										 // update the texture
										 pcDevice->UpdateTexture(pcTexture, *ppcStereoTwinTexture);

										 // set twin as private data interface
										 IDirect3DTexture9* pcTextureTwin = (IDirect3DTexture9*)*ppcStereoTwinTexture;
										 pcTexture->SetPrivateData(PDIID_IDirect3DBaseTexture9_Stereo_Twin, (void*)pcTextureTwin, sizeof(IUnknown*), D3DSPD_IUNKNOWN);

										 // loop throug all levels, set stereo twin
										 for (DWORD unI = 0; unI < pcTexture->GetLevelCount(); unI++)
										 {
											 // get current level
											 pcSurface = nullptr;
											 ((IDirect3DTexture9*)pcTexture)->GetSurfaceLevel(unI, &pcSurface);
											 if (pcSurface)
											 {
												 // get level from twin
												 IDirect3DSurface9* pcSurfaceTwin = nullptr;
												 pcTextureTwin->GetSurfaceLevel(unI, &pcSurfaceTwin);
												 if (pcSurfaceTwin)
												 {
													 // set as private interface
													 pcSurface->SetPrivateData(PDIID_IDirect3DSurface9_Stereo_Twin, (void*)pcSurfaceTwin, sizeof(IUnknown*), D3DSPD_IUNKNOWN);
													 pcSurfaceTwin->Release();
												 }
												 pcSurface->Release();
											 }

										 }
									 }
								 }
		}
			break;
		case D3DRTYPE_VOLUMETEXTURE:
			// TODO !! VOLUME TEXTURE !!
			OutputDebugString(L"VOLUMETEXTURE... TODO !!");
			break;
		case D3DRTYPE_CUBETEXTURE:
		{
									 // get first level
									 IDirect3DSurface9* pcSurface = nullptr;
									 ((IDirect3DCubeTexture9*)pcTexture)->GetCubeMapSurface(D3DCUBEMAP_FACES::D3DCUBEMAP_FACE_POSITIVE_X, 0, &pcSurface);
									 if (pcSurface)
									 {
										 // get description
										 D3DSURFACE_DESC sDesc = {};
										 pcSurface->GetDesc(&sDesc);
										 pcSurface->Release();

										 // should we duplicate ?
										 if (!ShouldDuplicateCubeTexture(sDesc.Width, pcTexture->GetLevelCount(), sDesc.Usage, sDesc.Format, sDesc.Pool))
										 {
											 // set no interface indicator
											 IUnknown* pcIndicator = NO_PRIVATE_INTERFACE;
											 pcTexture->SetPrivateData(PDIID_IDirect3DBaseTexture9_Stereo_Twin, (void*)&pcIndicator, sizeof(IUnknown*), 0);

											 // loop throug all levels, set no interface indicator
											 for (DWORD unI = 0; unI < pcTexture->GetLevelCount(); unI++)
											 {
												 // loop throug all facetypes
												 for (UINT unFaceType = 0; unFaceType < 6; unFaceType++)
												 {
													 // get current level
													 pcSurface = nullptr;
													 ((IDirect3DCubeTexture9*)pcTexture)->GetCubeMapSurface((D3DCUBEMAP_FACES)unFaceType, unI, &pcSurface);
													 if (pcSurface)
													 {
														 // set no interface indicator
														 pcSurface->SetPrivateData(PDIID_IDirect3DSurface9_Stereo_Twin, (void*)&pcIndicator, sizeof(IUnknown*), 0);
														 pcSurface->Release();
													 }
												 }
											 }

											 return;
										 }

										 // create the texture
										 if (FAILED(pcDevice->CreateCubeTexture((UINT)sDesc.Width, pcTexture->GetLevelCount(), sDesc.Usage, sDesc.Format, sDesc.Pool, (IDirect3DCubeTexture9**)ppcStereoTwinTexture, NULL)))
										 {
											 OutputDebugString(L"VireioStereoSplitter : Failed to create render target texture.");
#ifdef _DEBUGTHIS							
											 wchar_t buf[32];
											 wsprintf(buf, L"sDesc.Width %u", sDesc.Width); OutputDebugString(buf);
											 wsprintf(buf, L"sDesc.Height %u", sDesc.Height); OutputDebugString(buf);
											 wsprintf(buf, L"sDesc.Usage %u", sDesc.Usage); OutputDebugString(buf);
											 wsprintf(buf, L"sDesc.Format %u", sDesc.Format); OutputDebugString(buf);
#endif
											 *ppcStereoTwinTexture = nullptr;
										 }
										 else
										 {
											 // update the texture
											 pcDevice->UpdateTexture(pcTexture, *ppcStereoTwinTexture);

											 // set twin as private data interface
											 IDirect3DCubeTexture9* pcTextureTwin = (IDirect3DCubeTexture9*)*ppcStereoTwinTexture;
											 pcTexture->SetPrivateData(PDIID_IDirect3DBaseTexture9_Stereo_Twin, (void*)pcTextureTwin, sizeof(IUnknown*), D3DSPD_IUNKNOWN);

											 // loop throug all levels, set stereo twin
											 for (DWORD unI = 0; unI < pcTexture->GetLevelCount(); unI++)
											 {
												 // loop throug all facetypes
												 for (UINT unFaceType = 0; unFaceType < 6; unFaceType++)
												 {
													 // get current level
													 pcSurface = nullptr;
													 ((IDirect3DCubeTexture9*)pcTexture)->GetCubeMapSurface((D3DCUBEMAP_FACES)unFaceType, unI, &pcSurface);
													 if (pcSurface)
													 {
														 // get level from twin
														 IDirect3DSurface9* pcSurfaceTwin = nullptr;
														 pcTextureTwin->GetCubeMapSurface((D3DCUBEMAP_FACES)unFaceType, unI, &pcSurfaceTwin);
														 if (pcSurfaceTwin)
														 {
															 // set as private interface
															 pcSurface->SetPrivateData(PDIID_IDirect3DSurface9_Stereo_Twin, (void*)pcSurfaceTwin, sizeof(IUnknown*), D3DSPD_IUNKNOWN);
															 pcSurfaceTwin->Release();
														 }
														 pcSurface->Release();
													 }
												 }
											 }
										 }
									 }
		}
		default:
			break;
	}
}

/**
* True if the by parameters described render target is to be duplicated for the handled game.
* @see StereoSplitter::VerifyPrivateDataInterfaces()
***/
bool StereoSplitter::ShouldDuplicateRenderTarget(UINT unWidth, UINT unHeight, D3DFORMAT Format, D3DMULTISAMPLE_TYPE eMultiSample, DWORD unMultisampleQuality, BOOL bLockable, bool bIsSwapChainBackBuffer)
{
	// switch by defined behaviour
	switch (m_sGameSettings.nDuplicateRenderTarget)
	{
		case DetermineDuplicateRToDS::DT_FALSE:
			return false;
		case DetermineDuplicateRToDS::DT_TRUE:
			return true;
		case DetermineDuplicateRToDS::DT_WIDTH_UNEQUAL_HEIGHT:
			if (bIsSwapChainBackBuffer)
			{
				return true;
			}
			return unWidth != unHeight;
	}

	return true;
}

/**
* True if the by parameters described depth stencil surface is to be duplicated for the handled game.
* @see StereoSplitter::VerifyPrivateDataInterfaces()
***/
bool StereoSplitter::ShouldDuplicateDepthStencilSurface(UINT unWidth, UINT unHeight, D3DFORMAT eFormat, D3DMULTISAMPLE_TYPE eMultiSample, DWORD unMultisampleQuality, BOOL bDiscard)
{
	// switch by defined behaviour
	switch (m_sGameSettings.nDuplicateDepthStencil)
	{
		case DetermineDuplicateRToDS::DT_FALSE:
			return false;
		case DetermineDuplicateRToDS::DT_TRUE:
			return true;
		case DetermineDuplicateRToDS::DT_WIDTH_UNEQUAL_HEIGHT:
			return unWidth != unHeight;
	}

	return true;
}

/**
* True if the by parameters described texture is to be duplicated for the handled game.
* @see StereoSplitter::VerifyPrivateDataInterfaces()
***/
bool StereoSplitter::ShouldDuplicateTexture(UINT unWidth, UINT unHeight, UINT unLevels, DWORD unUsage, D3DFORMAT eFormat, D3DPOOL ePool)
{
	// switch by defined behaviour
	switch (m_sGameSettings.nDuplicateTexture)
	{
		case DetermineDuplicateTexture::DT_FALSE:
			return false;
		case DetermineDuplicateTexture::DT_TRUE:
			return true;
		case DetermineDuplicateTexture::DT_IF_RENDER_TARGET:
			if ((unUsage & D3DUSAGE_DEPTHSTENCIL) == D3DUSAGE_DEPTHSTENCIL)
				return true;
			return IS_RENDER_TARGET(unUsage);
		case DetermineDuplicateTexture::DT_IF_RENDER_TARGET_AND_WIDTH_UNEQUAL_HEIGHT:
			if ((unUsage & D3DUSAGE_DEPTHSTENCIL) == D3DUSAGE_DEPTHSTENCIL)
				return true;
			return IS_RENDER_TARGET(unUsage) && (unWidth != unHeight);
	}

	// default - won't ever get here
	if ((unUsage & D3DUSAGE_DEPTHSTENCIL) == D3DUSAGE_DEPTHSTENCIL)
		return true;

	return IS_RENDER_TARGET(unUsage);
}

/**
* True if the by parameters described cube texture is to be duplicated for the handled game.
* Currently, returns true if cube texture is a render target.
* @see D3DProxyDevice::CreateCubeTexture()
***/
bool StereoSplitter::ShouldDuplicateCubeTexture(UINT unEdgeLength, UINT unLevels, DWORD unUsage, D3DFORMAT eFormat, D3DPOOL ePool)
{
	// switch by defined behaviour
	switch (m_sGameSettings.nDuplicateCubeTexture)
	{
		case DetermineDuplicateCubeTexture::DT_FALSE:
			return false;
		case DetermineDuplicateCubeTexture::DT_TRUE:
			return true;
		case DetermineDuplicateCubeTexture::DT_IF_RENDER_TARGET:
			return IS_RENDER_TARGET(unUsage);
	}

	return IS_RENDER_TARGET(unUsage);
}

/**
* Comparison made against active primary render target.
***/
bool StereoSplitter::IsViewportDefaultForMainRT(CONST D3DVIEWPORT9* psViewport)
{
	if (!m_apcActiveRenderTargets[0]) return false;

	// get description from first active render target
	D3DSURFACE_DESC sRTDesc;
	m_apcActiveRenderTargets[0]->GetDesc(&sRTDesc);

	return  ((psViewport->Height == sRTDesc.Height) && (psViewport->Width == sRTDesc.Width) &&
		(psViewport->MinZ <= SMALL_FLOAT) && (psViewport->MaxZ >= SLIGHTLY_LESS_THAN_ONE));
}

/**
* Creates the Graphical User Interface for this node.
***/
void StereoSplitter::CreateGUI()
{
	SIZE sSizeOfThis;
	sSizeOfThis.cx = GUI_WIDTH; sSizeOfThis.cy = GUI_HEIGHT;
	m_pcVireioGUI = new Vireio_GUI(sSizeOfThis, L"Candara", TRUE, GUI_CONTROL_FONTSIZE, RGB(120, 125, 90), RGB(250, 250, 200));

	// first, add all pages
	for (int i = 0; i < (int)GUI_Pages::NumberOfPages; i++)
	{
		UINT dwPage = m_pcVireioGUI->AddPage();
		m_aunPageIDs.push_back(dwPage);
	}

	// control structure
	Vireio_Control sControl;

#pragma region Static Text
	static std::vector<std::wstring> aszGameSettingsStrings;
	aszGameSettingsStrings.push_back(L"Duplicate Render Target"); aszGameSettingsStrings.push_back(std::wstring()); aszGameSettingsStrings.push_back(std::wstring());
	aszGameSettingsStrings.push_back(L"Duplicate Depth Stencil"); aszGameSettingsStrings.push_back(std::wstring()); aszGameSettingsStrings.push_back(std::wstring());
	aszGameSettingsStrings.push_back(L"Duplicate Texture"); aszGameSettingsStrings.push_back(std::wstring()); aszGameSettingsStrings.push_back(std::wstring());
	aszGameSettingsStrings.push_back(L"Duplicate Cube Texture"); aszGameSettingsStrings.push_back(std::wstring()); aszGameSettingsStrings.push_back(std::wstring());
	aszGameSettingsStrings.push_back(L"Save Render States"); aszGameSettingsStrings.push_back(std::wstring()); aszGameSettingsStrings.push_back(std::wstring());

	static std::vector<std::wstring> aszDetermineRenderTargetStrings;
	aszDetermineRenderTargetStrings.push_back(L"DT_FALSE");
	aszDetermineRenderTargetStrings.push_back(L"DT_TRUE");
	aszDetermineRenderTargetStrings.push_back(L"DT_WIDTH_UNEQUAL_HEIGHT");

	static std::vector<std::wstring> aszDetermineDepthStencilStrings;
	aszDetermineDepthStencilStrings.push_back(L"DT_FALSE");
	aszDetermineDepthStencilStrings.push_back(L"DT_TRUE");
	aszDetermineDepthStencilStrings.push_back(L"DT_WIDTH_UNEQUAL_HEIGHT");

	static std::vector<std::wstring> aszDetermineTextureStrings;
	aszDetermineTextureStrings.push_back(L"DT_FALSE");
	aszDetermineTextureStrings.push_back(L"DT_TRUE");
	aszDetermineTextureStrings.push_back(L"DT_IF_RENDER_TARGET");
	aszDetermineTextureStrings.push_back(L"DT_IF_RENDER_TARGET_AND_WIDTH_UNEQUAL_HEIGHT");

	static std::vector<std::wstring> aszDetermineCubeTextureStrings;
	aszDetermineCubeTextureStrings.push_back(L"DT_FALSE");
	aszDetermineCubeTextureStrings.push_back(L"DT_TRUE");
	aszDetermineCubeTextureStrings.push_back(L"DT_IF_RENDER_TARGET");

	static std::vector<std::wstring> aszDetermineSaveRenderStates;
	aszDetermineSaveRenderStates.push_back(L"DT_STATE_BLOCK");
	aszDetermineSaveRenderStates.push_back(L"DT_SELECTED_STATES_MANUALLY");
	aszDetermineSaveRenderStates.push_back(L"DT_ALL_STATES_MANUALLY");
	aszDetermineSaveRenderStates.push_back(L"DT_DO_NOT_SAVE_AND_RESTORE");
#pragma endregion

#pragma region Main Page
	ZeroMemory(&sControl, sizeof(Vireio_Control));
	static std::vector<std::wstring> sEntriesCommanders;
	static std::vector<std::wstring> sEntriesDecommanders;

	// create the decommanders list
	sControl.m_eControlType = Vireio_Control_Type::StaticListBox;
	sControl.m_sPosition.x = GUI_CONTROL_FONTBORDER;
	sControl.m_sPosition.y = 0;
	sControl.m_sSize.cx = GUI_WIDTH - GUI_CONTROL_BORDER;
	sControl.m_sSize.cy = NUMBER_OF_DECOMMANDERS * 64;
	sControl.m_sStaticListBox.m_bSelectable = false;
	sControl.m_sStaticListBox.m_paszEntries = &sEntriesDecommanders;
	UINT dwDecommandersList = m_pcVireioGUI->AddControl(m_aunPageIDs[GUI_Pages::MainPage], sControl);

	// create the commanders list out of the decommanders list
	sControl.m_sPosition.y += NUMBER_OF_DECOMMANDERS * 64;
	sControl.m_sPosition.x = GUI_WIDTH >> 2;
	sControl.m_sSize.cy = NUMBER_OF_COMMANDERS * 64;
	sControl.m_sStaticListBox.m_paszEntries = &sEntriesCommanders;
	UINT dwCommandersList = m_pcVireioGUI->AddControl(m_aunPageIDs[GUI_Pages::MainPage], sControl);

	// and add all entries
	for (int i = 0; i < NUMBER_OF_DECOMMANDERS; i++)
		m_pcVireioGUI->AddEntry(dwDecommandersList, this->GetDecommanderName(i));
	for (int i = 0; i < NUMBER_OF_COMMANDERS; i++)
		m_pcVireioGUI->AddEntry(dwCommandersList, this->GetCommanderName(i));
#pragma endregion

#pragma region Game setting page
	m_sPageGameSettings.unTextlist = CreateStaticListControl(m_pcVireioGUI, m_aunPageIDs[GUI_Pages::GameSettingsPage], &aszGameSettingsStrings, DUPLICATE_RENDERTARGET_POS_X, DUPLICATE_RENDERTARGET_POS_Y - GUI_CONTROL_FONTSIZE, GUI_CONTROL_SPINSIZE);

	m_sPageGameSettings.unDuplicateRenderTargetID = CreateSpinControl(m_pcVireioGUI, m_aunPageIDs[GUI_Pages::GameSettingsPage], &aszDetermineRenderTargetStrings, (UINT)m_sGameSettings.nDuplicateRenderTarget, DUPLICATE_RENDERTARGET_POS_X, DUPLICATE_RENDERTARGET_POS_Y, GUI_CONTROL_SPINSIZE);
	m_sPageGameSettings.unDuplicateDepthStencilID = CreateSpinControl(m_pcVireioGUI, m_aunPageIDs[GUI_Pages::GameSettingsPage], &aszDetermineDepthStencilStrings, (UINT)m_sGameSettings.nDuplicateDepthStencil, DUPLICATE_DEPTHSTENCIL_POS_X, DUPLICATE_DEPTHSTENCIL_POS_Y, GUI_CONTROL_SPINSIZE);
	m_sPageGameSettings.unDuplicateTextureID = CreateSpinControl(m_pcVireioGUI, m_aunPageIDs[GUI_Pages::GameSettingsPage], &aszDetermineTextureStrings, (UINT)m_sGameSettings.nDuplicateTexture, DUPLICATE_TEXTURE_POS_X, DUPLICATE_TEXTURE_POS_Y, GUI_CONTROL_SPINSIZE);
	m_sPageGameSettings.unDuplicateCubeTextureID = CreateSpinControl(m_pcVireioGUI, m_aunPageIDs[GUI_Pages::GameSettingsPage], &aszDetermineCubeTextureStrings, (UINT)m_sGameSettings.nDuplicateCubeTexture, DUPLICATE_CUBETEXTURE_POS_X, DUPLICATE_CUBETEXTURE_POS_Y, GUI_CONTROL_SPINSIZE);
	m_sPageGameSettings.unSaveRenderStatesID = CreateSpinControl(m_pcVireioGUI, m_aunPageIDs[GUI_Pages::GameSettingsPage], &aszDetermineSaveRenderStates, (UINT)m_sGameSettings.nSaveRenderStatesID, SAVE_RENDERSTATES_POS_X, SAVE_RENDERSTATES_POS_Y, GUI_CONTROL_SPINSIZE);
#pragma endregion

#pragma region Description Page
	ZeroMemory(&sControl, sizeof(Vireio_Control));
	static std::vector<std::wstring> sEntriesDescription;

	// create the description list
	sControl.m_eControlType = Vireio_Control_Type::StaticListBox;
	sControl.m_sPosition.x = GUI_CONTROL_FONTBORDER;
	sControl.m_sPosition.y = 0;
	sControl.m_sSize.cx = GUI_WIDTH - GUI_CONTROL_BORDER;
	sControl.m_sSize.cy = (NUMBER_OF_DECOMMANDERS + NUMBER_OF_COMMANDERS) * 64;
	sControl.m_sStaticListBox.m_bSelectable = false;
	sControl.m_sStaticListBox.m_paszEntries = &sEntriesDescription;
	UINT dwDescriptionList = m_pcVireioGUI->AddControl(m_aunPageIDs[GUI_Pages::DescriptionPage], sControl);

	// and add all entries
	m_pcVireioGUI->AddEntry(dwDescriptionList, L"Device9::->SetRenderTarget()  target index");
	m_pcVireioGUI->AddEntry(dwDescriptionList, L"Device9::->SetRenderTarget() render target");
	m_pcVireioGUI->AddEntry(dwDescriptionList, L"Device9::->SetDepthStencilSurface() surface");
	m_pcVireioGUI->AddEntry(dwDescriptionList, L"Device9::->SetTexture() sampler index");
	m_pcVireioGUI->AddEntry(dwDescriptionList, L"Device9::->SetTexture() texture pointer");
	m_pcVireioGUI->AddEntry(dwDescriptionList, L"Device9::->UpdateSurface() source surface");
	m_pcVireioGUI->AddEntry(dwDescriptionList, L"Device9::->UpdateSurface() source rectangle");
	m_pcVireioGUI->AddEntry(dwDescriptionList, L"Device9::->UpdateSurface() destination surface");
	m_pcVireioGUI->AddEntry(dwDescriptionList, L"Device9::->UpdateSurface() destination point");
	m_pcVireioGUI->AddEntry(dwDescriptionList, L"Device9::->UpdateTexture() source texture");
	m_pcVireioGUI->AddEntry(dwDescriptionList, L"Device9::->UpdateTexture() destination texture");
	m_pcVireioGUI->AddEntry(dwDescriptionList, L"Device9::->ColorFill() surface pointer");
	m_pcVireioGUI->AddEntry(dwDescriptionList, L"Device9::->ColorFill() destination rectangle");
	m_pcVireioGUI->AddEntry(dwDescriptionList, L"Device9::->ColorFill() destination color");
	m_pcVireioGUI->AddEntry(dwDescriptionList, L"Device9::->StretchRect() source surface");
	m_pcVireioGUI->AddEntry(dwDescriptionList, L"Device9::->StretchRect() source rectangle");
	m_pcVireioGUI->AddEntry(dwDescriptionList, L"Device9::->StretchRect() destination surface");
	m_pcVireioGUI->AddEntry(dwDescriptionList, L"Device9::->StretchRect() destination rectangle");
	m_pcVireioGUI->AddEntry(dwDescriptionList, L"Device9::->StretchRect() filter");
	m_pcVireioGUI->AddEntry(dwDescriptionList, L"Pointer to the extern drawing side bool.");
	m_pcVireioGUI->AddEntry(dwDescriptionList, L"The constant rule indices for the vshader.");
	m_pcVireioGUI->AddEntry(dwDescriptionList, L"The constant rule indices for the pshader.");
	m_pcVireioGUI->AddEntry(dwDescriptionList, L"Device9::->SetRenderState() State");
	m_pcVireioGUI->AddEntry(dwDescriptionList, L"Device9::->SetRenderState() Value");
	m_pcVireioGUI->AddEntry(dwDescriptionList, L"Device9::->SetViewport() Viewport");
#pragma endregion
}
