#pragma once
#include <d3d9.h>
#include <memory>
#include <string>
#define MMD_PLUGIN_API __declspec(dllexport)
#ifdef MAKE_MMD_PLUGIN
# define MMD_DLL_FUNC_API __declspec(dllexport)
#else
# define MMD_DLL_FUNC_API __declspec(dllimport)
# pragma comment(lib,"MMDPlugin/MMDPlugin")
#endif // MAKE_MMD_PLUGIN

extern "C"
{
  // MMDのウィンドウハンドルを返します。
  MMD_DLL_FUNC_API HWND getHWND();

  // 重複しないWM_APP+X値を返します。
  // すでにすべての値を使い切っている場合は-1を返します。
  MMD_DLL_FUNC_API int createWM_APP_ID();

  /// <summary>
  /// [mini, maxi]の区間領域を予約します。すでに使われている場合falseを返します。
  /// </summary>
  /// <param name="mini">WM_APP &lt;= mini &lt;= WM_APP + 0x3FFF </param>
  /// <param name="maxi">WM_APP &lt;= maxi &lt;= WM_APP + 0x3FFF</param>
  /// <returns></returns>
  MMD_DLL_FUNC_API bool reserveWM_APP_ID(int mini, int maxi);
}
#pragma warning( push )
#pragma warning( disable : 4100 )

struct MMDPluginDLL1
{
public:

  virtual ~MMDPluginDLL1() {}

  // callback
  // すべて実際の関数が呼ばれる前にコールバックされる
  /*
  // 内部実装
  MMDPluginDLL1::QueryInterface(riid, ppvObj);
  return d3dDevice->QueryInterface(riid, ppvObj);
  */
  virtual void QueryInterface(REFIID riid, void** ppvObj) {}

  virtual void AddRef() {}

  virtual void Release() {}

  virtual void TestCooperativeLevel() {}

  virtual void GetAvailableTextureMem() {}

  virtual void EvictManagedResources() {}

  virtual void GetDirect3D(IDirect3D9** ppD3D9) {}

  virtual void GetDeviceCaps(D3DCAPS9* pCaps) {}

