using CommunityToolkit.Mvvm.ComponentModel;
using MicrosoftDisplayCaptureTools.CaptureCard;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;

namespace CaptureCardViewer.ViewModels
{
	public partial class CaptureSessionViewModel : ObservableObject
	{
		public WorkspaceViewModel Workspace { get; }

		public IDisplayInput CaptureInput { get; }
		public string Name => CaptureInput.Name;

		ImageSource? CaptureSource { get; }
		
		public CaptureSessionViewModel(WorkspaceViewModel workspace, IDisplayInput input)
		{
			Workspace = workspace;
			CaptureInput = input;
		}
	}
}
