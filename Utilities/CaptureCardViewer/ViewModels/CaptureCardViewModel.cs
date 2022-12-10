using MicrosoftDisplayCaptureTools.CaptureCard;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;
using System.Windows.Media.Converters;
using WinRT;

namespace CaptureCardViewer.ViewModels
{
	public partial class CaptureCardViewModel
	{
		ObservableCollection<DisplayInputViewModel> inputs = new();
			
		public IController Controller { get; }
		public IControllerWithFirmware? Firmware { get; }

		public string Name => Controller.Name;
		public string Version => Controller.Version;
		public bool? NeedsFirmwareUpdate => (Firmware?.FirmwareState ?? ControllerFirmwareState.UpToDate) != ControllerFirmwareState.UpToDate;
		public string FirmwareVersion => Firmware?.FirmwareVersion ?? "Unknown";
		
		public CaptureCardViewModel(IController controller)
		{
			Controller = controller;
			
			foreach (var input in controller.EnumerateDisplayInputs())
				inputs.Add(new DisplayInputViewModel(input));
			
			Inputs = new ReadOnlyObservableCollection<DisplayInputViewModel>(inputs);

			Firmware = Controller as IControllerWithFirmware;
		}

		public ReadOnlyObservableCollection<DisplayInputViewModel> Inputs { get; }
	}
}