  virtual void GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode) {}

  virtual void GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS* pParameters) {}

  virtual void SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap) {}

  virtual void SetCursorPosition(int X, int Y, DWORD Flags) {}

  virtual void ShowCursor(BOOL bShow) {}

  virtual void CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain) {}

  virtual void GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain) {}

  virtual void GetNumberOfSwapChains() {}

  virtual void Reset(D3DPRESENT_PARAMETERS* pPresentationParameters) {}

  virtual void Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindow, CONST RGNDATA* pDirtyRegion) {}

  virtual void GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer) {}

  virtual void GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus) {}

  virtual void SetDialogBoxMode(BOOL bEnableDialogs) {}

  virtual void SetGammaRamp(UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp) {}

  virtual void GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp) {}

  virtual void CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) {}

  virtual void CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle) {}

  virtual void CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle) {}

  virtual void CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle) {}

  virtual void CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle) {}

  virtual void CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {}

  virtual void CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {}

  virtual void UpdateSurface(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint) {}

  virtual void UpdateTexture(IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture) {}

  virtual void GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface) {}

  virtual void GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface) {}

  virtual void StretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter) {}

  virtual void ColorFill(IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color) {}

  virtual void CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {}

  virtual void SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) {}

  virtual void GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget) {}

  virtual void SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil) {}

  virtual void GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface) {}

  virtual void BeginScene() {}

  virtual void EndScene() {}

  virtual void Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) {}

  virtual void SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) {}

  virtual void GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix) {}

  virtual void MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) {}

  virtual void SetViewport(CONST D3DVIEWPORT9* pViewport) {}

  virtual void GetViewport(D3DVIEWPORT9* pViewport) {}

  virtual void SetMaterial(CONST D3DMATERIAL9* pMaterial) {}

  virtual void GetMaterial(D3DMATERIAL9* pMaterial) {}

  virtual void SetLight(DWORD Index, CONST D3DLIGHT9* pLight) {}

  virtual void GetLight(DWORD Index, D3DLIGHT9* pLight) {}

  virtual void LightEnable(DWORD Index, BOOL Enable) {}

  virtual void GetLightEnable(DWORD Index, BOOL* pEnable) {}

  virtual void SetClipPlane(DWORD Index, CONST float* pPlane) {}

  virtual void GetClipPlane(DWORD Index, float* pPlane) {}

  virtual void SetRenderState(D3DRENDERSTATETYPE State, DWORD Value) {}

  virtual void GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue) {}

  virtual void CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB) {}

  virtual void BeginStateBlock() {}

  virtual void EndStateBlock(IDirect3DStateBlock9** ppSB) {}

  virtual void SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus) {}

  virtual void GetClipStatus(D3DCLIPSTATUS9* pClipStatus) {}

  virtual void GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture) {}

  virtual void SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture) {}

  virtual void GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue) {}

  virtual void SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) {}

  virtual void GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue) {}

  virtual void SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) {}

  virtual void ValidateDevice(DWORD* pNumPasses) {}

  virtual void SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY* pEntries) {}

  virtual void GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries) {}

  virtual void SetCurrentTexturePalette(UINT PaletteNumber) {}

  virtual void GetCurrentTexturePalette(UINT* PaletteNumber) {}

  virtual void SetScissorRect(CONST RECT* pRect) {}

  virtual void GetScissorRect(RECT* pRect) {}

  virtual void SetSoftwareVertexProcessing(BOOL bSoftware) {}

  virtual void GetSoftwareVertexProcessing() {}

  virtual void SetNPatchMode(float nSegments) {}

  virtual void GetNPatchMode() {}

  virtual void DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) {}

  virtual void DrawIndexedPrimitive(D3DPRIMITIVETYPE, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {}

  virtual void DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {}

  virtual void DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {}

  virtual void ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags) {}

  virtual void CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl) {}

  virtual void SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl) {}

  virtual void GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl) {}

  virtual void SetFVF(DWORD FVF) {}

  virtual void GetFVF(DWORD* pFVF) {}

  virtual void CreateVertexShader(CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader) {}

  virtual void SetVertexShader(IDirect3DVertexShader9* pShader) {}

  virtual void GetVertexShader(IDirect3DVertexShader9** ppShader) {}

  virtual void SetVertexShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) {}

  virtual void GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) {}

  virtual void SetVertexShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) {}

  virtual void GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) {}

  virtual void SetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount) {}

  virtual void GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) {}

  virtual void SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride) {}

  virtual void GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride) {}

  virtual void SetStreamSourceFreq(UINT StreamNumber, UINT Setting) {}

  virtual void GetStreamSourceFreq(UINT StreamNumber, UINT* pSetting) {}

  virtual void SetIndices(IDirect3DIndexBuffer9* pIndexData) {}

  virtual void GetIndices(IDirect3DIndexBuffer9** ppIndexData) {}

  virtual void CreatePixelShader(CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader) {}

  virtual void SetPixelShader(IDirect3DPixelShader9* pShader) {}

  virtual void GetPixelShader(IDirect3DPixelShader9** ppShader) {}

  virtual void SetPixelShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) {}

  virtual void GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) {}

  virtual void SetPixelShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) {}

  virtual void GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) {}

  virtual void SetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount) {}

  virtual void GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) {}

  virtual void DrawRectPatch(UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo) {}

  virtual void DrawTriPatch(UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo) {}

  virtual void DeletePatch(UINT Handle) {}

  virtual void CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery) {}
};

struct MMDPluginDLL2 : public MMDPluginDLL1
{
public:

  // callback
  // すべて実際の関数が呼ばれ後にコールバックされる
  /*
  // 内部実装
  auto res = d3dDevice->QueryInterface(riid, ppvObj);
  MMDPluginDLL2::PostQueryInterface(riid, ppvObj, res);

  */
  virtual void PostQueryInterface(REFIID riid, void** ppvObj, HRESULT& res) {}

  virtual void PostAddRef(ULONG& res) {}

  virtual void PostRelease(ULONG& res) {}

  virtual void PostTestCooperativeLevel(HRESULT& res) {}

  virtual void PostGetAvailableTextureMem(UINT& res) {}

  virtual void PostEvictManagedResources(HRESULT& res) {}

  virtual void PostGetDirect3D(IDirect3D9** ppD3D9, HRESULT& res) {}

  virtual void PostGetDeviceCaps(D3DCAPS9* pCaps, HRESULT& res) {}

