using CommunityToolkit.Mvvm.ComponentModel;
using MicrosoftDisplayCaptureTools.Display;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CaptureCardViewer.ViewModels
{
	public partial class DisplayEngineViewModel : ObservableObject
	{
		public IDisplayEngine Engine { get; }
		public string Name => Engine.Name;
		public string Version => Engine.Version;

		public DisplayEngineViewModel(IDisplayEngine engine)
		{
			Engine = engine;
		}
	}
}
