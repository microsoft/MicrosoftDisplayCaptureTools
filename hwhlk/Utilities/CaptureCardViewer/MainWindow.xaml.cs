using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using Microsoft.Win32;
using MicrosoftDisplayCaptureTools.Framework;
using Windows.ApplicationModel.VoiceCommands;
using Windows.Foundation;
using MicrosoftDisplayCaptureTools;
using Windows.Devices.Display;
using Windows.Devices.Display.Core;
using Windows.Graphics.Imaging;
using Windows.Data.Json;
using Windows.Media.Capture;
using System.Threading;
using System.Drawing;
using System.IO;
using WinRT;
using ABI.Windows.Foundation;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Runtime.CompilerServices;
using Windows.Storage.Streams;
using MicrosoftDisplayCaptureTools.Display;
using Windows.UI.Core;
using Windows.Graphics.DirectX;
using System.Net.Http.Headers;
using MicrosoftDisplayCaptureTools.CaptureCard;
using System.Diagnostics;
using Microsoft.UI.Xaml.Media.Imaging;
using Microsoft.UI.Xaml.Media;

namespace CaptureCardViewer
{
	//
	// Add the interface so that we can access the raw pixel data.
	//
	[ComImport]
	[Guid("5B0D3235-4DBA-4D44-865E-8F1D0E4FD04D")]
	[InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]


	unsafe interface IMemoryBufferByteAccess
	{
		void GetBuffer(out byte* buffer, out uint capacity);
	}

	public partial class MainWindow : Window
    {
		public string? setTool;
		public string? currentTool;
		Core testFramework = new Core();

		//private IBuffer predBuffer;

		public MainWindow()
        {
            InitializeComponent();
        }

		//Loading the plugin framework
		private async void loadFramework (object sender, RoutedEventArgs e)
		{
			/*var dialog = new OpenFileDialog(); //file picker
			dialog.Filter = "Plugin DLLs|*.dll";
			dialog.Title = "Load a Capture Plugin";
			string configPath = "TestConfig.json";*/
			String configPath= "TestConfig.json";
			await Task.Run(() =>
			{
				testFramework.LoadConfigFile(configPath);
			});
			MessageBox.Show("Capture card plugin done loading");

		}

		//Converting buffer to image source
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

		//Displaying frames & properties from the plugin
		private async void displayProperties(object sender, RoutedEventArgs e)
		{
			
			//Display & Capture of frames 
			await Task.Run(() =>
			{
				//Captured frames from the tanager board 
				var genericCapture = testFramework.GetCaptureCard();
				var captureInputs = genericCapture.EnumerateDisplayInputs();
				var displayEngine = testFramework.GetDisplayEngine();
				var captureInput = captureInputs[0];
				captureInput.FinalizeDisplayState();
				var capturedFrame = captureInput.CaptureFrame();
				var capPixelBuffer = capturedFrame.GetRawPixelData();
				var capSrc = BufferToImgConv(capPixelBuffer);

				//Loading & displaying tools
				//Reset the display manager to the correct one
				displayEngine.InitializeForStableMonitorId("DEL41846VTHZ13_1E_07E4_EC");

				// Get the list of tools, iterate through it and call 'apply' without changing the default setting
				var tools = testFramework.GetLoadedTools();
				foreach (var tool in tools)
					tool.Apply(displayEngine);

				var renderer = displayEngine.StartRender();
				Thread.Sleep(5000);

				//Get the framowork's properties
				var prop = displayEngine.GetProperties();
				var mode = prop.ActiveMode;
				var resolution = prop.Resolution;
				var refreshRate = prop.RefreshRate;

				//Generate  & display frames to compare the Tanager's frames against
				renderer.Dispose();
				var prediction = displayEngine.GetPrediction();
				var bitmap = prediction.GetBitmap();
				var bmpBuffer = bitmap.LockBuffer(BitmapBufferAccessMode.ReadWrite);
				IMemoryBufferReference predPixelBuffer = bmpBuffer.CreateReference();
				var predSrc = BufferToImgConv(predPixelBuffer);

				// You can't update UI elements from a background thread (here in the Await Task.Run). So we update the UI by queuing up an operation on the UI thread
				this.Dispatcher.Invoke(
						new Action(() =>
						{
							CaptImage.Source = capSrc;
							PredImage.Source = predSrc;
						}
						));

				Thread.Sleep(1000);

			});
		
		}

		//sets the tool from the selected item in the ComboBox
		private void availableTools_SelectionChanged(object sender, SelectionChangedEventArgs e)
		{	
			ComboBoxItem cbi = (ComboBoxItem)availableTools.SelectedItem;	
			string? selected = cbi.Content.ToString();
			switch (selected)
			{
				case "Event Log":
					setTool = "EventLog";
					break;

				case "Supported Features":
					setTool = "SuppFeat";
					break;

				case "":
					setTool = "LoadFromDisk";
					break;

			}
		}

    }}
