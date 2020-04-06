

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

#ifndef __softgpuidl_h__
#define __softgpuidl_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __ISoftGpuSource_FWD_DEFINED__
#define __ISoftGpuSource_FWD_DEFINED__
typedef interface ISoftGpuSource ISoftGpuSource;

#endif 	/* __ISoftGpuSource_FWD_DEFINED__ */


#ifndef __ISoftGpuTarget_FWD_DEFINED__
#define __ISoftGpuTarget_FWD_DEFINED__
typedef interface ISoftGpuTarget ISoftGpuTarget;

#endif 	/* __ISoftGpuTarget_FWD_DEFINED__ */


#ifndef __ISoftGpuMonitor_FWD_DEFINED__
#define __ISoftGpuMonitor_FWD_DEFINED__
typedef interface ISoftGpuMonitor ISoftGpuMonitor;

#endif 	/* __ISoftGpuMonitor_FWD_DEFINED__ */


#ifndef __ISoftGpuAdapter_FWD_DEFINED__
#define __ISoftGpuAdapter_FWD_DEFINED__
typedef interface ISoftGpuAdapter ISoftGpuAdapter;

#endif 	/* __ISoftGpuAdapter_FWD_DEFINED__ */


#ifndef __ISoftGpuConfig_FWD_DEFINED__
#define __ISoftGpuConfig_FWD_DEFINED__
typedef interface ISoftGpuConfig ISoftGpuConfig;

#endif 	/* __ISoftGpuConfig_FWD_DEFINED__ */


#ifndef __ISoftGpuLogSink_FWD_DEFINED__
#define __ISoftGpuLogSink_FWD_DEFINED__
typedef interface ISoftGpuLogSink ISoftGpuLogSink;

#endif 	/* __ISoftGpuLogSink_FWD_DEFINED__ */


#ifndef __ISoftGpuApi_FWD_DEFINED__
#define __ISoftGpuApi_FWD_DEFINED__
typedef interface ISoftGpuApi ISoftGpuApi;

#endif 	/* __ISoftGpuApi_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "dxgi1_2.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_softgpuidl_0000_0000 */
/* [local] */ 







typedef 
enum SOFTGPU_DEVID_
    {
        SOFTGPU_DEVID_FULL_D3D9L_	= 0x80,
        SOFTGPU_DEVID_VIRTUAL_D3D12_	= 0x81,
        SOFTGPU_DEVID_COMPUTE_ONLY_	= 0x86,
        SOFTGPU_DEVID_FULL_D3D12_	= 0x87,
        SOFTGPU_DEVID_RENDER_ONLY_	= 0x88,
        SOFTGPU_DEVID_DISPLAY_ONLY_	= 0x89,
        SOFTGPU_DEVID_INDIRECT_DISPLAY_V10_	= 0x90,
        SOFTGPU_DEVID_INDIRECT_DISPLAY_V12_	= 0x91,
        SOFTGPU_DEVID_INDIRECT_DISPLAY_V13_	= 0x92,
        SOFTGPU_DEVID_INDIRECT_DISPLAY_V14_	= 0x93,
        SOFTGPU_DEVID_RESERVED_	= 0x94,
        SOFTGPU_DEVID_INDIRECT_DISPLAY_V16_	= 0x95,
        SOFTGPU_DEVID_INDIRECT_DISPLAY_	= SOFTGPU_DEVID_INDIRECT_DISPLAY_V16_
    } 	SOFTGPU_DEVID_;

typedef 
enum SOFTGPU_BRIGHTNESS_VERSION_
    {
        SOFTGPU_BRIGHTNESS_NONE_	= 0,
        SOFTGPU_BRIGHTNESS_1_	= 1,
        SOFTGPU_BRIGHTNESS_2_	= 2,
        SOFTGPU_BRIGHTNESS_3_	= 3
    } 	SOFTGPU_BRIGHTNESS_VERSION_;

