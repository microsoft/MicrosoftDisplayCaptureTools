using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using MicrosoftDisplayCaptureTools.CaptureCard;
using MicrosoftDisplayCaptureTools.Display;
using System;
using System.Collections.ObjectModel;
using System.Runtime.CompilerServices;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using Windows.Devices.Display;
using Windows.Devices.Display.Core;
using Windows.Devices.Enumeration;
using Windows.Foundation;
using Windows.Graphics.Imaging;
using WinRT;

namespace CaptureCardViewer.ViewModels
{
	public partial class CaptureSessionViewModel : ObservableObject
	{
		public WorkspaceViewModel Workspace { get; }

		/// <summary>
		/// The capture card used for capturing on the other side of the output.
		/// </summary>
		public CaptureCardViewModel CaptureCard { get; }
				
		/// <summary>
		/// The specific input on the capture card that is being captured.
		/// </summary>
		public IDisplayInput CaptureInput { get; }
		public string CaptureInputName => CaptureInput?.Name ?? "None";

		/// <summary>
		/// The engine used for rendering to the output.
		/// </summary>
		public IDisplayEngine Engine { get; }

		[ObservableProperty]
		[AlsoNotifyChangeFor(nameof(SelectedEngineOutputName))]
		IDisplayOutput? selectedEngineOutput;
		public string SelectedEngineOutputName => SelectedEngineOutput?.Target.StableMonitorId ?? "None";

		/// <summary>
		/// Represents a possible DisplayTarget that can be rendered to.
		/// </summary>
		public partial class AvailableEngineOutput : ObservableObject
		{
			CaptureSessionViewModel parent;
			
			public DisplayMonitor Monitor { get; }
			public DisplayTarget Target { get; }

			[ObservableProperty]
			[AlsoNotifyChangeFor(nameof(Name))]
			string adapterFriendlyName = "";

			public string Name => $"{Monitor?.DisplayName ?? Target.StableMonitorId} ({Monitor?.PhysicalConnector} port on {AdapterFriendlyName})";

			public AvailableEngineOutput(CaptureSessionViewModel parent, DisplayTarget target)
			{
				this.parent = parent;
				Target = target;
				Monitor = target.TryGetMonitor();

				InitAsync();
			}

			async void InitAsync()
			{
				var adapterInterface = await DeviceInformation.CreateFromIdAsync(Target.Adapter.DeviceInterfacePath, new[] { "System.Devices.DeviceInstanceId" });
				var adapterDeviceId = adapterInterface?.Properties["System.Devices.DeviceInstanceId"] as string;
				if (adapterDeviceId != null)
				{
					var adapterDevice = await DeviceInformation.CreateFromIdAsync(adapterDeviceId, new string[] { }, DeviceInformationKind.Device);
					AdapterFriendlyName = adapterDevice?.Name ?? "";
				}
			}

			[ICommand]
			void Connect()
			{
				parent.SelectedEngineOutput = parent.Engine.InitializeOutput(Target);
			}
		}

		[ObservableProperty]
		ImageSource? predictionSource;
		[ObservableProperty]
		ImageSource? captureSource;

		ObservableCollection<AvailableEngineOutput> availableEngineOutputs = new();
		public ReadOnlyObservableCollection<AvailableEngineOutput> AvailableEngineOutputs { get; }

		[ObservableProperty]
		int framesCaptured = 0;
		
		/// <summary>
		/// The live capture async task sets this to trigger the UI to update.
		/// </summary>
		[ObservableProperty]
		[AlsoNotifyChangeFor(nameof(CanStartLiveCapture))]
		[AlsoNotifyChangeFor(nameof(CanStopLiveCapture))]
		[AlsoNotifyChangeFor(nameof(CanSingleFrameCapture))]
		bool isRunningLiveCapture = false;

		public bool CanStartLiveCapture => CaptureInput != null && !isRunningLiveCapture;
		[ObservableProperty]
		bool canStopLiveCapture = false;
		public bool CanSingleFrameCapture => CanStartLiveCapture;

