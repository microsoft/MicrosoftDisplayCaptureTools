using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using MicrosoftDisplayCaptureTools.CaptureCard;
using MicrosoftDisplayCaptureTools.ConfigurationTools;
using MicrosoftDisplayCaptureTools.Display;
using MicrosoftDisplayCaptureTools.Framework;
using System;
using System.Collections.ObjectModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks;
using System.Windows;
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

		public ToolboxViewModel Toolbox { get; }

		[ObservableProperty]
		[NotifyPropertyChangedFor(nameof(SelectedEngineOutputName))]
		[NotifyPropertyChangedFor(nameof(CanStartOutputRender))]
		IDisplayOutput? selectedEngineOutput;
		public string SelectedEngineOutputName => SelectedEngineOutput?.Target.StableMonitorId ?? "None";

		IDisposable? selectedEngineOutputRenderer;

		[ObservableProperty]
		[NotifyPropertyChangedFor(nameof(CanCompare))]
		IDisplayCapture? lastCapturedFrame;

		[ObservableProperty]
		ObservableCollection<MetadataViewModel> lastCapturedFrameMetadata = new ObservableCollection<MetadataViewModel>();

		[ObservableProperty]
		[NotifyPropertyChangedFor(nameof(CanCompare))]
		IRawFrameSet? lastPredictedFrame;

		[ObservableProperty]
		[NotifyPropertyChangedFor(nameof(IsComparisonFailed))]
		[NotifyPropertyChangedFor(nameof(IsComparisonPassed))]
		bool? lastComparisonResult;

		public bool CanCompare => LastCapturedFrame != null && LastPredictedFrame != null;
		public bool IsComparisonPassed => LastComparisonResult.HasValue && LastComparisonResult == true;
		public bool IsComparisonFailed => LastComparisonResult.HasValue && LastComparisonResult == false;
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
			[NotifyPropertyChangedFor(nameof(Name))]
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

			[RelayCommand]
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

		[ObservableProperty]
		int framesCaptured = 0;

		/// <summary>
		/// The live capture async task sets this to trigger the UI to update.
		/// </summary>
		[ObservableProperty]
		[NotifyPropertyChangedFor(nameof(CanStartLiveCapture))]
		[NotifyPropertyChangedFor(nameof(CanStopLiveCapture))]
		[NotifyPropertyChangedFor(nameof(CanSingleFrameCapture))]
		bool isRunningLiveCapture = false;

		public bool CanStartLiveCapture => CaptureInput != null && !IsRunningLiveCapture;
		[ObservableProperty]
		bool canStopLiveCapture = false;
		public bool CanSingleFrameCapture => CanStartLiveCapture;

		/// <summary>
		/// The output render task uses this to reflect the buttons
		/// </summary>
		[ObservableProperty]
		[NotifyPropertyChangedFor(nameof(CanStopOutputRender))]
		[NotifyPropertyChangedFor(nameof(CanStartOutputRender))]
		[NotifyPropertyChangedFor(nameof(IsNotRendering))]
		bool isRenderingOutput = false;

		public bool IsNotRendering => !IsRenderingOutput;

		public bool CanStartOutputRender => SelectedEngineOutput != null && !IsRenderingOutput;
		public bool CanStopOutputRender => IsRenderingOutput;


		public CaptureSessionViewModel(WorkspaceViewModel workspace, IDisplayEngine engine, ISourceToSinkMapping inputOutputMapping, ToolboxViewModel toolbox)
		{
			Workspace = workspace;
			Engine = engine;
			Toolbox = toolbox;
			CaptureInput = inputOutputMapping.Sink;

			SelectedEngineOutput = Engine.InitializeOutput(inputOutputMapping.Source);

			if (SelectedEngineOutput == null)
			{
				ModernWpf.MessageBox.Show("Could not take control of the mapped output.");
				return;
			}
		}

		// Displaying frames & properties from the plugin
		[RelayCommand]
		async void CaptureSingleFrame()
		{
			// Display & Capture of frames 
			(var capturedFrame, var capturedBitmap, var capturedMetadata) =
				await Task.Run(async () =>
			{
				// Ensure that the capture card is ready to capture
				var captureInput = CaptureInput;

				var capturedFrame = captureInput.CaptureFrame();

				var capPixelBuffer = capturedFrame.GetFrameData().Frames()[0];

				BitmapSource capturedBitmap = null;
				if (capPixelBuffer is IRawFrameRenderable capPixelBufferRenderable)
				{
					capturedBitmap = BufferToImgConv(await capPixelBufferRenderable.GetRenderableApproximationAsync());
				}

				return (capturedFrame, capturedBitmap, capturedFrame.ExtendedProperties);
			});

			CaptureSource = capturedBitmap;
			LastCapturedFrame = capturedFrame;
			LastCapturedFrameMetadata = new ObservableCollection<MetadataViewModel>(
				capturedMetadata.Select(kvp => new MetadataViewModel(kvp.Key, kvp.Value)));
			FramesCaptured += 1;
		}

		[RelayCommand]
		async void StartLiveCapture()
		{
			FramesCaptured = 0;
		}

		[RelayCommand]
		async void StopLiveCapture()
		{
			IsRunningLiveCapture = false;
		}

		[RelayCommand]
		async void StartOutputRender()
		{
			await Task.Run(() =>
			{
				// Start the render
				if (SelectedEngineOutput != null)
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
								tool.ApplyToOutput(SelectedEngineOutput);
							}
						}
					}
					var captureInput = CaptureInput;

					CaptureInput.FinalizeDisplayState();

					// Ensure that the capture card can actually handle the output
					var caps = captureInput.GetCapabilities();
					caps.ValidateAgainstDisplayOutput(SelectedEngineOutput);

					selectedEngineOutputRenderer = SelectedEngineOutput?.StartRender();
				}

				IsRenderingOutput = true;
			});
		}

		[RelayCommand]
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

		[RelayCommand]
		async void RenderPrediction()
		{
			var displayPrediction = Toolbox.Toolbox.CreatePrediction();

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

				var prediction = await displayPrediction.FinalizePredictionAsync();
				var predictionBitmap = prediction.Frames()[0];

				if (predictionBitmap is IRawFrameRenderable predictionBitmapRenderable)
				{
					return (prediction, BufferToImgConv(await predictionBitmapRenderable.GetRenderableApproximationAsync()));
				}
				return (prediction, BufferToImgConv(predictionBitmap.Data, predictionBitmap.Resolution, predictionBitmap.DataFormat));
			});

			LastPredictedFrame = predictedFrame;
			PredictionSource = predictedBitmap;
		}

		[RelayCommand]
		async void CompareCapture()
		{
			// Snap the current frames in case they change while we're comparing
			var lastCapture = this.LastCapturedFrame;
			var lastPrediction = this.LastPredictedFrame;

			if (lastCapture == null || lastPrediction == null)
				return;

			LastComparisonResult = await Task.Run(() =>
			{
				return lastCapture.CompareCaptureToPrediction("Basic Test", lastPrediction);
			});
		}

		// Converting buffer to image source
		private static BitmapSource BufferToImgConv(IBuffer pixelBuffer, Windows.Graphics.SizeInt32 resolution, DisplayWireFormat dataFormat)
		{
			int stride = resolution.Width * 3;
			// TODO: Implement other data formats

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

		// Convert SoftwareBitmap to a WPF BitmapSource
		private unsafe static BitmapSource BufferToImgConv(SoftwareBitmap bitmap)
		{
			BitmapSource imgSource;

			// The bitmap format here should always be RGBA8, which is how the approximation is documented to be rendered.
			var pixelFormat = bitmap.BitmapPixelFormat switch
			{
				BitmapPixelFormat.Rgba8 => PixelFormats.Bgr32,
				_ => throw new Exception($"Unsupported preview bitmap format {bitmap.BitmapPixelFormat}")
			};

			// Fix the bitmap format to work with WPF's BitmapSource.
			var bgra32Bitmap = SoftwareBitmap.Convert(bitmap, BitmapPixelFormat.Bgra8, BitmapAlphaMode.Premultiplied);

			using (var lockedBuffer = bgra32Bitmap.LockBuffer(BitmapBufferAccessMode.Read))
			using (var memRef = lockedBuffer.CreateReference())
			{
				var memRefAccess = memRef.As<IMemoryBufferByteAccess>();
				memRefAccess.GetBuffer(out var buffer, out var capacity);

				var description = lockedBuffer.GetPlaneDescription(0);
				
				imgSource = BitmapSource.Create(description.Width, description.Height, 96, 96, pixelFormat, null, (nint)buffer, (int)capacity, description.Stride);
			}

			imgSource.Freeze();
			return imgSource;

		}
	}
}
