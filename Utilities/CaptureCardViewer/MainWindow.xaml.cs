using CaptureCardViewer.ViewModels;
using CommunityToolkit.Mvvm.Input;
using Microsoft.Win32;
using MicrosoftDisplayCaptureTools.Framework;
using System;
using System.Diagnostics;
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

		[ICommand]
		async void ShowAbout()
		{
			await aboutDialog.ShowAsync();
		}

		[ICommand]
		void ShowDocumentation()
		{
			try
			{
				Process.Start(new ProcessStartInfo("https://learn.microsoft.com/windows-hardware/") { UseShellExecute = true });
			}
			catch { }
		}
	}
}
