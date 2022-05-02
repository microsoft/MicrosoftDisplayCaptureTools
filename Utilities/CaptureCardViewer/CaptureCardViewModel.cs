using MicrosoftDisplayCaptureTools.Framework;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace CaptureCardViewer
{
	internal class CaptureCardViewModel : INotifyPropertyChanged
	{
		Framework? rootFramework;
		bool isCapturingFrames = false;
		WriteableBitmap? captureImageSource;

		public CaptureCardViewModel(string dllPluginPath)
		{
			rootFramework = new Framework(dllPluginPath);
		}

		public event PropertyChangedEventHandler? PropertyChanged;

		protected void OnPropertyChanged([CallerMemberName] string propertyName = "")
		{
			if (PropertyChanged != null)
				PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
		}

		private int m_resolutionWidth;
		private int m_resolutionHeight;
		private double m_refreshRate;

		public int ResolutionWidth
		{
			get => m_resolutionWidth;
			private set { m_resolutionWidth = value; OnPropertyChanged(); }
		}
		public int ResolutionHeight
		{
			get => m_resolutionHeight;
			private set { m_resolutionHeight = value; OnPropertyChanged(); }
		}

		public double RefreshRate
		{
			get => m_refreshRate;
			private set { m_refreshRate = value; OnPropertyChanged(); }
		}

		public ImageSource FrameSource
		{
			get
			{
				//ImageSource captureImageSource = null;
				return captureImageSource;
			}
		}

		public void StartCapture()
		{
			// TODO: Insert logic to start a background thread that writes bitmap data to captureImageSource
			MessageBox.Show("Started the capture :-)");
			/*//Image captureImg= Get from plugin; 
			BitmapImage captureImageSource=new BitmapImage();
			capturedBmp.BeginInit();
			capturedBmp.UriSource = new Uri(@""); //add directory where the file will be saved 
												  //figure desired height & width of rendered image 
			capturedBmp.DecodePixelWidth = 150;
			capturedBmp.EndInit();
			capturedImg.Source = capturedBmp;
			captureImageSource=capturedBmp;
			*/

		}

		public void StopCapture()
		{
			// TODO: Insert logic to stop the background thread
			MessageBox.Show("Ending the capture :-)");
		}

		public bool IsCapturingFrames
		{
			get
			{
				return isCapturingFrames;
			}
			set
			{
				if (value != isCapturingFrames)
				{
					isCapturingFrames = value;
					if (isCapturingFrames)
					{
						StartCapture();
					}
					else
					{
						StopCapture();
					}
				}
			}
		}
	}

	internal class Framework
	{
		public Framework(string dllPluginPath)
		{
			DllPluginPath = dllPluginPath;
		}

		public string DllPluginPath { get; }
	}
}