  virtual void PostGetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode, HRESULT& res) {}

  virtual void PostGetCreationParameters(D3DDEVICE_CREATION_PARAMETERS* pParameters, HRESULT& res) {}

  virtual void PostSetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap, HRESULT& res) {}

  virtual void PostSetCursorPosition(int X, int Y, DWORD Flags) {}

  virtual void PostShowCursor(BOOL bShow, BOOL& res) {}

  virtual void PostCreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain, HRESULT& res) {}

  virtual void PostGetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain, HRESULT& res) {}

  virtual void PostGetNumberOfSwapChains(UINT& res) {}

  virtual void PostReset(D3DPRESENT_PARAMETERS* pPresentationParameters, HRESULT& res) {}

  virtual void PostPresent(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindow, CONST RGNDATA* pDirtyRegion, HRESULT& res) {}

  virtual void PostGetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer, HRESULT& res) {}

  virtual void PostGetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus, HRESULT& res) {}

  virtual void PostSetDialogBoxMode(BOOL bEnableDialogs, HRESULT& res) {}

  virtual void PostSetGammaRamp(UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp) {}

  virtual void PostGetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp) {}

  virtual void PostCreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle, HRESULT& res) {}

  virtual void PostCreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle, HRESULT& res) {}

  virtual void PostCreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle, HRESULT& res) {}

  virtual void PostCreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle, HRESULT& res) {}

  virtual void PostCreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle, HRESULT& res) {}

  virtual void PostCreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, HRESULT& res) {}

  virtual void PostCreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, HRESULT& res) {}

  virtual void PostUpdateSurface(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint, HRESULT& res) {}

  virtual void PostUpdateTexture(IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture, HRESULT& res) {}

  virtual void PostGetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface, HRESULT& res) {}

  virtual void PostGetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface, HRESULT& res) {}

  virtual void PostStretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter, HRESULT& res) {}

  virtual void PostColorFill(IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color, HRESULT& res) {}

  virtual void PostCreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, HRESULT& res) {}

  virtual void PostSetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget, HRESULT& res) {}

  virtual void PostGetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget, HRESULT& res) {}

  virtual void PostSetDepthStencilSurface(IDirect3DSurface9* pNewZStencil, HRESULT& res) {}

  virtual void PostGetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface, HRESULT& res) {}

  virtual void PostBeginScene(HRESULT& res) {}

  virtual void PostEndScene(HRESULT& res) {}

  virtual void PostClear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil, HRESULT& res) {}

  virtual void PostSetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix, HRESULT& res) {}

  virtual void PostGetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix, HRESULT& res) {}

  virtual void PostMultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix, HRESULT& res) {}

  virtual void PostSetViewport(CONST D3DVIEWPORT9* pViewport, HRESULT& res) {}

  virtual void PostGetViewport(D3DVIEWPORT9* pViewport, HRESULT& res) {}

  virtual void PostSetMaterial(CONST D3DMATERIAL9* pMaterial, HRESULT& res) {}

  virtual void PostGetMaterial(D3DMATERIAL9* pMaterial, HRESULT& res) {}

  virtual void PostSetLight(DWORD Index, CONST D3DLIGHT9* pLight, HRESULT& res) {}

  virtual void PostGetLight(DWORD Index, D3DLIGHT9* pLight, HRESULT& res) {}

  virtual void PostLightEnable(DWORD Index, BOOL Enable, HRESULT& res) {}

  virtual void PostGetLightEnable(DWORD Index, BOOL* pEnable, HRESULT& res) {}

  virtual void PostSetClipPlane(DWORD Index, CONST float* pPlane, HRESULT& res) {}

  virtual void PostGetClipPlane(DWORD Index, float* pPlane, HRESULT& res) {}

  virtual void PostSetRenderState(D3DRENDERSTATETYPE State, DWORD Value, HRESULT& res) {}

  virtual void PostGetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue, HRESULT& res) {}

  virtual void PostCreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB, HRESULT& res) {}

  virtual void PostBeginStateBlock(HRESULT& res) {}

  virtual void PostEndStateBlock(IDirect3DStateBlock9** ppSB, HRESULT& res) {}

  virtual void PostSetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus, HRESULT& res) {}

  virtual void PostGetClipStatus(D3DCLIPSTATUS9* pClipStatus, HRESULT& res) {}

  virtual void PostGetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture, HRESULT& res) {}

  virtual void PostSetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture, HRESULT& res) {}

  virtual void PostGetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue, HRESULT& res) {}

  virtual void PostSetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value, HRESULT& res) {}

  virtual void PostGetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue, HRESULT& res) {}

  virtual void PostSetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value, HRESULT& res) {}

  virtual void PostValidateDevice(DWORD* pNumPasses, HRESULT& res) {}

  virtual void PostSetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY* pEntries, HRESULT& res) {}

  virtual void PostGetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries, HRESULT& res) {}

  virtual void PostSetCurrentTexturePalette(UINT PaletteNumber, HRESULT& res) {}

  virtual void PostGetCurrentTexturePalette(UINT* PaletteNumber, HRESULT& res) {}

  virtual void PostSetScissorRect(CONST RECT* pRect, HRESULT& res) {}

  virtual void PostGetScissorRect(RECT* pRect, HRESULT& res) {}

  virtual void PostSetSoftwareVertexProcessing(BOOL bSoftware, HRESULT& res) {}

  virtual void PostGetSoftwareVertexProcessing(BOOL& res) {}

  virtual void PostSetNPatchMode(float nSegments, HRESULT& res) {}

  virtual void PostGetNPatchMode(float& res) {}

  virtual void PostDrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount, HRESULT& res) {}

  virtual void PostDrawIndexedPrimitive(D3DPRIMITIVETYPE, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount, HRESULT& res) {}

  virtual void PostDrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride, HRESULT& res) {}

  virtual void PostDrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride, HRESULT& res) {}

  virtual void PostProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags, HRESULT& res) {}

  virtual void PostCreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl, HRESULT& res) {}

  virtual void PostSetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl, HRESULT& res) {}

  virtual void PostGetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl, HRESULT& res) {}

  virtual void PostSetFVF(DWORD FVF, HRESULT& res) {}

  virtual void PostGetFVF(DWORD* pFVF, HRESULT& res) {}

  virtual void PostCreateVertexShader(CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader, HRESULT& res) {}

  virtual void PostSetVertexShader(IDirect3DVertexShader9* pShader, HRESULT& res) {}

  virtual void PostGetVertexShader(IDirect3DVertexShader9** ppShader, HRESULT& res) {}

  virtual void PostSetVertexShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount, HRESULT& res) {}

  virtual void PostGetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount, HRESULT& res) {}

  virtual void PostSetVertexShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount, HRESULT& res) {}

  virtual void PostGetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount, HRESULT& res) {}

  virtual void PostSetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount, HRESULT& res) {}

  virtual void PostGetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount, HRESULT& res) {}

  virtual void PostSetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride, HRESULT& res) {}

  virtual void PostGetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride, HRESULT& res) {}

  virtual void PostSetStreamSourceFreq(UINT StreamNumber, UINT Setting, HRESULT& res) {}

  virtual void PostGetStreamSourceFreq(UINT StreamNumber, UINT* pSetting, HRESULT& res) {}

  virtual void PostSetIndices(IDirect3DIndexBuffer9* pIndexData, HRESULT& res) {}

  virtual void PostGetIndices(IDirect3DIndexBuffer9** ppIndexData, HRESULT& res) {}

  virtual void PostCreatePixelShader(CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader, HRESULT& res) {}

  virtual void PostSetPixelShader(IDirect3DPixelShader9* pShader, HRESULT& res) {}

  virtual void PostGetPixelShader(IDirect3DPixelShader9** ppShader, HRESULT& res) {}

  virtual void PostSetPixelShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount, HRESULT& res) {}

  virtual void PostGetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount, HRESULT& res) {}

  virtual void PostSetPixelShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount, HRESULT& res) {}

  virtual void PostGetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount, HRESULT& res) {}

  virtual void PostSetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount, HRESULT& res) {}

  virtual void PostGetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount, HRESULT& res) {}

  virtual void PostDrawRectPatch(UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo, HRESULT& res) {}

  virtual void PostDrawTriPatch(UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo, HRESULT& res) {}

  virtual void PostDeletePatch(UINT Handle, HRESULT& res) {}

  virtual void PostCreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery, HRESULT& res) {}
};

