using CommunityToolkit.Mvvm.ComponentModel;
using MicrosoftDisplayCaptureTools.ConfigurationTools;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CaptureCardViewer.ViewModels
{
	/// <summary>
	/// Represents an instance of a tool in the UI.
	/// </summary>
	public partial class ToolViewModel : ObservableObject
	{
		public IConfigurationTool Tool { get; }
		public string Name => Tool.Name;

		public ToolViewModel(IConfigurationTool tool)
		{
			Tool = tool;
			AvailableValues = tool.GetSupportedConfigurations();
		}

		public string Value
		{
			get
			{
				return Tool.GetConfiguration();
			}
			set
			{
				OnPropertyChanging();
				Tool.SetConfiguration(value);
				OnPropertyChanged();
			}
		}

		public string[] AvailableValues { get; }
	}
}
