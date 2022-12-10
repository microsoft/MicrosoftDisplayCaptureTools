using MicrosoftDisplayCaptureTools.CaptureCard;
using CommunityToolkit.Mvvm.ComponentModel;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CaptureCardViewer.ViewModels
{
	public partial class DisplayInputViewModel : ObservableObject
	{
		public IDisplayInput Input { get; }
		public string Name => Input.Name;

		public DisplayInputViewModel(IDisplayInput input)
		{
			Input = input;
		}
	}
}