struct MMDPluginDLL3: public MMDPluginDLL2
{
  virtual const char* getPluginTitle() const { return nullptr; }

  /// <summary>
  /// SetWindowsHookExでフックしたプロシージャです。通常はこちらを使って割り込みをしてください
  /// </summary>
  /// <param name="param"></param>
  virtual void WndProc(const CWPSTRUCT* param) {}

  /// <summary>
  /// MMD側の呼び出しが制御できるプロシージャ
  /// </summary>
  /// <param name="hwnd"></param>
  /// <param name="uMsg"></param>
  /// <param name="wParam"></param>
  /// <param name="lParam"></param>
  /// <returns>
  /// false : MMDのプロシージャも呼ばれます。LRESULTは無視されます
  /// true  : MMDやその他のプラグインのプロシージャは呼ばれません。LRESULTが全体のプロシージャの戻り値として返されます。
  /// </returns>
  virtual std::pair<bool, LRESULT> WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) { return { false,0 }; }
};

#pragma warning( pop )

namespace mmp
{
  namespace detail
  {
    struct IFuncBinderImpl
    {
      MMD_DLL_FUNC_API static void addPrev(std::shared_ptr<IFuncBinderImpl> impl, void* addr, void* new_func);
      MMD_DLL_FUNC_API static void deleteFunc(void* new_func);
    protected:
      IFuncBinderImpl() = default;
      virtual ~IFuncBinderImpl() = default;

