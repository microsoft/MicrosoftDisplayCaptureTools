

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0622 */
/* @@MIDL_FILE_HEADING(  ) */



/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

/* verify that the <rpcsal.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCSAL_H_VERSION__
#define __REQUIRED_RPCSAL_H_VERSION__ 100
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __peekidl_h__
#define __peekidl_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IPeekApi_FWD_DEFINED__
#define __IPeekApi_FWD_DEFINED__
typedef interface IPeekApi IPeekApi;

#endif 	/* __IPeekApi_FWD_DEFINED__ */


#ifndef __IPeekManager_FWD_DEFINED__
#define __IPeekManager_FWD_DEFINED__
typedef interface IPeekManager IPeekManager;

#endif 	/* __IPeekManager_FWD_DEFINED__ */


#ifndef __IFrameProvider_FWD_DEFINED__
#define __IFrameProvider_FWD_DEFINED__
typedef interface IFrameProvider IFrameProvider;

#endif 	/* __IFrameProvider_FWD_DEFINED__ */


#ifndef __IFrameCompositor_FWD_DEFINED__
#define __IFrameCompositor_FWD_DEFINED__
typedef interface IFrameCompositor IFrameCompositor;

#endif 	/* __IFrameCompositor_FWD_DEFINED__ */


#ifndef __IFrameProviderFrame_FWD_DEFINED__
#define __IFrameProviderFrame_FWD_DEFINED__
typedef interface IFrameProviderFrame IFrameProviderFrame;

#endif 	/* __IFrameProviderFrame_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "dxgi1_2.h"
#include "SoftGpuIdl.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_peekidl_0000_0000 */
/* [local] */ 








extern RPC_IF_HANDLE __MIDL_itf_peekidl_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_peekidl_0000_0000_v0_0_s_ifspec;

#ifndef __IPeekApi_INTERFACE_DEFINED__
#define __IPeekApi_INTERFACE_DEFINED__

/* interface IPeekApi */
/* [local][unique][uuid][object] */ 


