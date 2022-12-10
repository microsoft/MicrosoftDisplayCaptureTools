using MicrosoftDisplayCaptureTools.CaptureCard;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CaptureCardViewer.ViewModels
{
	public partial class CaptureCardViewModel
	{
		ObservableCollection<DisplayInputViewModel> inputs = new();
			
		public IController Controller { get; }

		public string Name => Controller.Name;
		public string Version => Controller.Version;

		public CaptureCardViewModel(IController controller)
		{
			Controller = controller;
			
			foreach (var input in controller.EnumerateDisplayInputs())
				inputs.Add(new DisplayInputViewModel(input));
			
			Inputs = new ReadOnlyObservableCollection<DisplayInputViewModel>(inputs);
		}

		public ReadOnlyObservableCollection<DisplayInputViewModel> Inputs { get; }
	}
}