      virtual void add_prev(std::shared_ptr<IFuncBinderImpl> prev) = 0;
    };

    template<class T>
    struct FuncBinderImpl;

    template<class Result, class... Args>
    struct FuncBinderImpl<Result(WINAPI*)(Args ...)> : IFuncBinderImpl, std::enable_shared_from_this<FuncBinderImpl<Result(WINAPI*)(Args ...)>>
    {
      using Func = Result(WINAPI*)(Args ...);

      FuncBinderImpl(std::string module_name, std::string func_name, Func new_func)
        : module_name_(module_name), func_name_(func_name), new_func_(new_func)
      {
        MMD_DLL_FUNC_API void* RewriteFunction(const char* szRewriteModuleName, const char* szRewriteFunctionName, void* pRewriteFunctionPointer, int ordinal);
        oridinal_func_ = static_cast<decltype(oridinal_func_)>(RewriteFunction(module_name.c_str(), func_name.c_str(), new_func, -1));
      }

      std::string module_name_, func_name_;
      Func oridinal_func_, new_func_;

      Result operator()(Args ... args) { return oridinal_func_(args...); }

      void add_prev(std::shared_ptr<IFuncBinderImpl> p) override
      {
        prev_ = std::dynamic_pointer_cast<FuncBinderImpl>(p);
      }

      void reset()
      {
        MMD_DLL_FUNC_API void* RewriteFunction(const char* szRewriteModuleName, const char* szRewriteFunctionName, void* pRewriteFunctionPointer, int ordinal);
        RewriteFunction(module_name_.c_str(), func_name_.c_str(), oridinal_func_, -1);
        if ( prev_ )
        {
          prev_->oridinal_func_ = oridinal_func_;
        }
      }

      ~FuncBinderImpl()
      {
        reset();
      }

      std::shared_ptr<FuncBinderImpl<Func>> prev_;
    };
  }

  template<class T>
  class WinAPIHooker;

  template<class Result, class... Args>
  class WinAPIHooker<Result(WINAPI*)(Args ...)>
  {
  public:
    WinAPIHooker() = default;
    WinAPIHooker(const WinAPIHooker&) = delete;

    ~WinAPIHooker()
    {
      reset();
    }

    using Func = Result(WINAPI*)(Args ...);

    bool hook(const char* module_name, const char* func_name, Func new_func)
    {
      impl_ = std::make_shared<detail::FuncBinderImpl<Func>>(module_name, func_name, new_func);
      detail::IFuncBinderImpl::addPrev(impl_, impl_->oridinal_func_, new_func);
      return true;
    }

    void reset()
    {
      if ( !impl_ ) return;
      detail::IFuncBinderImpl::deleteFunc(impl_->new_func_);
      impl_->reset();
    }

    Result operator()(Args ... args) { return (*impl_)(args...); }

  private:

    std::shared_ptr<detail::FuncBinderImpl<Func>> impl_;
  };
}

extern "C"
{
  MMD_PLUGIN_API int version();

  MMD_PLUGIN_API MMDPluginDLL1* create1(IDirect3DDevice9* device);
  MMD_PLUGIN_API void destroy1(MMDPluginDLL1* p);

  MMD_PLUGIN_API MMDPluginDLL2* create2(IDirect3DDevice9* device);
  MMD_PLUGIN_API void destroy2(MMDPluginDLL2* p);

  MMD_PLUGIN_API MMDPluginDLL3* create3(IDirect3DDevice9* device);
  MMD_PLUGIN_API void destroy3(MMDPluginDLL2* p);
}