EXTERN_C const IID IID_IPeekApi;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("067F8C50-9DB2-4E94-9DE4-BE5EBC11C00C")
    IPeekApi : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreatePeekManager( 
            /* [retval][out] */ IPeekManager **Value) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HmdPanic( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateFrameProviderFromGdiName( 
            LPCWSTR ViewName,
            /* [retval][out] */ IFrameProvider **pProvider) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateFrameProviderFromSource( 
            LPCWSTR AdapterName,
            UINT Source,
            /* [retval][out] */ IFrameProvider **pProvider) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateFrameCompositor( 
            UINT Width,
            UINT Height,
            /* [retval][out] */ IFrameCompositor **pCompositor) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IPeekApiVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPeekApi * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPeekApi * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPeekApi * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreatePeekManager )( 
            IPeekApi * This,
            /* [retval][out] */ IPeekManager **Value);
        
        HRESULT ( STDMETHODCALLTYPE *HmdPanic )( 
            IPeekApi * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateFrameProviderFromGdiName )( 
            IPeekApi * This,
            LPCWSTR ViewName,
            /* [retval][out] */ IFrameProvider **pProvider);
        
        HRESULT ( STDMETHODCALLTYPE *CreateFrameProviderFromSource )( 
            IPeekApi * This,
            LPCWSTR AdapterName,
            UINT Source,
            /* [retval][out] */ IFrameProvider **pProvider);
        
        HRESULT ( STDMETHODCALLTYPE *CreateFrameCompositor )( 
            IPeekApi * This,
            UINT Width,
            UINT Height,
            /* [retval][out] */ IFrameCompositor **pCompositor);
        
        END_INTERFACE
    } IPeekApiVtbl;

    interface IPeekApi
    {
        CONST_VTBL struct IPeekApiVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPeekApi_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IPeekApi_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IPeekApi_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IPeekApi_CreatePeekManager(This,Value)	\
    ( (This)->lpVtbl -> CreatePeekManager(This,Value) ) 

#define IPeekApi_HmdPanic(This)	\
    ( (This)->lpVtbl -> HmdPanic(This) ) 

#define IPeekApi_CreateFrameProviderFromGdiName(This,ViewName,pProvider)	\
    ( (This)->lpVtbl -> CreateFrameProviderFromGdiName(This,ViewName,pProvider) ) 

#define IPeekApi_CreateFrameProviderFromSource(This,AdapterName,Source,pProvider)	\
    ( (This)->lpVtbl -> CreateFrameProviderFromSource(This,AdapterName,Source,pProvider) ) 

#define IPeekApi_CreateFrameCompositor(This,Width,Height,pCompositor)	\
    ( (This)->lpVtbl -> CreateFrameCompositor(This,Width,Height,pCompositor) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IPeekApi_INTERFACE_DEFINED__ */


#ifndef __IPeekManager_INTERFACE_DEFINED__
#define __IPeekManager_INTERFACE_DEFINED__

/* interface IPeekManager */
/* [local][unique][uuid][object] */ 


EXTERN_C const IID IID_IPeekManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BC1F28A1-D915-47F8-B012-06720F0A404A")
    IPeekManager : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE InitializeForHwnd( 
            HWND Window) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InitializeForHmd( 
            LUID Adapter,
            UINT TargetId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Resize( 
            DWORD Width,
            DWORD Height) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RunMessageLoop( 
            INT_PTR AccelTable) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Present( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WaitForPresentOrMessage( 
            /* [retval][out] */ BOOL *Value) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetOverviewMode( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPanMode( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSingleOutputMode( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ZoomIn( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ZoomOut( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSingleOutputFromName( 
            LPCWSTR Name) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPanModeBoundsEnabled( 
            BOOL IsEnabled) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RecreateDesktop( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetRedundantViewName( 
            LPCWSTR Name) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IPeekManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPeekManager * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPeekManager * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPeekManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *InitializeForHwnd )( 
            IPeekManager * This,
            HWND Window);
        
        HRESULT ( STDMETHODCALLTYPE *InitializeForHmd )( 
            IPeekManager * This,
            LUID Adapter,
            UINT TargetId);
        
        HRESULT ( STDMETHODCALLTYPE *Resize )( 
            IPeekManager * This,
            DWORD Width,
            DWORD Height);
        
        HRESULT ( STDMETHODCALLTYPE *RunMessageLoop )( 
            IPeekManager * This,
            INT_PTR AccelTable);
        
        HRESULT ( STDMETHODCALLTYPE *Present )( 
            IPeekManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *WaitForPresentOrMessage )( 
            IPeekManager * This,
            /* [retval][out] */ BOOL *Value);
        
        HRESULT ( STDMETHODCALLTYPE *SetOverviewMode )( 
            IPeekManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetPanMode )( 
            IPeekManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetSingleOutputMode )( 
            IPeekManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *ZoomIn )( 
            IPeekManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *ZoomOut )( 
            IPeekManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetSingleOutputFromName )( 
            IPeekManager * This,
            LPCWSTR Name);
        
        HRESULT ( STDMETHODCALLTYPE *SetPanModeBoundsEnabled )( 
            IPeekManager * This,
            BOOL IsEnabled);
        
        HRESULT ( STDMETHODCALLTYPE *RecreateDesktop )( 
            IPeekManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetRedundantViewName )( 
            IPeekManager * This,
            LPCWSTR Name);
        
        END_INTERFACE
    } IPeekManagerVtbl;

    interface IPeekManager
    {
        CONST_VTBL struct IPeekManagerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPeekManager_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IPeekManager_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IPeekManager_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IPeekManager_InitializeForHwnd(This,Window)	\
    ( (This)->lpVtbl -> InitializeForHwnd(This,Window) ) 

#define IPeekManager_InitializeForHmd(This,Adapter,TargetId)	\
    ( (This)->lpVtbl -> InitializeForHmd(This,Adapter,TargetId) ) 

#define IPeekManager_Resize(This,Width,Height)	\
    ( (This)->lpVtbl -> Resize(This,Width,Height) ) 

#define IPeekManager_RunMessageLoop(This,AccelTable)	\
    ( (This)->lpVtbl -> RunMessageLoop(This,AccelTable) ) 

#define IPeekManager_Present(This)	\
    ( (This)->lpVtbl -> Present(This) ) 

#define IPeekManager_WaitForPresentOrMessage(This,Value)	\
    ( (This)->lpVtbl -> WaitForPresentOrMessage(This,Value) ) 

#define IPeekManager_SetOverviewMode(This)	\
    ( (This)->lpVtbl -> SetOverviewMode(This) ) 

#define IPeekManager_SetPanMode(This)	\
    ( (This)->lpVtbl -> SetPanMode(This) ) 

#define IPeekManager_SetSingleOutputMode(This)	\
    ( (This)->lpVtbl -> SetSingleOutputMode(This) ) 

#define IPeekManager_ZoomIn(This)	\
    ( (This)->lpVtbl -> ZoomIn(This) ) 

#define IPeekManager_ZoomOut(This)	\
    ( (This)->lpVtbl -> ZoomOut(This) ) 

#define IPeekManager_SetSingleOutputFromName(This,Name)	\
    ( (This)->lpVtbl -> SetSingleOutputFromName(This,Name) ) 

#define IPeekManager_SetPanModeBoundsEnabled(This,IsEnabled)	\
    ( (This)->lpVtbl -> SetPanModeBoundsEnabled(This,IsEnabled) ) 

#define IPeekManager_RecreateDesktop(This)	\
    ( (This)->lpVtbl -> RecreateDesktop(This) ) 

#define IPeekManager_SetRedundantViewName(This,Name)	\
    ( (This)->lpVtbl -> SetRedundantViewName(This,Name) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IPeekManager_INTERFACE_DEFINED__ */


#ifndef __IFrameProvider_INTERFACE_DEFINED__
#define __IFrameProvider_INTERFACE_DEFINED__

/* interface IFrameProvider */
/* [local][unique][uuid][object] */ 


EXTERN_C const IID IID_IFrameProvider;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CB6D138E-B017-417C-B83C-EB58A7E1CB9D")
    IFrameProvider : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateResources( 
            LUID RenderAdapter) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateResourcesForCpuAccess( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DestroyResources( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LockCurrentFrame( 
            IFrameProviderFrame **pFrame) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetWaitHandle( 
            /* [retval][out] */ HANDLE *pHandle) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IFrameProviderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFrameProvider * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFrameProvider * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFrameProvider * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateResources )( 
            IFrameProvider * This,
            LUID RenderAdapter);
        
        HRESULT ( STDMETHODCALLTYPE *CreateResourcesForCpuAccess )( 
            IFrameProvider * This);
        
        HRESULT ( STDMETHODCALLTYPE *DestroyResources )( 
            IFrameProvider * This);
        
        HRESULT ( STDMETHODCALLTYPE *LockCurrentFrame )( 
            IFrameProvider * This,
            IFrameProviderFrame **pFrame);
        
        HRESULT ( STDMETHODCALLTYPE *GetWaitHandle )( 
            IFrameProvider * This,
            /* [retval][out] */ HANDLE *pHandle);
        
        END_INTERFACE
    } IFrameProviderVtbl;

    interface IFrameProvider
    {
        CONST_VTBL struct IFrameProviderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFrameProvider_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IFrameProvider_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IFrameProvider_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IFrameProvider_CreateResources(This,RenderAdapter)	\
    ( (This)->lpVtbl -> CreateResources(This,RenderAdapter) ) 

#define IFrameProvider_CreateResourcesForCpuAccess(This)	\
    ( (This)->lpVtbl -> CreateResourcesForCpuAccess(This) ) 

#define IFrameProvider_DestroyResources(This)	\
    ( (This)->lpVtbl -> DestroyResources(This) ) 

#define IFrameProvider_LockCurrentFrame(This,pFrame)	\
    ( (This)->lpVtbl -> LockCurrentFrame(This,pFrame) ) 

#define IFrameProvider_GetWaitHandle(This,pHandle)	\
    ( (This)->lpVtbl -> GetWaitHandle(This,pHandle) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IFrameProvider_INTERFACE_DEFINED__ */


#ifndef __IFrameCompositor_INTERFACE_DEFINED__
#define __IFrameCompositor_INTERFACE_DEFINED__

/* interface IFrameCompositor */
/* [local][unique][uuid][object] */ 


EXTERN_C const IID IID_IFrameCompositor;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A363977C-6033-4F74-92E6-1354308C0970")
    IFrameCompositor : public IFrameProvider
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddFrameProvider( 
            IFrameProvider *Provider,
            POINT Offset) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IFrameCompositorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFrameCompositor * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFrameCompositor * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFrameCompositor * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateResources )( 
            IFrameCompositor * This,
            LUID RenderAdapter);
        
        HRESULT ( STDMETHODCALLTYPE *CreateResourcesForCpuAccess )( 
            IFrameCompositor * This);
        
        HRESULT ( STDMETHODCALLTYPE *DestroyResources )( 
            IFrameCompositor * This);
        
        HRESULT ( STDMETHODCALLTYPE *LockCurrentFrame )( 
            IFrameCompositor * This,
            IFrameProviderFrame **pFrame);
        
        HRESULT ( STDMETHODCALLTYPE *GetWaitHandle )( 
            IFrameCompositor * This,
            /* [retval][out] */ HANDLE *pHandle);
        
        HRESULT ( STDMETHODCALLTYPE *AddFrameProvider )( 
            IFrameCompositor * This,
            IFrameProvider *Provider,
            POINT Offset);
        
        END_INTERFACE
    } IFrameCompositorVtbl;

    interface IFrameCompositor
    {
        CONST_VTBL struct IFrameCompositorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFrameCompositor_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IFrameCompositor_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IFrameCompositor_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IFrameCompositor_CreateResources(This,RenderAdapter)	\
    ( (This)->lpVtbl -> CreateResources(This,RenderAdapter) ) 

#define IFrameCompositor_CreateResourcesForCpuAccess(This)	\
    ( (This)->lpVtbl -> CreateResourcesForCpuAccess(This) ) 

#define IFrameCompositor_DestroyResources(This)	\
    ( (This)->lpVtbl -> DestroyResources(This) ) 

#define IFrameCompositor_LockCurrentFrame(This,pFrame)	\
    ( (This)->lpVtbl -> LockCurrentFrame(This,pFrame) ) 

#define IFrameCompositor_GetWaitHandle(This,pHandle)	\
    ( (This)->lpVtbl -> GetWaitHandle(This,pHandle) ) 


#define IFrameCompositor_AddFrameProvider(This,Provider,Offset)	\
    ( (This)->lpVtbl -> AddFrameProvider(This,Provider,Offset) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IFrameCompositor_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_peekidl_0000_0004 */
/* [local] */ 


enum FrameProviderPlaneType
    {
        BasePlane	= 0,
        MultiOverlayPlane	= ( BasePlane + 1 ) ,
        LegacyOverlayPlane	= ( MultiOverlayPlane + 1 ) ,
        CursorPlane	= ( LegacyOverlayPlane + 1 ) 
    } ;
struct FrameProviderMatrix
    {
    FLOAT _11;
    FLOAT _12;
    FLOAT _13;
    FLOAT _21;
    FLOAT _22;
    FLOAT _23;
    FLOAT _31;
    FLOAT _32;
    FLOAT _33;
    FLOAT _41;
    FLOAT _42;
    FLOAT _43;
    } ;


extern RPC_IF_HANDLE __MIDL_itf_peekidl_0000_0004_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_peekidl_0000_0004_v0_0_s_ifspec;

#ifndef __IFrameProviderFrame_INTERFACE_DEFINED__
#define __IFrameProviderFrame_INTERFACE_DEFINED__

/* interface IFrameProviderFrame */
/* [local][unique][uuid][object] */ 


EXTERN_C const IID IID_IFrameProviderFrame;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("24E4D842-D6E9-40DB-958B-2829DB1B9E23")
    IFrameProviderFrame : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPlaneCount( 
            /* [retval][out] */ UINT *Value) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPlaneType( 
            UINT Plane,
            /* [retval][out] */ enum FrameProviderPlaneType *Type) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPlaneSurface( 
            UINT Plane,
            IDXGISurface **pSurface) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPlaneSurfaceCpu( 
            UINT Plane,
            BYTE **pData,
            UINT *pPitch) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPlaneMatrix( 
            UINT Plane,
            struct FrameProviderMatrix *pMatrix) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSourceSize( 
            /* [retval][out] */ SIZE *Value) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTargetSize( 
            /* [retval][out] */ SIZE *Value) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFrameTime( 
            /* [retval][out] */ LONGLONG *Value) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IFrameProviderFrameVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFrameProviderFrame * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFrameProviderFrame * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFrameProviderFrame * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPlaneCount )( 
            IFrameProviderFrame * This,
            /* [retval][out] */ UINT *Value);
        
        HRESULT ( STDMETHODCALLTYPE *GetPlaneType )( 
            IFrameProviderFrame * This,
            UINT Plane,
            /* [retval][out] */ enum FrameProviderPlaneType *Type);
        
        HRESULT ( STDMETHODCALLTYPE *GetPlaneSurface )( 
            IFrameProviderFrame * This,
            UINT Plane,
            IDXGISurface **pSurface);
        
        HRESULT ( STDMETHODCALLTYPE *GetPlaneSurfaceCpu )( 
            IFrameProviderFrame * This,
            UINT Plane,
            BYTE **pData,
            UINT *pPitch);
        
        HRESULT ( STDMETHODCALLTYPE *GetPlaneMatrix )( 
            IFrameProviderFrame * This,
            UINT Plane,
            struct FrameProviderMatrix *pMatrix);
        
        HRESULT ( STDMETHODCALLTYPE *GetSourceSize )( 
            IFrameProviderFrame * This,
            /* [retval][out] */ SIZE *Value);
        
        HRESULT ( STDMETHODCALLTYPE *GetTargetSize )( 
            IFrameProviderFrame * This,
            /* [retval][out] */ SIZE *Value);
        
        HRESULT ( STDMETHODCALLTYPE *GetFrameTime )( 
            IFrameProviderFrame * This,
            /* [retval][out] */ LONGLONG *Value);
        
        END_INTERFACE
    } IFrameProviderFrameVtbl;

    interface IFrameProviderFrame
    {
        CONST_VTBL struct IFrameProviderFrameVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFrameProviderFrame_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IFrameProviderFrame_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IFrameProviderFrame_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IFrameProviderFrame_GetPlaneCount(This,Value)	\
    ( (This)->lpVtbl -> GetPlaneCount(This,Value) ) 

#define IFrameProviderFrame_GetPlaneType(This,Plane,Type)	\
    ( (This)->lpVtbl -> GetPlaneType(This,Plane,Type) ) 

#define IFrameProviderFrame_GetPlaneSurface(This,Plane,pSurface)	\
    ( (This)->lpVtbl -> GetPlaneSurface(This,Plane,pSurface) ) 

#define IFrameProviderFrame_GetPlaneSurfaceCpu(This,Plane,pData,pPitch)	\
    ( (This)->lpVtbl -> GetPlaneSurfaceCpu(This,Plane,pData,pPitch) ) 

#define IFrameProviderFrame_GetPlaneMatrix(This,Plane,pMatrix)	\
    ( (This)->lpVtbl -> GetPlaneMatrix(This,Plane,pMatrix) ) 

#define IFrameProviderFrame_GetSourceSize(This,Value)	\
    ( (This)->lpVtbl -> GetSourceSize(This,Value) ) 

#define IFrameProviderFrame_GetTargetSize(This,Value)	\
    ( (This)->lpVtbl -> GetTargetSize(This,Value) ) 

#define IFrameProviderFrame_GetFrameTime(This,Value)	\
    ( (This)->lpVtbl -> GetFrameTime(This,Value) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IFrameProviderFrame_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_peekidl_0000_0005 */
/* [local] */ 

HRESULT __stdcall CreatePeekApi(IPeekApi** pApi);


extern RPC_IF_HANDLE __MIDL_itf_peekidl_0000_0005_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_peekidl_0000_0005_v0_0_s_ifspec;

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


