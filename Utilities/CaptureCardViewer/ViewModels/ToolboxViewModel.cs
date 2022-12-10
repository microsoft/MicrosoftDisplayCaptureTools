using CommunityToolkit.Mvvm.ComponentModel;
using MicrosoftDisplayCaptureTools.ConfigurationTools;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
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

			foreach (var tool in SupportedTools)
				ActiveTools.Add(new ToolViewModel(Toolbox.GetTool(tool)));
		}

		public string[] SupportedTools { get; }

		public ObservableCollection<ToolViewModel> ActiveTools { get; } = new();
	}
}
