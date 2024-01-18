using CaptureCardViewer.ViewModels;
using CommunityToolkit.Mvvm.Input;
using System.Diagnostics;
using System.Windows;

namespace CaptureCardViewer
{
	public partial class MainWindow : Window
	{
		public WorkspaceViewModel ViewModel { get; } = new WorkspaceViewModel();

		public MainWindow()
		{
			InitializeComponent();
		}

		[RelayCommand]
		async void ShowAbout()
		{
			await aboutDialog.ShowAsync();
		}

		[RelayCommand]
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
