using CommunityToolkit.Mvvm.ComponentModel;
using MicrosoftDisplayCaptureTools.Display;

namespace CaptureCardViewer.ViewModels
{
	public partial class DisplayEngineViewModel : ObservableObject
	{
		public IDisplayEngine Engine { get; }
		public string Name => Engine.Name;
		public string Version => Engine.Version.ToString();

		public DisplayEngineViewModel(IDisplayEngine engine)
		{
			Engine = engine;
		}
	}
}
