using MicrosoftDisplayCaptureTools.CaptureCard;
using CommunityToolkit.Mvvm.ComponentModel;
using System.Collections.Generic;
using System.Linq;
using CommunityToolkit.Mvvm.Input;
using System.Windows;

namespace CaptureCardViewer.ViewModels
{
	public partial class DisplayInputViewModel : ObservableObject
	{
		public WorkspaceViewModel Workspace { get; }
		public CaptureCardViewModel CaptureCard { get; }
		public IDisplayInput Input { get; }
		public ICaptureCapabilities Capabilities { get; }
		
		public string Name => Input.Name;
		public bool CanReturnRawFramesToHost => Capabilities.CanReturnRawFramesToHost();
		public bool CanReturnFramesToHost => Capabilities.CanReturnFramesToHost();
		public bool CanCaptureFrameSeries => Capabilities.CanCaptureFrameSeries();
		public bool CanHotPlug => Capabilities.CanHotPlug();
		public bool CanConfigureEDID => Capabilities.CanConfigureEDID();
		public bool CanConfigureDisplayID => Capabilities.CanConfigureDisplayID();
		public int MaxDescriptorSize => (int)Capabilities.GetMaxDescriptorSize();

		public string CapabilitiesString
		{
			get
			{
				List<string> caps = new();
				if (CanReturnFramesToHost) caps.Add("Frame Capture");
				if (CanReturnRawFramesToHost) caps.Add("Raw Frames");
				if (CanCaptureFrameSeries) caps.Add("Series Capture");
				if (CanHotPlug) caps.Add("Hotplug");
				if (CanConfigureEDID) caps.Add("EDID");
				if (CanConfigureDisplayID) caps.Add("DisplayID");
				return string.Join(", ", caps);
			}
		}

		public DisplayInputViewModel(WorkspaceViewModel workspace, CaptureCardViewModel parent, IDisplayInput input)
		{
			Workspace = workspace;
			CaptureCard = parent;
			Input = input;

			Capabilities = input.GetCapabilities();
		}

		[ICommand]
		void CreateCaptureSession()
		{
			if (Workspace.DisplayEngines.Count == 0)
			{
				MessageBox.Show("You must load at least one Render Engine before creating a capture session.");
				return;
			}

			// TODO: Allow selecting a specific render engine
			var newSession = new CaptureSessionViewModel(Workspace, Workspace.DisplayEngines.First().Engine, CaptureCard, Input);
			Workspace.Documents.Add(newSession);
		}
	}
}