typedef struct SOFTGPU_BRIGHTNESS_NIT_RANGE_
    {
    UINT MinimumLevelMillinit;
    UINT MaximumLevelMillinit;
    UINT StepSizeMillinit;
    } 	SOFTGPU_BRIGHTNESS_NIT_RANGE_;

typedef 
enum WDDM_VERSION_
    {
        WDDM_V10_	= 0,
        WDDM_V12_	= ( WDDM_V10_ + 1 ) ,
        WDDM_V13_	= ( WDDM_V12_ + 1 ) ,
        WDDM_V20_	= ( WDDM_V13_ + 1 ) ,
        WDDM_V21_	= ( WDDM_V20_ + 1 ) ,
        WDDM_V22_	= ( WDDM_V21_ + 1 ) ,
        WDDM_V23_	= ( WDDM_V22_ + 1 ) ,
        WDDM_V24_	= ( WDDM_V23_ + 1 ) ,
        WDDM_V25_	= ( WDDM_V24_ + 1 ) ,
        WDDM_V26_	= ( WDDM_V25_ + 1 ) ,
        WDDM_V27_	= ( WDDM_V26_ + 1 ) ,
        WDDM_VERSIONS_CNT_	= ( WDDM_V27_ + 1 ) ,
        WDDM_VLATEST_	= WDDM_V27_,
        WDDM_INDIRECTDISPLAY_	= WDDM_V10_
    } 	WDDM_VERSION_;

typedef 
enum INDIRECTDISPLAY_VERSION_
    {
        INDIRECTDISPLAY_V10_	= 0,
        INDIRECTDISPLAY_V11_	= 1
    } 	INDIRECTDISPLAY_VERSION_;

typedef 
enum HYBRID_MODE_
    {
        HYBRID_NONE_	= 0,
        HYBRID_DISCRETE_	= ( HYBRID_NONE_ + 1 ) ,
        HYBRID_INTEGRATED_	= ( HYBRID_DISCRETE_ + 1 ) 
    } 	HYBRID_MODE_;

typedef 
enum GPU_MODE_
    {
        GPU_MODE_GPUMMU_	= 0,
        GPU_MODE_IOMMU_	= ( GPU_MODE_GPUMMU_ + 1 ) ,
        GPU_MODE_PHYSICAL_	= ( GPU_MODE_IOMMU_ + 1 ) 
    } 	GPU_MODE_;

typedef 
enum MONITOR_TYPE_
    {
        MONITOR_1080P_	= 0,
        MONITOR_1680x1050_	= ( MONITOR_1080P_ + 1 ) ,
        MONITOR_1600x1200_	= ( MONITOR_1680x1050_ + 1 ) ,
        MONITOR_1280x1024_	= ( MONITOR_1600x1200_ + 1 ) 
    } 	MONITOR_TYPE_;

typedef struct SOFTGPU_CREATE_PARAMS_
    {
    SOFTGPU_DEVID_ SoftGpuDriverType;
    WDDM_VERSION_ WddmVersion;
    HYBRID_MODE_ HybridMode;
    BOOL bBootPersisted;
    UINT NumDisplaySources;
    UINT NumDisplayTargets;
    BOOL VmsSupported;
    BOOL CreateDefaultMonitor;
    BOOL BasicMpoSupport;
    SIZE_T VidMemSize;
    SIZE_T ApertureSize;
    GPU_MODE_ GpuMode;
    BOOL SupportHwScheduling;
    UINT16 NumVidMems;
    BOOL UseContiguousVPR;
    BOOL MovePagingSupported;
    BOOL ExposeVideoCaptureChild;
    } 	SOFTGPU_CREATE_PARAMS;



extern RPC_IF_HANDLE __MIDL_itf_softgpuidl_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_softgpuidl_0000_0000_v0_0_s_ifspec;

#ifndef __ISoftGpuSource_INTERFACE_DEFINED__
#define __ISoftGpuSource_INTERFACE_DEFINED__