		public CaptureSessionViewModel(WorkspaceViewModel workspace, IDisplayEngine engine, CaptureCardViewModel captureCard, IDisplayInput input)
		{
			Workspace = workspace;
			Engine = engine;
			CaptureInput = input;
			CaptureCard = captureCard;

			// Enumerate available targets
			var dispManager = DisplayManager.Create(DisplayManagerOptions.None)!;
			foreach (var target in dispManager.GetCurrentTargets())
			{
				if (target.IsConnected)
				{
					availableEngineOutputs.Add(new AvailableEngineOutput(this, target));
				}
			}

			AvailableEngineOutputs = new ReadOnlyObservableCollection<AvailableEngineOutput>(availableEngineOutputs);
		}

		// Displaying frames & properties from the plugin
		[ICommand]
		async void CaptureSingleFrame()
		{
			// TODO: Encapsulate the active tools into a sub-property of this CaptureSessionViewModel
			//var activeTools = new List<Tool>();
			//Workspace.Toolboxes.

			var displayOutput = SelectedEngineOutput;

			BitmapSource? capturedBitmap = null;

			//Display & Capture of frames 
			await Task.Run(() =>
			{
				// Captured frames from the tanager board
				var captureInput = CaptureInput;
				captureInput.FinalizeDisplayState();

				var capturedFrame = captureInput.CaptureFrame();
				var capPixelBuffer = capturedFrame.GetRawPixelData();
				capturedBitmap = BufferToImgConv(capPixelBuffer);

				//Updating the UI by queuing up an operation on the UI thread
				/*this.dispatcher.Invoke(
						new Action(() =>
						{
							CaptImage.ImageSource = capSrc;
							PredImage.ImageSource = predSrc;
							capturedImageProperties.Text = "Refresh Rate: " + refreshRate.ToString() + "\r\n";
							capturedImageProperties.Text += "Resolution: " + resolution.Height.ToString() + "x" + resolution.Width.ToString() + "\r\n";
							capturedImageProperties.Text += "Source pixel format: " + mode.SourcePixelFormat.ToString() + "\r\n";

						}
						));*/
			});

			CaptureSource = capturedBitmap;
			FramesCaptured += 1;
		}

		[ICommand]
		async void StartLiveCapture()
		{
			FramesCaptured = 0;
		}

		[ICommand]
		async void StopLiveCapture()
		{
			IsRunningLiveCapture = false;
		}

		async void RenderToOutput()
		{
			var displayOutput = SelectedEngineOutput;
			
			//ApplyToolsToEngine(displayOutput);

			using (var renderer = displayOutput.StartRender())
			{
				//Get the framework's properties
				var prop = displayOutput.GetProperties();
				var mode = prop.ActiveMode;
				var resolution = prop.Resolution;
				var refreshRate = prop.RefreshRate;

				var prediction = displayOutput.GetPrediction();
				var predictionBitmap = prediction.GetBitmap();

				var bmpBuffer = predictionBitmap.LockBuffer(BitmapBufferAccessMode.ReadWrite);
				IMemoryBufferReference predPixelBuffer = bmpBuffer.CreateReference();
				var predSrc = BufferToImgConv(predPixelBuffer);

			}
		}

		private void compareFrames_Click(object sender, RoutedEventArgs e)
		{
			var capturedFrame = CaptureInput.CaptureFrame();
			var prediction = SelectedEngineOutput.GetPrediction();
			capturedFrame.CompareCaptureToPrediction("Basic Test", prediction);
		}

		// Converting buffer to image source
		private static System.Windows.Media.Imaging.BitmapSource BufferToImgConv(IMemoryBufferReference pixelBuffer)
		{
			System.Windows.Media.Imaging.BitmapSource imgSource;
			unsafe
			{
				byte[] bytes = new byte[pixelBuffer.Capacity];
				fixed (byte* bytesAccess = bytes)
				{
					byte* ptr;
					uint capacity;
					var ByteAccess = pixelBuffer.As<IMemoryBufferByteAccess>();
					ByteAccess.GetBuffer(out ptr, out capacity);

					// copy the raw memory out to the byte array
					Unsafe.CopyBlockUnaligned(
						ref bytesAccess[0], ref ptr[0], capacity);

				}
				var pixCap = pixelBuffer.Capacity;
				imgSource = System.Windows.Media.Imaging.BitmapSource.Create(800, 600, 96, 96, PixelFormats.Bgr32, null, bytes, (800 * PixelFormats.Bgr32.BitsPerPixel + 7) / 8);
			}

			imgSource.Freeze();
			return imgSource;
		}
	}
}
