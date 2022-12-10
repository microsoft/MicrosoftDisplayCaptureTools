using CaptureCardViewer.ViewModels;
using CommunityToolkit.Mvvm.Input;
using Microsoft.Win32;
using MicrosoftDisplayCaptureTools.Framework;
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using Windows.Foundation;
using Windows.Graphics.Imaging;
using WinRT;

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
		bool userInput = false;

		public WorkspaceViewModel ViewModel { get; } = new WorkspaceViewModel();

		public MainWindow()
		{
			InitializeComponent();
		}

		private void configurations(object sender, SelectionChangedEventArgs e)
		{
			userInput = true;

		}

		[ICommand]
		async void About()
		{
			await aboutDialog.ShowAsync();
		}
	}
}
