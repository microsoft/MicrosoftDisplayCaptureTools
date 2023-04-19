using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using MicrosoftDisplayCaptureTools.CaptureCard;
using MicrosoftDisplayCaptureTools.ConfigurationTools;
using MicrosoftDisplayCaptureTools.Display;
using System;
using System.Collections.ObjectModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using Windows.Devices.Display;
using Windows.Devices.Display.Core;
using Windows.Devices.Enumeration;
using Windows.Foundation;
using Windows.Graphics.Imaging;
using Windows.Storage.Streams;
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
		[AlsoNotifyChangeFor(nameof(CanStartOutputRender))]
		IDisplayOutput? selectedEngineOutput;
		public string SelectedEngineOutputName => SelectedEngineOutput?.Target.StableMonitorId ?? "None";

		IDisposable? selectedEngineOutputRenderer;

		[ObservableProperty]
		[AlsoNotifyChangeFor(nameof(CanCompare))]
		IDisplayCapture? lastCapturedFrame;

		[ObservableProperty]
		ObservableCollection<MetadataViewModel> lastCapturedFrameMetadata = new ObservableCollection<MetadataViewModel>();

		[ObservableProperty]
		[AlsoNotifyChangeFor(nameof(CanCompare))]
		IDisplayPredictionData? lastPredictedFrame;

		[ObservableProperty]
		[AlsoNotifyChangeFor(nameof(IsComparisonFailed))]
		[AlsoNotifyChangeFor(nameof(IsComparisonPassed))]
		bool? lastComparisonResult;

		public bool CanCompare => lastCapturedFrame != null && lastPredictedFrame != null;
		public bool IsComparisonPassed => lastComparisonResult.HasValue && lastComparisonResult == true;
		public bool IsComparisonFailed => lastComparisonResult.HasValue && lastComparisonResult == false;
		public bool IsComparisonDetailsAvailable => false;

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
				try
				{
					parent.SelectedEngineOutput = parent.Engine.InitializeOutput(Target);
				}
				catch (Exception ex)
				{
					ModernWpf.MessageBox.Show("Failed to connect to GPU output: " + ex.Message);
				}
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

		/// <summary>
		/// The output render task uses this to reflect the buttons
		/// </summary>
		[ObservableProperty]
		[AlsoNotifyChangeFor(nameof(CanStopOutputRender))]
		[AlsoNotifyChangeFor(nameof(CanStartOutputRender))]
		[AlsoNotifyChangeFor(nameof(IsNotRendering))]
		bool isRenderingOutput = false;

		public bool IsNotRendering => !isRenderingOutput;

		public bool CanStartOutputRender => selectedEngineOutput != null && !isRenderingOutput;
		public bool CanStopOutputRender => isRenderingOutput;


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
			var displayOutput = SelectedEngineOutput;

			// Display & Capture of frames 
			(var capturedFrame, var capturedBitmap, var capturedMetadata) =
				await Task.Run(() =>
			{
				// Captured frames from the tanager board
				var captureInput = CaptureInput;
				captureInput.FinalizeDisplayState();

				var capturedFrame = captureInput.CaptureFrame();
				var capPixelBuffer = capturedFrame.GetFrameData();
				var capturedBitmap = BufferToImgConv(capPixelBuffer.Data, capPixelBuffer.Resolution, (int)capPixelBuffer.FormatDescription.Stride);

				return (capturedFrame, capturedBitmap, capturedFrame.ExtendedProperties);
			});

			CaptureSource = capturedBitmap;
			LastCapturedFrame = capturedFrame;
			LastCapturedFrameMetadata = new ObservableCollection<MetadataViewModel>(
				capturedMetadata.Select(kvp => new MetadataViewModel(kvp.Key, kvp.Value)));
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

		[ICommand]
		async void StartOutputRender()
		{
			await Task.Run(() =>
			{
				// Start the render
				if (selectedEngineOutput != null)
				{
					var activeTools =
						this.Workspace.Toolboxes
						.SelectMany((toolbox) => toolbox.ActiveTools
							.Select((tool) => tool.Tool))
						.ToList();

					// Apply all tools to the display, in categorical order
					ConfigurationToolCategory[] toolOrder = {
						ConfigurationToolCategory.DisplaySetup,
						ConfigurationToolCategory.RenderSetup,
						ConfigurationToolCategory.Render };

					foreach (var category in toolOrder)
					{
						foreach (var tool in activeTools)
						{
							if (tool.Category == category)
							{
								tool.ApplyToOutput(selectedEngineOutput);
							}
						}
					}

					selectedEngineOutputRenderer = selectedEngineOutput.StartRender();
				}

				IsRenderingOutput = true;
			});
		}

		[ICommand]
		async void StopOutputRender()
		{
			await Task.Run(() =>
			{
				// Stop the render
				if (selectedEngineOutputRenderer != null)
				{
					selectedEngineOutputRenderer.Dispose();
					selectedEngineOutputRenderer = null;
				}

				IsRenderingOutput = false;
			});
		}

		[ICommand]
		async void RenderPrediction()
		{
			var displayOutput = SelectedEngineOutput;

			if (displayOutput == null)
				return;

			var displayPrediction = Engine.CreateDisplayPrediction();

			// TODO: Encapsulate the active tools into a sub-property of this CaptureSessionViewModel
			var activeTools =
				this.Workspace.Toolboxes
				.SelectMany((toolbox) => toolbox.ActiveTools
					.Select((tool) => tool.Tool))
				.ToList();

			(var predictedFrame, var predictedBitmap) = await Task.Run(async () =>
			{
				// Apply all tools to the display
				foreach (var tool in activeTools)
				{
					tool.ApplyToPrediction(displayPrediction);
				}

				var prediction = await displayPrediction.GeneratePredictionDataAsync();
				var predictionBitmap = prediction.FrameData;

				return (prediction, BufferToImgConv(predictionBitmap.Data, predictionBitmap.Resolution, (int)predictionBitmap.FormatDescription.Stride));
			});

			LastPredictedFrame = predictedFrame;
			PredictionSource = predictedBitmap;
		}

		[ICommand]
		async void CompareCapture()
		{
			// Snap the current frames in case they change while we're comparing
			var lastCapture = this.lastCapturedFrame;
			var lastPrediction = this.lastPredictedFrame;

			if (lastCapture == null || lastPrediction == null)
				return;

			LastComparisonResult = await Task.Run(() =>
			{
				return lastCapture.CompareCaptureToPrediction("Basic Test", lastPrediction);
			});
		}

		// Converting buffer to image source
		private static BitmapSource BufferToImgConv(IBuffer pixelBuffer, Windows.Graphics.SizeInt32 resolution, int stride)
		{
			BitmapSource imgSource;
			unsafe
			{
				byte[] bytes = new byte[pixelBuffer.Capacity];
				pixelBuffer.CopyTo(bytes);

				var pixCap = pixelBuffer.Capacity;
				imgSource = BitmapSource.Create(resolution.Width, resolution.Height, 96, 96, PixelFormats.Bgr32, null, bytes, stride);
			}

			imgSource.Freeze();
			return imgSource;
		}
	}
}
