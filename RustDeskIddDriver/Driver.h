#pragma once

#define NOMINMAX
#include <windows.h>
#include <bugcodes.h>
#include <wudfwdm.h>
#include <wdf.h>
#include <iddcx.h>

#include <dxgi1_5.h>
#include <d3d11_2.h>
#include <avrt.h>
#include <wrl.h>

#include <memory>
#include <vector>

#include "Trace.h"
#include "Public.h"

namespace Microsoft
{
    namespace WRL
    {
        namespace Wrappers
        {
            // Adds a wrapper for thread handles to the existing set of WRL handle wrapper classes
            typedef HandleT<HandleTraits::HANDLENullTraits> Thread;
        }
    }
}

namespace Microsoft
{
    namespace IndirectDisp
    {
        /// <summary>
        /// Manages the creation and lifetime of a Direct3D render device.
        /// </summary>
        struct SampleMonitorMode {
            DWORD Width;
            DWORD Height;
            DWORD VSync;
        };

        /// <summary>
        /// Manages the creation and lifetime of a Direct3D render device.
        /// </summary>
        struct Direct3DDevice
        {
            Direct3DDevice(LUID AdapterLuid);
            Direct3DDevice();
            HRESULT Init();

            LUID AdapterLuid;
            Microsoft::WRL::ComPtr<IDXGIFactory5> DxgiFactory;
            Microsoft::WRL::ComPtr<IDXGIAdapter1> Adapter;
            Microsoft::WRL::ComPtr<ID3D11Device> Device;
            Microsoft::WRL::ComPtr<ID3D11DeviceContext> DeviceContext;
        };

        /// <summary>
        /// Manages a thread that consumes buffers from an indirect display swap-chain object.
        /// </summary>
        class SwapChainProcessor
        {
        public:
            SwapChainProcessor(IDDCX_SWAPCHAIN hSwapChain, std::shared_ptr<Direct3DDevice> Device, HANDLE NewFrameEvent);
            ~SwapChainProcessor();

        private:
            static DWORD CALLBACK RunThread(LPVOID Argument);

            void Run();
            void RunCore();

            IDDCX_SWAPCHAIN m_hSwapChain;
            std::shared_ptr<Direct3DDevice> m_Device;
            HANDLE m_hAvailableBufferEvent;
            Microsoft::WRL::Wrappers::Thread m_hThread;
            Microsoft::WRL::Wrappers::Event m_hTerminateEvent;
        };

        /// <summary>
        /// Provides a sample implementation of an indirect display driver.
        /// </summary>
        class IndirectDeviceContext
        {
        public:
            IndirectDeviceContext(_In_ WDFDEVICE WdfDevice);
            virtual ~IndirectDeviceContext();

            void InitAdapter();
            void FinishInit(UINT ConnectorIndex);

            inline void SetAdapterInitStatus(NTSTATUS Status)
            {
                m_AdapterInitStatus = Status;
            }

            NTSTATUS PlugInMonitor(PCtlPlugIn Param);
            NTSTATUS PlugOutMonitor(PCtlPlugOut Param);

            NTSTATUS UpdateMonitorModes(PCtlMonitorModes Param);

            NTSTATUS CheckIndex(UINT index);

            static UINT GetMaxMonitorCount()
            {
                return m_sMaxMonitorCount;
            }

        protected:
            static constexpr UINT m_sMaxMonitorCount = 1;
            IDDCX_MONITOR m_Monitors[m_sMaxMonitorCount];

            NTSTATUS m_AdapterInitStatus;

            WDFDEVICE m_WdfDevice;
            IDDCX_ADAPTER m_Adapter;
        };

        class IndirectMonitorContext
        {
        public:
            IndirectMonitorContext(_In_ IDDCX_MONITOR Monitor);
            virtual ~IndirectMonitorContext();

            void AssignSwapChain(IDDCX_SWAPCHAIN SwapChain, LUID RenderAdapter, HANDLE NewFrameEvent);
            void UnassignSwapChain();

        private:
            IDDCX_MONITOR m_Monitor;
            std::unique_ptr<SwapChainProcessor> m_ProcessingThread;
        } ;
    }
}
