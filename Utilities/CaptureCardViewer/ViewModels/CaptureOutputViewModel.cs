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
	public partial class CaptureOutputViewModel : ObservableObject
	{
		public IDisplayInput CaptureInput { get; }
		public string Name => CaptureInput.Name;

		ImageSource? CaptureSource { get; }
		
		public CaptureOutputViewModel(IDisplayInput input)
		{
			CaptureInput = input;
		}
	}
}
