using CommunityToolkit.Mvvm.ComponentModel;
using MicrosoftDisplayCaptureTools.ConfigurationTools;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CaptureCardViewer.ViewModels
{
	public partial class ToolboxViewModel : ObservableObject
	{
		public IConfigurationToolbox Toolbox { get; }
		public string Name => Toolbox.Name;
		public string Version => Toolbox.Version;
		
		public ToolboxViewModel(IConfigurationToolbox toolbox)
		{
			Toolbox = toolbox;
			SupportedTools = toolbox.GetSupportedTools();
		}

		public string[] SupportedTools { get; }
	}
}
