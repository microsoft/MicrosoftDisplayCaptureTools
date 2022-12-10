using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using Microsoft.Win32;
using MicrosoftDisplayCaptureTools.CaptureCard;
using MicrosoftDisplayCaptureTools.ConfigurationTools;
using MicrosoftDisplayCaptureTools.Display;
using MicrosoftDisplayCaptureTools.Framework;
using System;
using System.Collections.ObjectModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;
using System.Windows.Threading;
using Windows.Foundation;
using Windows.Graphics.Imaging;
using WinRT;

namespace CaptureCardViewer.ViewModels
{
	public partial class WorkspaceViewModel : ObservableObject
	{
		Core testFramework;
		Dispatcher dispatcher;

		public ObservableCollection<ToolboxViewModel> Toolboxes { get; } = new();
		public ObservableCollection<CaptureCardViewModel> CaptureCards { get; } = new();
		public ObservableCollection<DisplayEngineViewModel> DisplayEngines { get; } = new();

		public RichTextLogger Logger { get; } = new RichTextLogger();

		public ObservableCollection<object> Documents { get; } = new();

		public WorkspaceViewModel()
		{
			testFramework = new Core(Logger);
			dispatcher = Dispatcher.CurrentDispatcher;
			var command = this.LoadFromConfigFileCommand;

			Documents.Add(this);
			Documents.Add(new CaptureSessionViewModel(this, null));
		}

		[ICommand]
		async void LoadFromConfigFile()
		{
			var dialog = new OpenFileDialog(); //file picker
			dialog.Title = "Load a Capture Plugin";
			if (dialog.ShowDialog() == true)
			{
				try
				{
					await Task.Run(() =>
					{
						// Create a new framework from scratch
						var newInstance = new Core(Logger);
						newInstance.LoadConfigFile(dialog.FileName);

						// Now that the configuration was successfully loaded, update the view models
						testFramework = newInstance;
						CaptureCards.Clear();
						Toolboxes.Clear();

						testFramework.GetCaptureCards()
							.Select((card) => new CaptureCardViewModel(card))
							.ToList()
							.ForEach((card) => CaptureCards.Add(card));

						testFramework.GetConfigurationToolboxes()
							.Select((toolbox) => new ToolboxViewModel(toolbox))
							.ToList()
							.ForEach((toolbox) => Toolboxes.Add(toolbox));

						testFramework.GetDisplayEngines()
							.Select((engine) => new DisplayEngineViewModel(engine))
							.ToList()
							.ForEach((engine) => DisplayEngines.Add(engine));
					});
				}
				catch (Exception ex)
				{
					MessageBox.Show("Failed to load configuration.\r\n" + ex.Message);
				}
			}
		}

		// Loading Display Manager file
		[ICommand]
		async void LoadDisplayEngineFromFile()
		{
			var dialog = new OpenFileDialog(); //file picker
			dialog.Title = "Load Display Engine file";
			if (dialog.ShowDialog() == true)
			{
				try
				{
					var filename = dialog.FileName.ToString();
					IDisplayEngine engine = null;
					await Task.Run(() =>
					{
						engine = testFramework.LoadDisplayEngine(filename);
					});
					DisplayEngines.Add(new DisplayEngineViewModel(engine!));
				}
				catch (Exception ex)
				{
					MessageBox.Show("Failed to load the display engine.\r\n" + ex.Message);
				}
			}
		}

		// Loading Capture Plugin file
		[ICommand]
		async void LoadCaptureCardFromFile()
		{
			var dialog = new OpenFileDialog();
			dialog.Title = "Load Capture Plugin file";
			if (dialog.ShowDialog() == true)
			{
				try
				{
					var filename = dialog.FileName.ToString();
					CaptureCardViewModel controller = null;
					
					await Task.Run(() =>
					{
						controller = new CaptureCardViewModel(testFramework.LoadCapturePlugin(filename));
					});

					CaptureCards.Add(controller!);
				}
				catch (Exception ex)
				{
					MessageBox.Show("Failed to load the capture plugin.\r\n" + ex.Message);
				}

			}
		}

		// Loading Toolbox file
		[ICommand]
		async void LoadToolboxFromFile()
		{
			var dialog = new OpenFileDialog();
			dialog.Title = "Load Toolbox file";
			if (dialog.ShowDialog() == true)
			{
				try
				{
					var filename = dialog.FileName.ToString();
					IConfigurationToolbox toolbox = null;
					await Task.Run(() =>
					{
						toolbox = testFramework.LoadToolbox(filename);
					});
					Toolboxes.Add(new ToolboxViewModel(toolbox!));
				}
				catch (Exception ex)
				{
					MessageBox.Show("Failed to load the toolbox.\r\n" + ex.Message);
				}
			}
		}

		// Apply tools to the framework's display engine
		private void ApplyToolsToEngine(MicrosoftDisplayCaptureTools.Display.IDisplayOutput displayOutput)
		{
			foreach (var toolbox in this.testFramework.GetConfigurationToolboxes())
			{
				foreach (var toolName in toolbox.GetSupportedTools())
				{
					var tool = toolbox.GetTool(toolName);

					/*if (userInput)
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
					}*/

					tool.Apply(displayOutput);
				}
			}
		}

		// Displaying frames & properties from the plugin
		[ICommand]
		async void CaptureSingleFrame()
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
			var genericCapture = this.testFramework.GetCaptureCards()[0];
			var captureInputs = genericCapture.EnumerateDisplayInputs();
			var displayEngine = this.testFramework.GetDisplayEngines()[0];
			var captureInput = captureInputs[0];
			var capturedFrame = captureInput.CaptureFrame();
			var displayOutput = displayEngine.InitializeOutput("DEL41846VTHZ13_1E_07E4_EC");
			var prediction = displayOutput.GetPrediction();
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
