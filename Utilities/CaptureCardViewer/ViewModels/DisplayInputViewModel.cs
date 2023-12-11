using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using Microsoft.Win32;
using MicrosoftDisplayCaptureTools.CaptureCard;
using MicrosoftDisplayCaptureTools.Framework;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

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

		[RelayCommand]
		void CreateCaptureSession()
		{
			Workspace.SelectedCaptureCard = CaptureCard;
			Workspace.SelectedDisplayInput = this;

			Workspace.CreateCaptureSessionCommand.Execute(null);
		}

		[RelayCommand]
		async void SetDescriptorFromFile()
		{
			var openDialog = new OpenFileDialog();
			openDialog.Filter = "ASCII Hex Files (*.txt)|*.txt|Binary files (*.bin)|*.bin";
			if (openDialog.ShowDialog() == true)
			{
				try
				{
					byte[] descriptorData;

					if (openDialog.FilterIndex == 0)
					{
						// ASCII Hex
						var text = new StringBuilder(File.ReadAllText(openDialog.FileName));
						text.Replace("0x", null);
						text.Replace("0X", null);
						text.Replace(" ", null);
						text.Replace(",", null);
						text.Replace("\r", null);
						text.Replace("\n", null);

						if (text.Length % 2 != 0)
							throw new FormatException("The text file may contain only hex numbers and optionally commas, spaces, and 0x prefixes");

						List<byte> bytes = new();
						foreach (var byteChars in text.ToString().Chunk(2))
						{
							bytes.Add(byte.Parse(byteChars, System.Globalization.NumberStyles.HexNumber));
						}
						descriptorData = bytes.ToArray();
					}
					else
					{
						// Binary
						descriptorData = File.ReadAllBytes(openDialog.FileName);
					}

					await Task.Run(async () =>
					{
						var descriptor = MonitorDescriptorLoader.LoadDescriptorFromVectorView(descriptorData);

						Input.SetDescriptor(descriptor);
					});
				}
				catch (Exception ex)
				{
					ModernWpf.MessageBox.Show("Failed to load monitor descriptor.\r\n" + ex.Message);
				}
			}
		}
	}
}
