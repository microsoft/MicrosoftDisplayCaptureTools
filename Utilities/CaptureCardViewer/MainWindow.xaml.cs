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
using System.Security.Cryptography;
//using Microsoft.UI.Xaml.Media.Imaging;
//using Microsoft.UI.Xaml.Media;

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
		bool userInput = false;

		//private IBuffer predBuffer;

		public MainWindow()
        {
            InitializeComponent();
        }

		//Loading the plugin framework
		private async void loadFramework (object sender, RoutedEventArgs e)
		{
			var dialog = new OpenFileDialog(); //file picker
			dialog.Title = "Load a Capture Plugin";
			if (dialog.ShowDialog() == true)
			{
				try
				{
					//var configFile = dialog.FileName;
					var configFile = System.IO.Path.GetFileName(dialog.FileName);


					await Task.Run(() =>
					{
						testFramework.LoadConfigFile(configFile);
					});
					MessageBox.Show("Capture card plugin done loading");
				}
				catch (Exception) { }	

			}
			else { MessageBox.Show("Dialog Box trouble loading"); }
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

		// Apply Render and capture reusable method
		private void ApplyRenderAndCapture(IDisplayEngine displayEngine)
		{
			displayEngine.InitializeForStableMonitorId("DEL41846VTHZ13_1E_07E4_EC");
			var tools = this.testFramework.GetLoadedTools();
			foreach (var tool in tools)
			{
				if (userInput)
				{
					var suppConfig = tool.GetSupportedConfigurations();
					foreach (var config in suppConfig)
						{
						if (cbi_res.SelectionBoxItem.ToString() == config)
						{
							tool.SetConfiguration(config);
						}
							
						else if(cbi_ref.SelectionBoxItem.ToString() == config)
						{
							tool.SetConfiguration(config);
						}
					}
				}
				tool.Apply(displayEngine);
			}
			var renderer = displayEngine.StartRender();
			Thread.Sleep(5000);
			renderer.Dispose();
		}


		//Displaying frames & properties from the plugin
		private async void displayProperties(object sender, RoutedEventArgs e)
		{
			//Display & Capture of frames 
			await Task.Run(() =>
			{
				//Captured frames from the tanager board 
				var genericCapture = this.testFramework.GetCaptureCard();
				var captureInputs = genericCapture.EnumerateDisplayInputs();
				var displayEngine = this.testFramework.GetDisplayEngine();
				
				var captureInput = captureInputs[0];
				captureInput.FinalizeDisplayState();
				ApplyRenderAndCapture(displayEngine);


				var capturedFrame = captureInput.CaptureFrame();
				var capPixelBuffer = capturedFrame.GetRawPixelData();
				var capSrc = BufferToImgConv(capPixelBuffer);

				//Loading & displaying tools
				//Reset the display manager to the correct one
				//;

				
				
				
				//Get the framework's properties
				var prop = displayEngine.GetProperties();
				var mode = prop.ActiveMode;
				var resolution = prop.Resolution;
				var refreshRate = prop.RefreshRate;
				var planeProp = prop.GetPlaneProperties();
				

				//Generate  & display frames to compare the Tanager's frames against
				//renderer.Dispose();
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
							TextBlock.Text = "Mode:  " + mode.ToString() + "\r\n";
							TextBlock.Text += "Resolution: " + resolution.ToString() +"\r\n";
							TextBlock.Text += "Refresh Rate: " + refreshRate.ToString() + "\r\n";
							TextBlock.Text += "Plane Properties: " + planeProp.ToString() + "\r\n";
						}
						));

				Thread.Sleep(1000);
			});

			
		}

		//dialog to filename string
		private string dialogToFilename()
		{
			var filename = "";
			var dialog = new OpenFileDialog();
			dialog.Title = "Select file";
			if(dialog.ShowDialog()==true)
			{
				try
				{
					filename = dialog.FileName.ToString();
				}
				catch (Exception) { }
			}
			return filename;

		}

		private void compareFrames_Click(object sender, RoutedEventArgs e)
		{
			var genericCapture = this.testFramework.GetCaptureCard();
			var captureInputs = genericCapture.EnumerateDisplayInputs();
			var displayEngine = this.testFramework.GetDisplayEngine();
			var captureInput = captureInputs[0];
			var capturedFrame = captureInput.CaptureFrame();
			var prediction = displayEngine.GetPrediction();
			capturedFrame.CompareCaptureToPrediction("Basic Test",prediction);
		}

		//Loading Display Manager file
		private async void DispMan_Click(object sender, RoutedEventArgs e)
		{
			var dialog = new OpenFileDialog(); //file picker
			dialog.Title = "Load Display Manager file";
			if (dialog.ShowDialog() == true)
			{
				try
				{
					var DispMan_filename = dialog.FileName.ToString();
					await Task.Run(() =>
					{
						testFramework.LoadDisplayManager(DispMan_filename);
					});
					MessageBox.Show("Display Manager file loaded");
				}
				catch (Exception) { }

			}
			else { MessageBox.Show("Display Manager trouble loading"); }
		}

		//Loading Capture Plugin file
		private async void CapPlgn_Click(object sender, RoutedEventArgs e)
		{
			var dialog = new OpenFileDialog(); 
			dialog.Title = "Load Capture Plugin file";
			if (dialog.ShowDialog() == true)
			{
				try
				{
					var plugin_filename = dialog.FileName.ToString();
					await Task.Run(() =>
					{
						testFramework.LoadCapturePlugin(plugin_filename);
					});
					MessageBox.Show("Plugin file loaded");
				}
				catch (Exception) { }

			}
			else { MessageBox.Show("Capture Plugin trouble loading"); }
		}

		//Loading Toolbox file
		private async void Tlbx_Click(object sender, RoutedEventArgs e)
		{
			var dialog = new OpenFileDialog(); 
			dialog.Title = "Load Toolbox file";
			if (dialog.ShowDialog() == true)
			{
				try
				{
					var tlbx_filename = dialog.FileName.ToString();
					await Task.Run(() =>
					{
						testFramework.LoadToolbox(tlbx_filename);
					});
					MessageBox.Show("Toolbox file loaded");
				}
				catch (Exception) { }
			}
			else { MessageBox.Show("Toolbox trouble loading"); }
		}

		private void configurations(object sender, SelectionChangedEventArgs e)
		{
			//ComboBoxItem cbi_res = ((ComboBoxItem)cbi_res).SelectedItem;
			if (cbi_ref!=null)
			{
				ApplyRenderAndCapture(this.testFramework.GetDisplayEngine());
				userInput = true;
			}
			if (cbi_res!= null)
			{
				ApplyRenderAndCapture(this.testFramework.GetDisplayEngine());
				userInput = true;
			}
		}
	}
}