/* interface ISoftGpuSource */
/* [local][unique][uuid][object] */ 


EXTERN_C const IID IID_ISoftGpuSource;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6E444A86-792A-41C5-ABE6-7C95E596A97D")
    ISoftGpuSource : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetId( 
            /* [retval][out] */ UINT *Value) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReadPrimary( 
            RECT CopyRect,
            void *Buffer,
            DWORD BufferSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetModeDesc( 
            /* [retval][out] */ DXGI_MODE_DESC *ModeDesc) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct ISoftGpuSourceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISoftGpuSource * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISoftGpuSource * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISoftGpuSource * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetId )( 
            ISoftGpuSource * This,
            /* [retval][out] */ UINT *Value);
        
        HRESULT ( STDMETHODCALLTYPE *ReadPrimary )( 
            ISoftGpuSource * This,
            RECT CopyRect,
            void *Buffer,
            DWORD BufferSize);
        
        HRESULT ( STDMETHODCALLTYPE *GetModeDesc )( 
            ISoftGpuSource * This,
            /* [retval][out] */ DXGI_MODE_DESC *ModeDesc);
        
        END_INTERFACE
    } ISoftGpuSourceVtbl;

    interface ISoftGpuSource
    {
        CONST_VTBL struct ISoftGpuSourceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISoftGpuSource_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ISoftGpuSource_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ISoftGpuSource_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ISoftGpuSource_GetId(This,Value)	\
    ( (This)->lpVtbl -> GetId(This,Value) ) 

#define ISoftGpuSource_ReadPrimary(This,CopyRect,Buffer,BufferSize)	\
    ( (This)->lpVtbl -> ReadPrimary(This,CopyRect,Buffer,BufferSize) ) 

#define ISoftGpuSource_GetModeDesc(This,ModeDesc)	\
    ( (This)->lpVtbl -> GetModeDesc(This,ModeDesc) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ISoftGpuSource_INTERFACE_DEFINED__ */


#ifndef __ISoftGpuTarget_INTERFACE_DEFINED__
#define __ISoftGpuTarget_INTERFACE_DEFINED__

/* interface ISoftGpuTarget */
/* [local][unique][uuid][object] */ 


EXTERN_C const IID IID_ISoftGpuTarget;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E5BE434E-7DE3-4135-B6AC-EBF46BE26814")
    ISoftGpuTarget : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetId( 
            /* [retval][out] */ UINT *Value) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ConnectMonitor( 
            ISoftGpuMonitor *Monitor) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DisconnectMonitor( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetNitRanges( 
            SOFTGPU_BRIGHTNESS_NIT_RANGE_ *NitRanges,
            UINT TotalRangeCount,
            UINT NormalRangeCount) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct ISoftGpuTargetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISoftGpuTarget * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISoftGpuTarget * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISoftGpuTarget * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetId )( 
            ISoftGpuTarget * This,
            /* [retval][out] */ UINT *Value);
        
        HRESULT ( STDMETHODCALLTYPE *ConnectMonitor )( 
            ISoftGpuTarget * This,
            ISoftGpuMonitor *Monitor);
        
        HRESULT ( STDMETHODCALLTYPE *DisconnectMonitor )( 
            ISoftGpuTarget * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetNitRanges )( 
            ISoftGpuTarget * This,
            SOFTGPU_BRIGHTNESS_NIT_RANGE_ *NitRanges,
            UINT TotalRangeCount,
            UINT NormalRangeCount);
        
        END_INTERFACE
    } ISoftGpuTargetVtbl;

    interface ISoftGpuTarget
    {
        CONST_VTBL struct ISoftGpuTargetVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISoftGpuTarget_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ISoftGpuTarget_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ISoftGpuTarget_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ISoftGpuTarget_GetId(This,Value)	\
    ( (This)->lpVtbl -> GetId(This,Value) ) 

#define ISoftGpuTarget_ConnectMonitor(This,Monitor)	\
    ( (This)->lpVtbl -> ConnectMonitor(This,Monitor) ) 

#define ISoftGpuTarget_DisconnectMonitor(This)	\
    ( (This)->lpVtbl -> DisconnectMonitor(This) ) 

#define ISoftGpuTarget_SetNitRanges(This,NitRanges,TotalRangeCount,NormalRangeCount)	\
    ( (This)->lpVtbl -> SetNitRanges(This,NitRanges,TotalRangeCount,NormalRangeCount) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ISoftGpuTarget_INTERFACE_DEFINED__ */


#ifndef __ISoftGpuMonitor_INTERFACE_DEFINED__
#define __ISoftGpuMonitor_INTERFACE_DEFINED__

/* interface ISoftGpuMonitor */
/* [local][unique][uuid][object] */ 


EXTERN_C const IID IID_ISoftGpuMonitor;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7EBDDB51-3C84-4CED-ACBA-C2CFAA8D14B7")
    ISoftGpuMonitor : public IUnknown
    {
    public:
    };
    
    
#else 	/* C style interface */

    typedef struct ISoftGpuMonitorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISoftGpuMonitor * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISoftGpuMonitor * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISoftGpuMonitor * This);
        
        END_INTERFACE
    } ISoftGpuMonitorVtbl;

    interface ISoftGpuMonitor
    {
        CONST_VTBL struct ISoftGpuMonitorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISoftGpuMonitor_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ISoftGpuMonitor_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ISoftGpuMonitor_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ISoftGpuMonitor_INTERFACE_DEFINED__ */


#ifndef __ISoftGpuAdapter_INTERFACE_DEFINED__
#define __ISoftGpuAdapter_INTERFACE_DEFINED__

/* interface ISoftGpuAdapter */
/* [local][unique][uuid][object] */ 


EXTERN_C const IID IID_ISoftGpuAdapter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5F125519-9657-4B4D-943D-FD783FF6C4C9")
    ISoftGpuAdapter : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Destroy( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSource( 
            UINT SourceIndex,
            /* [retval][out] */ ISoftGpuSource **Source) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTarget( 
            UINT TargetIndex,
            /* [retval][out] */ ISoftGpuTarget **Target) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetUniqueName( 
            /* [retval][out] */ BSTR *Name) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFriendlyName( 
            /* [retval][out] */ BSTR *Name) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLuid( 
            /* [retval][out] */ LUID *Luid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPnpState( 
            BOOL IsStarted) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPnpState( 
            /* [retval][out] */ BOOL *Value) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBootPersistent( 
            /* [retval][out] */ BOOL *Value) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetIsIndirect( 
            /* [retval][out] */ BOOL *Value) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Escape( 
            DWORD BufferSize,
            BYTE *Buffer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetInstanceId( 
            /* [retval][out] */ BSTR *InstanceId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SurpriseRemove( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnableXGPU( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetBrightnessInterfaceVersion( 
            SOFTGPU_BRIGHTNESS_VERSION_ InterfaceVersion) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetHandle( 
            /* [retval][out] */ UINT *Handle) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct ISoftGpuAdapterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISoftGpuAdapter * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISoftGpuAdapter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISoftGpuAdapter * This);
        
        HRESULT ( STDMETHODCALLTYPE *Destroy )( 
            ISoftGpuAdapter * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetSource )( 
            ISoftGpuAdapter * This,
            UINT SourceIndex,
            /* [retval][out] */ ISoftGpuSource **Source);
        
        HRESULT ( STDMETHODCALLTYPE *GetTarget )( 
            ISoftGpuAdapter * This,
            UINT TargetIndex,
            /* [retval][out] */ ISoftGpuTarget **Target);
        
        HRESULT ( STDMETHODCALLTYPE *GetUniqueName )( 
            ISoftGpuAdapter * This,
            /* [retval][out] */ BSTR *Name);
        
        HRESULT ( STDMETHODCALLTYPE *GetFriendlyName )( 
            ISoftGpuAdapter * This,
            /* [retval][out] */ BSTR *Name);
        
        HRESULT ( STDMETHODCALLTYPE *GetLuid )( 
            ISoftGpuAdapter * This,
            /* [retval][out] */ LUID *Luid);
        
        HRESULT ( STDMETHODCALLTYPE *SetPnpState )( 
            ISoftGpuAdapter * This,
            BOOL IsStarted);
        
        HRESULT ( STDMETHODCALLTYPE *GetPnpState )( 
            ISoftGpuAdapter * This,
            /* [retval][out] */ BOOL *Value);
        
        HRESULT ( STDMETHODCALLTYPE *GetBootPersistent )( 
            ISoftGpuAdapter * This,
            /* [retval][out] */ BOOL *Value);
        
        HRESULT ( STDMETHODCALLTYPE *GetIsIndirect )( 
            ISoftGpuAdapter * This,
            /* [retval][out] */ BOOL *Value);
        
        HRESULT ( STDMETHODCALLTYPE *Escape )( 
            ISoftGpuAdapter * This,
            DWORD BufferSize,
            BYTE *Buffer);
        
        HRESULT ( STDMETHODCALLTYPE *GetInstanceId )( 
            ISoftGpuAdapter * This,
            /* [retval][out] */ BSTR *InstanceId);
        
        HRESULT ( STDMETHODCALLTYPE *SurpriseRemove )( 
            ISoftGpuAdapter * This);
        
        HRESULT ( STDMETHODCALLTYPE *EnableXGPU )( 
            ISoftGpuAdapter * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetBrightnessInterfaceVersion )( 
            ISoftGpuAdapter * This,
            SOFTGPU_BRIGHTNESS_VERSION_ InterfaceVersion);
        
        HRESULT ( STDMETHODCALLTYPE *GetHandle )( 
            ISoftGpuAdapter * This,
            /* [retval][out] */ UINT *Handle);
        
        END_INTERFACE
    } ISoftGpuAdapterVtbl;

    interface ISoftGpuAdapter
    {
        CONST_VTBL struct ISoftGpuAdapterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISoftGpuAdapter_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ISoftGpuAdapter_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ISoftGpuAdapter_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ISoftGpuAdapter_Destroy(This)	\
    ( (This)->lpVtbl -> Destroy(This) ) 

#define ISoftGpuAdapter_GetSource(This,SourceIndex,Source)	\
    ( (This)->lpVtbl -> GetSource(This,SourceIndex,Source) ) 

#define ISoftGpuAdapter_GetTarget(This,TargetIndex,Target)	\
    ( (This)->lpVtbl -> GetTarget(This,TargetIndex,Target) ) 

#define ISoftGpuAdapter_GetUniqueName(This,Name)	\
    ( (This)->lpVtbl -> GetUniqueName(This,Name) ) 

#define ISoftGpuAdapter_GetFriendlyName(This,Name)	\
    ( (This)->lpVtbl -> GetFriendlyName(This,Name) ) 

#define ISoftGpuAdapter_GetLuid(This,Luid)	\
    ( (This)->lpVtbl -> GetLuid(This,Luid) ) 

#define ISoftGpuAdapter_SetPnpState(This,IsStarted)	\
    ( (This)->lpVtbl -> SetPnpState(This,IsStarted) ) 

#define ISoftGpuAdapter_GetPnpState(This,Value)	\
    ( (This)->lpVtbl -> GetPnpState(This,Value) ) 

#define ISoftGpuAdapter_GetBootPersistent(This,Value)	\
    ( (This)->lpVtbl -> GetBootPersistent(This,Value) ) 

#define ISoftGpuAdapter_GetIsIndirect(This,Value)	\
    ( (This)->lpVtbl -> GetIsIndirect(This,Value) ) 

#define ISoftGpuAdapter_Escape(This,BufferSize,Buffer)	\
    ( (This)->lpVtbl -> Escape(This,BufferSize,Buffer) ) 

#define ISoftGpuAdapter_GetInstanceId(This,InstanceId)	\
    ( (This)->lpVtbl -> GetInstanceId(This,InstanceId) ) 

#define ISoftGpuAdapter_SurpriseRemove(This)	\
    ( (This)->lpVtbl -> SurpriseRemove(This) ) 

#define ISoftGpuAdapter_EnableXGPU(This)	\
    ( (This)->lpVtbl -> EnableXGPU(This) ) 

#define ISoftGpuAdapter_SetBrightnessInterfaceVersion(This,InterfaceVersion)	\
    ( (This)->lpVtbl -> SetBrightnessInterfaceVersion(This,InterfaceVersion) ) 

#define ISoftGpuAdapter_GetHandle(This,Handle)	\
    ( (This)->lpVtbl -> GetHandle(This,Handle) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ISoftGpuAdapter_INTERFACE_DEFINED__ */


#ifndef __ISoftGpuConfig_INTERFACE_DEFINED__
#define __ISoftGpuConfig_INTERFACE_DEFINED__

/* interface ISoftGpuConfig */
/* [local][unique][uuid][object] */ 


EXTERN_C const IID IID_ISoftGpuConfig;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7F9B3E23-9C4A-4CC4-9A9A-0FB656AA8BEF")
    ISoftGpuConfig : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetActive( 
            BOOL IsActive) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumAdapters( 
            UINT AdapterIndex,
            /* [retval][out] */ ISoftGpuAdapter **Adapter) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LoadConfigFromXml( 
            LPCWSTR XmlBlob) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateSoftGpuAdapterTemp( 
            LPCWSTR Name,
            UINT Sources,
            UINT Targets,
            /* [retval][out] */ ISoftGpuAdapter **Adapter) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateSoftGpuAdapterFromXml( 
            LPCWSTR XmlBlob,
            /* [retval][out] */ ISoftGpuAdapter **Adapter) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateMonitor( 
            LPCWSTR Name,
            BYTE *Edid,
            UINT EdidLength,
            /* [retval][out] */ ISoftGpuMonitor **Monitor) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateSoftGpuAdapter( 
            LPCWSTR Name,
            SOFTGPU_CREATE_PARAMS *pSoftGpuParams,
            /* [retval][out] */ ISoftGpuAdapter **Adapter) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateMonitorFromType( 
            LPCWSTR Name,
            MONITOR_TYPE_ Type,
            /* [retval][out] */ ISoftGpuMonitor **Monitor) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetHmdState( 
            BOOL bEnableAsHmd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetActiveEx( 
            BOOL IsActive,
            BOOL EnforceWaitForPresent,
            BOOL EnforceActivePathCheck) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct ISoftGpuConfigVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISoftGpuConfig * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISoftGpuConfig * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISoftGpuConfig * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetActive )( 
            ISoftGpuConfig * This,
            BOOL IsActive);
        
        HRESULT ( STDMETHODCALLTYPE *EnumAdapters )( 
            ISoftGpuConfig * This,
            UINT AdapterIndex,
            /* [retval][out] */ ISoftGpuAdapter **Adapter);
        
        HRESULT ( STDMETHODCALLTYPE *LoadConfigFromXml )( 
            ISoftGpuConfig * This,
            LPCWSTR XmlBlob);
        
        HRESULT ( STDMETHODCALLTYPE *CreateSoftGpuAdapterTemp )( 
            ISoftGpuConfig * This,
            LPCWSTR Name,
            UINT Sources,
            UINT Targets,
            /* [retval][out] */ ISoftGpuAdapter **Adapter);
        
        HRESULT ( STDMETHODCALLTYPE *CreateSoftGpuAdapterFromXml )( 
            ISoftGpuConfig * This,
            LPCWSTR XmlBlob,
            /* [retval][out] */ ISoftGpuAdapter **Adapter);
        
        HRESULT ( STDMETHODCALLTYPE *CreateMonitor )( 
            ISoftGpuConfig * This,
            LPCWSTR Name,
            BYTE *Edid,
            UINT EdidLength,
            /* [retval][out] */ ISoftGpuMonitor **Monitor);
        
        HRESULT ( STDMETHODCALLTYPE *CreateSoftGpuAdapter )( 
            ISoftGpuConfig * This,
            LPCWSTR Name,
            SOFTGPU_CREATE_PARAMS *pSoftGpuParams,
            /* [retval][out] */ ISoftGpuAdapter **Adapter);
        
        HRESULT ( STDMETHODCALLTYPE *CreateMonitorFromType )( 
            ISoftGpuConfig * This,
            LPCWSTR Name,
            MONITOR_TYPE_ Type,
            /* [retval][out] */ ISoftGpuMonitor **Monitor);
        
        HRESULT ( STDMETHODCALLTYPE *SetHmdState )( 
            ISoftGpuConfig * This,
            BOOL bEnableAsHmd);
        
        HRESULT ( STDMETHODCALLTYPE *SetActiveEx )( 
            ISoftGpuConfig * This,
            BOOL IsActive,
            BOOL EnforceWaitForPresent,
            BOOL EnforceActivePathCheck);
        
        END_INTERFACE
    } ISoftGpuConfigVtbl;

    interface ISoftGpuConfig
    {
        CONST_VTBL struct ISoftGpuConfigVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISoftGpuConfig_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ISoftGpuConfig_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ISoftGpuConfig_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ISoftGpuConfig_SetActive(This,IsActive)	\
    ( (This)->lpVtbl -> SetActive(This,IsActive) ) 

#define ISoftGpuConfig_EnumAdapters(This,AdapterIndex,Adapter)	\
    ( (This)->lpVtbl -> EnumAdapters(This,AdapterIndex,Adapter) ) 

#define ISoftGpuConfig_LoadConfigFromXml(This,XmlBlob)	\
    ( (This)->lpVtbl -> LoadConfigFromXml(This,XmlBlob) ) 

#define ISoftGpuConfig_CreateSoftGpuAdapterTemp(This,Name,Sources,Targets,Adapter)	\
    ( (This)->lpVtbl -> CreateSoftGpuAdapterTemp(This,Name,Sources,Targets,Adapter) ) 

#define ISoftGpuConfig_CreateSoftGpuAdapterFromXml(This,XmlBlob,Adapter)	\
    ( (This)->lpVtbl -> CreateSoftGpuAdapterFromXml(This,XmlBlob,Adapter) ) 

#define ISoftGpuConfig_CreateMonitor(This,Name,Edid,EdidLength,Monitor)	\
    ( (This)->lpVtbl -> CreateMonitor(This,Name,Edid,EdidLength,Monitor) ) 

#define ISoftGpuConfig_CreateSoftGpuAdapter(This,Name,pSoftGpuParams,Adapter)	\
    ( (This)->lpVtbl -> CreateSoftGpuAdapter(This,Name,pSoftGpuParams,Adapter) ) 

#define ISoftGpuConfig_CreateMonitorFromType(This,Name,Type,Monitor)	\
    ( (This)->lpVtbl -> CreateMonitorFromType(This,Name,Type,Monitor) ) 

#define ISoftGpuConfig_SetHmdState(This,bEnableAsHmd)	\
    ( (This)->lpVtbl -> SetHmdState(This,bEnableAsHmd) ) 

#define ISoftGpuConfig_SetActiveEx(This,IsActive,EnforceWaitForPresent,EnforceActivePathCheck)	\
    ( (This)->lpVtbl -> SetActiveEx(This,IsActive,EnforceWaitForPresent,EnforceActivePathCheck) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ISoftGpuConfig_INTERFACE_DEFINED__ */


#ifndef __ISoftGpuLogSink_INTERFACE_DEFINED__
#define __ISoftGpuLogSink_INTERFACE_DEFINED__

/* interface ISoftGpuLogSink */
/* [local][unique][uuid][object] */ 


EXTERN_C const IID IID_ISoftGpuLogSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1CC507FE-C954-45FE-9B6A-1E751B577E46")
    ISoftGpuLogSink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE LogInfo( 
            BSTR Message) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LogError( 
            BSTR Message) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LogAssert( 
            BSTR Message) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct ISoftGpuLogSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISoftGpuLogSink * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISoftGpuLogSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISoftGpuLogSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *LogInfo )( 
            ISoftGpuLogSink * This,
            BSTR Message);
        
        HRESULT ( STDMETHODCALLTYPE *LogError )( 
            ISoftGpuLogSink * This,
            BSTR Message);
        
        HRESULT ( STDMETHODCALLTYPE *LogAssert )( 
            ISoftGpuLogSink * This,
            BSTR Message);
        
        END_INTERFACE
    } ISoftGpuLogSinkVtbl;

    interface ISoftGpuLogSink
    {
        CONST_VTBL struct ISoftGpuLogSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISoftGpuLogSink_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ISoftGpuLogSink_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ISoftGpuLogSink_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ISoftGpuLogSink_LogInfo(This,Message)	\
    ( (This)->lpVtbl -> LogInfo(This,Message) ) 

#define ISoftGpuLogSink_LogError(This,Message)	\
    ( (This)->lpVtbl -> LogError(This,Message) ) 

#define ISoftGpuLogSink_LogAssert(This,Message)	\
    ( (This)->lpVtbl -> LogAssert(This,Message) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ISoftGpuLogSink_INTERFACE_DEFINED__ */


#ifndef __ISoftGpuApi_INTERFACE_DEFINED__
#define __ISoftGpuApi_INTERFACE_DEFINED__

/* interface ISoftGpuApi */
/* [local][unique][uuid][object] */ 


EXTERN_C const IID IID_ISoftGpuApi;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("235E4AA1-9E2F-4F49-89F0-C583CABC5468")
    ISoftGpuApi : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateSystemConfig( 
            /* [retval][out] */ ISoftGpuConfig **Value) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetLogSink( 
            ISoftGpuLogSink *Sink) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct ISoftGpuApiVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISoftGpuApi * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISoftGpuApi * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISoftGpuApi * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateSystemConfig )( 
            ISoftGpuApi * This,
            /* [retval][out] */ ISoftGpuConfig **Value);
        
        HRESULT ( STDMETHODCALLTYPE *SetLogSink )( 
            ISoftGpuApi * This,
            ISoftGpuLogSink *Sink);
        
        END_INTERFACE
    } ISoftGpuApiVtbl;

    interface ISoftGpuApi
    {
        CONST_VTBL struct ISoftGpuApiVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISoftGpuApi_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ISoftGpuApi_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ISoftGpuApi_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ISoftGpuApi_CreateSystemConfig(This,Value)	\
    ( (This)->lpVtbl -> CreateSystemConfig(This,Value) ) 

#define ISoftGpuApi_SetLogSink(This,Sink)	\
    ( (This)->lpVtbl -> SetLogSink(This,Sink) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ISoftGpuApi_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_softgpuidl_0000_0007 */
/* [local] */ 

HRESULT __stdcall CreateSoftGpuApi(ISoftGpuApi** pApi);


extern RPC_IF_HANDLE __MIDL_itf_softgpuidl_0000_0007_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_softgpuidl_0000_0007_v0_0_s_ifspec;

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


