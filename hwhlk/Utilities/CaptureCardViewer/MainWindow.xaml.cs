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
using Windows.UI.Xaml.Media.Imaging;

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

        private void LoadPlugin_Click(object sender, RoutedEventArgs e)
        {
            var dialog = new OpenFileDialog();
            dialog.Filter = "Plugin DLLs|*.dll";
            dialog.Title = "Load a Capture Plugin";
			 

            if (dialog.ShowDialog() ?? false)
            {
                try
                {
					//Framework loading the components
                    testFramework = new Core();
					var cwd=System.IO.Directory.GetCurrentDirectory();
					string fullPath = (cwd.ToString() + "BasicConfig.json").ToString();
					testFramework.LoadConfigFile(fullPath);
					testFramework.LoadPlugin(dialog.FileName, "GenericCaptureCardPlugin.Plugin");
					testFramework.LoadToolbox(dialog.FileName, "DisplayConfiguration.Toolbox");
					testFramework.LoadDisplayManager(dialog.FileName, "DisplayControl.DisplayEngine");
					
				}
                catch (Exception)
                {

                }
            }
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

		//sets the current tool for the add button
		private void addButtonClick(object sender, RoutedEventArgs e)
		{
			currentTool = setTool;
			 
		} 

		//getting predicted frames to frame comparison
		private async void displayPredictedFrames(IDisplayEngine displayEngine,IDisplayInput captureInput)
		{
			await Task.Run(async() =>
			{
				//Reset the display manager to the correct one
				displayEngine.InitializeForStableMonitorId("DEL41846VTHZ13_1E_07E4_EC");

				// Get the list of tools, iterate through it and call 'apply' without changing the default setting
				var tools = testFramework.GetLoadedTools();

				foreach (var tool in tools)
					tool.Apply(displayEngine);

				var renderer = displayEngine.StartRender();

				Thread.Sleep(5000);

				var capturedFrame = captureInput.CaptureFrame();
				renderer.Dispose();
				var prediction = displayEngine.GetPrediction();
				var bitmap = prediction.GetBitmap();
				SoftwareBitmapSource pSource = new SoftwareBitmapSource();
				await pSource.SetBitmapAsync(bitmap);
				
				this.Dispatcher.Invoke(
				new Action(() =>
				{
					
					PredictedImage.Source = pSource;
				}
				));
				Thread.Sleep(1000);
			});
		}

		//run the frame debugger depending on the set tool
		private async void runAndCompareButtonClick(object sender, RoutedEventArgs e)
		{
			// Eventually the config file should be sourced from a file picker like you have below here in comments.
			/*var dialog = new OpenFileDialog(); //file picker
			dialog.Filter = "Plugin DLLs|*.dll";
			dialog.Title = "Load a Capture Plugin";*/
			string fP = "TestConfig.json";
			
			switch (currentTool)
			{
				case (null or "LoadFromDisk"):
					await Task.Run(() =>
					{
						//testFramework.LoadConfigFile(fP);
					});

					MessageBox.Show("Capture card plugin done loading");

					break;

				case "EventLog":
					//TODO: Query event logs from the capture card plugin
					break;

				case "SuppFeat":
					testFramework.GetLoadedTools();
					break;
			}

			System.Windows.Media.Imaging.BitmapSource bSource;
			
			//capturing frames 
			await Task.Run(() =>
			{
				testFramework.LoadConfigFile(fP);
				var genericCapture = testFramework.GetCaptureCard();
				var captureInputs = genericCapture.EnumerateDisplayInputs();
				var displayEngine = testFramework.GetDisplayEngine();
				var captureInput = captureInputs[0];
				captureInput.FinalizeDisplayState();

				displayPredictedFrames(displayEngine, captureInput);

				/*//Reset the display manager to the correct one
				displayEngine.InitializeForStableMonitorId("DEL41846VTHZ13_1E_07E4_EC");

				// Get the list of tools, iterate through it and call 'apply' without changing the default setting
				var tools = testFramework.GetLoadedTools();
				foreach (var tool in tools)
					tool.Apply(displayEngine);

				var renderer = displayEngine.StartRender();

				Thread.Sleep(5000);

				var capturedFrame = captureInput.CaptureFrame();

				renderer.Dispose();

				var prediction = displayEngine.GetPrediction();*/
				var capturedFrame = captureInput.CaptureFrame();
				var pixelBuffer = capturedFrame.GetRawPixelData();
				
				// This needs to be inside of an unsafe block because we are manipulating bytes directly
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

					bSource = System.Windows.Media.Imaging.BitmapSource.Create(800, 600, 96, 96, PixelFormats.Bgr32, null, bytes, (800 * PixelFormats.Bgr32.BitsPerPixel + 7) / 8);
					
				}

				//
				bSource.Freeze();
			// You can't update UI elements from a background thread (here in the Await Task.Run). So we update the UI by queuing up an operation on the UI thread
			this.Dispatcher.Invoke(
					new Action(() => 
					{
						ActualImage.Source = bSource;	
					}
					));
				
				Thread.Sleep(1000);
					
				
			});




		}
    }}
