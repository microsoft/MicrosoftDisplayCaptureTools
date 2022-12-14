using System;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using Microsoft.Win32;
using System.Threading;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using Windows.Foundation;
using Windows.Graphics.Imaging;
using WinRT;
using MicrosoftDisplayCaptureTools.Framework;
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
		Core? testFramework;
		bool userInput = false;

		public MainWindow()
		{
			InitializeComponent();
		}

		
		//Loading the plugin framework
		private async void loadFramework(object sender, RoutedEventArgs e)
		{
			testFramework = new Core();

			var dialog = new OpenFileDialog(); //file picker
			dialog.Title = "Load a Capture Plugin";
			if (dialog.ShowDialog() == true)
			{
				try
				{
					await Task.Run(() =>
					{
						testFramework.LoadConfigFile(dialog.FileName);
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

		// Apply tools to the framework's display engine
		private void ApplyToolsToEngine(MicrosoftDisplayCaptureTools.Display.IDisplayOutput displayOutput)
		{
			foreach (var toolbox in this.testFramework.GetConfigurationToolboxes())
			{ 
				foreach (var toolName in toolbox.GetSupportedTools())
				{
					var tool = toolbox.GetTool(toolName);

					if (userInput)
					{
						var suppConfig = tool.GetSupportedConfigurations();
						foreach (var config in suppConfig)
						{
							if (cbi_ref.SelectedItem != null)
							{
								ComboBoxItem cbi = (ComboBoxItem)cbi_ref.SelectedItem;
								string? sel = cbi.Content.ToString();
								if (sel == config)
								{
									tool.SetConfiguration(config);
								}

							}
							if (cbi_res.SelectedItem != null)
							{
								ComboBoxItem cbi = (ComboBoxItem)cbi_res.SelectedItem;
								string? sel = cbi.Content.ToString();
								if (sel == config)
								{
									tool.SetConfiguration(config);
								}
							}

							if (cbi_col.SelectedItem != null)
							{
								ComboBoxItem cbi = (ComboBoxItem)cbi_col.SelectedItem;
								string? sel = cbi.Content.ToString();
								if (sel == config)
								{
									tool.SetConfiguration(config);
								}
							}
						}
					}

					tool.Apply(displayOutput);
				}
			}
		}


		//Displaying frames & properties from the plugin
		private async void displayProperties(object sender, RoutedEventArgs e)
		{
			//Display & Capture of frames 
			await Task.Run(() =>
			{
				//Captured frames from the tanager board 
				var genericCapture = this.testFramework.GetCaptureCards()[0];
				var displayEngine = this.testFramework.GetDisplayEngines()[0];
				var captureInputs = genericCapture.EnumerateDisplayInputs();
				var captureInput = captureInputs[0];
				captureInput.FinalizeDisplayState();

				var displayOutput = displayEngine.InitializeOutput("DEL41846VTHZ13_1E_07E4_EC");

				ApplyToolsToEngine(displayOutput);

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
				this.Dispatcher.Invoke(
						new Action(() =>
						{
							CaptImage.Source = capSrc;
							PredImage.Source = predSrc;
							TextBlock.Text = "Refresh Rate: " + refreshRate.ToString() + "\r\n";
							TextBlock.Text += "Resolution: " + resolution.Height.ToString() + "x" + resolution.Width.ToString() + "\r\n";							
							TextBlock.Text += "Source pixel format: " + mode.SourcePixelFormat.ToString() + "\r\n";

						}
						));

				Thread.Sleep(5000);
				
			});
		}

		//dialog to filename string
		private string dialogToFilename()
		{
			var filename = "";
			var dialog = new OpenFileDialog();
			dialog.Title = "Select file";
			if (dialog.ShowDialog() == true)
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
			var genericCapture = this.testFramework.GetCaptureCards()[0];
			var captureInputs = genericCapture.EnumerateDisplayInputs();
			var displayEngine = this.testFramework.GetDisplayEngines()[0];
			var captureInput = captureInputs[0];
			var capturedFrame = captureInput.CaptureFrame();
			var displayOutput = displayEngine.InitializeOutput("DEL41846VTHZ13_1E_07E4_EC");
			var prediction = displayOutput.GetPrediction();
			capturedFrame.CompareCaptureToPrediction("Basic Test", prediction);
		}

		// Loading Display Manager file
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
						testFramework.LoadDisplayEngine(DispMan_filename);
					});
					MessageBox.Show("Display Manager file loaded");
				}
				catch (Exception) { }

			}
			else { MessageBox.Show("Display Manager trouble loading"); }
		}

		// Loading Capture Plugin file
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

		// Loading Toolbox file
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
			userInput = true;

		}
		
	}
}
