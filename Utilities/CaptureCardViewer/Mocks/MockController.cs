using MicrosoftDisplayCaptureTools.CaptureCard;
using MicrosoftDisplayCaptureTools.Framework;
using System;
using System.Threading.Tasks;
using Windows.Data.Json;
using Windows.Foundation;

namespace CaptureCardViewer.Mocks
{
	class MockController : IController, IControllerWithFirmware
	{
		public string Name => "Mock Controller";

		public string Version => "0.1.2.3";

		public ControllerFirmwareState FirmwareState { get; set; } = ControllerFirmwareState.UpdateRequired;

		public string FirmwareVersion { get; set; } = "2.3.4.5";

		public IDisplayInput[] EnumerateDisplayInputs()
		{
			return new IDisplayInput[] { new MockDisplayInput() };
		}

		public void SetConfigData(IJsonValue data)
		{
			throw new NotImplementedException();
		}

		public IAsyncAction UpdateFirmwareAsync()
		{
			return UpdateFirmwareTaskAsync().AsAsyncAction();
		}

		private async Task UpdateFirmwareTaskAsync()
		{
			await Task.Delay(3000);
			FirmwareVersion = "3.2.1.0";
			FirmwareState = ControllerFirmwareState.UpToDate;
		}
	}

	public class MockDisplayInput : IDisplayInput
	{
		private class MockDisplayInputCaps : ICaptureCapabilities
		{
			public bool CanCaptureFrameSeries() => true;

			public bool CanConfigureDisplayID() => true;

			public bool CanConfigureEDID() => true;

			public bool CanHotPlug() => true;

			public bool CanReturnFramesToHost() => true;

			public bool CanReturnRawFramesToHost() => true;

			public uint GetMaxDescriptorSize() => 512;
		}

		public string Name => "HDMI 2.0 Input";

		public IDisplayCapture CaptureFrame()
		{
			throw new NotImplementedException();
		}

		public void FinalizeDisplayState()
		{
			throw new NotImplementedException();
		}

		public ICaptureCapabilities GetCapabilities()
		{
			return new MockDisplayInputCaps();
		}

		public ICaptureTrigger GetCaptureTrigger()
		{
			throw new NotImplementedException();
		}

		public void SetDescriptor(IMonitorDescriptor descriptor)
		{
			throw new NotImplementedException();
		}
	}
}
