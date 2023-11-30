using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using MicrosoftDisplayCaptureTools.CaptureCard;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Threading.Tasks;

namespace CaptureCardViewer.ViewModels
{
	public partial class CaptureCardViewModel : ObservableObject
	{
		ObservableCollection<DisplayInputViewModel> inputs = new();

		public WorkspaceViewModel Workspace { get; }
		public IController Controller { get; }
		public IControllerWithFirmware? Firmware { get; }

		public string Name => Controller.Name;
		public string Version => Controller.Version.ToString();

		#region Firmware Update Properties
		public ControllerFirmwareState FirmwareStatus => Firmware?.FirmwareState ?? ControllerFirmwareState.UpToDate;
		public bool? NeedsFirmwareUpdate => (Firmware?.FirmwareState ?? ControllerFirmwareState.UpToDate) != ControllerFirmwareState.UpToDate;
		public string FirmwareVersion => Firmware?.FirmwareVersion ?? "Unknown";
		public bool CanUpdateFirmware => (NeedsFirmwareUpdate == true) && !IsUpdatingFirmware;
		[ObservableProperty]
		[NotifyPropertyChangedFor(nameof(CanUpdateFirmware))]
		[NotifyPropertyChangedFor(nameof(FirmwareVersion))]
		[NotifyPropertyChangedFor(nameof(FirmwareStatus))]
		[NotifyPropertyChangedFor(nameof(NeedsFirmwareUpdate))]
		bool isUpdatingFirmware;
		#endregion

		public CaptureCardViewModel(WorkspaceViewModel workspace, IController controller)
		{
			Workspace = workspace;
			Controller = controller;

			var controllerInputs = controller.EnumerateDisplayInputs();
			if (controllerInputs != null)
			{
				foreach (var input in controllerInputs)
					inputs.Add(new DisplayInputViewModel(workspace, this, input));

				Inputs = new ReadOnlyObservableCollection<DisplayInputViewModel>(inputs);
			}

			Firmware = Controller as IControllerWithFirmware;
		}

		public ReadOnlyObservableCollection<DisplayInputViewModel> Inputs { get; }

		[RelayCommand]
		async void UpdateFirmware()
		{
			if (Firmware == null)
			{
				return;
			}

			IsUpdatingFirmware = true;
			try
			{
				List<DisplayInputViewModel> newInputs = new();

				// Perform the update on a background thread
				await Task.Run(async () =>
				{
					await Firmware.UpdateFirmwareAsync();

					// Re-enumerate inputs after the firmware update, still on the background thread
					newInputs.AddRange(Controller.EnumerateDisplayInputs()
						.Select((input) => new DisplayInputViewModel(Workspace, this, input)));
				});

				// Update the ObservableCollection back on the UI thread
				inputs.Clear();
				foreach (var input in newInputs)
				{
					inputs.Add(input);
				}
			}
			catch (Exception ex)
			{
				ModernWpf.MessageBox.Show("Failed to update firmware.\r\n" + ex.Message);
			}

			IsUpdatingFirmware = false;
		}
	}
}
