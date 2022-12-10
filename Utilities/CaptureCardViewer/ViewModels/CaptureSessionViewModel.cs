using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using MicrosoftDisplayCaptureTools.CaptureCard;
using MicrosoftDisplayCaptureTools.Display;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;
using Windows.Devices.Display;
using Windows.Devices.Display.Core;
using Windows.Foundation;
using Windows.Graphics.Imaging;
using WinRT;

namespace CaptureCardViewer.ViewModels
{
	public partial class CaptureSessionViewModel : ObservableObject
	{
		public WorkspaceViewModel Workspace { get; }

		public CaptureCardViewModel CaptureCard { get; }
		public IDisplayInput CaptureInput { get; }
		public IDisplayEngine Engine { get; }

		[ObservableProperty, AlsoNotifyChangeFor(nameof(SelectedEngineOutputName))]
		IDisplayOutput? selectedEngineOutput;
		public string SelectedEngineOutputName => SelectedEngineOutput?.Target.StableMonitorId ?? "None";
		public string Name => CaptureInput.Name;

		public partial class AvailableEngineOutput
		{
			CaptureSessionViewModel parent;
			public DisplayMonitor Monitor { get; }
			public DisplayTarget Target { get; }
			public string Name => $"{Monitor?.DisplayName} ({Target.StableMonitorId})";

			public AvailableEngineOutput(CaptureSessionViewModel parent, DisplayTarget target)
			{
				this.parent = parent;
				Target = target;
				Monitor = target.TryGetMonitor();
			}

			[ICommand]
			void Connect()
			{
				parent.SelectedEngineOutput = parent.Engine.InitializeOutput(Target);
			}
		}

		public ImageSource? PredictionSource { get; }
		public ImageSource? CaptureSource { get; }

		ObservableCollection<AvailableEngineOutput> availableEngineOutputs = new();
		public ReadOnlyObservableCollection<AvailableEngineOutput> AvailableEngineOutputs { get; }
		
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
			//var activeTools = new List<Tool>();
			//Workspace.Toolboxes.
			
			//Display & Capture of frames 
			await Task.Run(() =>
			{
				//Captured frames from the tanager board 
				var captureInput = CaptureInput;
				captureInput.FinalizeDisplayState();

				var displayOutput = Engine.InitializeOutput("DEL41846VTHZ13_1E_07E4_EC");

				//ApplyToolsToEngine(displayOutput);

				var renderer = displayOutput.StartRender();
				Thread.Sleep(5000);

				var capturedFrame = captureInput.CaptureFrame();
				var capPixelBuffer = capturedFrame.GetRawPixelData();
				var capSrc = BufferToImgConv(capPixelBuffer);

				//Get the framework's properties
				var prop = displayOutput.GetProperties();
				var mode = prop.ActiveMode;
				var resolution = prop.Resolution;
				var refreshRate = prop.RefreshRate;

				renderer.Dispose();
				var prediction = displayOutput.GetPrediction();
				var bitmap = prediction.GetBitmap();

				var bmpBuffer = bitmap.LockBuffer(BitmapBufferAccessMode.ReadWrite);
				IMemoryBufferReference predPixelBuffer = bmpBuffer.CreateReference();
				var predSrc = BufferToImgConv(predPixelBuffer);

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
