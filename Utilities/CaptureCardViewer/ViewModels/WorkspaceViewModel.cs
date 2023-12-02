using CaptureCardViewer.Mocks;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using Microsoft.Win32;
using MicrosoftDisplayCaptureTools.ConfigurationTools;
using MicrosoftDisplayCaptureTools.Display;
using MicrosoftDisplayCaptureTools.Framework;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Reflection;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Threading;
using Windows.Web.AtomPub;

namespace CaptureCardViewer.ViewModels
{
	public partial class WorkspaceViewModel : ObservableObject
	{
		Core testFramework;
		Dispatcher dispatcher;

		[ObservableProperty]
		object activeContent;

		public Core Framework => testFramework;
		public string AppVersion => Assembly.GetExecutingAssembly().GetName().Version!.ToString();

		public ObservableCollection<ToolboxViewModel> Toolboxes { get; } = new();
		public ObservableCollection<CaptureCardViewModel> CaptureCards { get; } = new();
		public ObservableCollection<DisplayEngineViewModel> DisplayEngines { get; } = new();

		[ObservableProperty]
		public DisplayEngineViewModel selectedDisplayEngine;

		[ObservableProperty]
		public ToolboxViewModel selectedToolbox;

		[ObservableProperty]
		public DisplayInputViewModel selectedDisplayInput;

		[ObservableProperty]
		[NotifyPropertyChangedFor(nameof(SelectedDisplayInput))]
		public CaptureCardViewModel selectedCaptureCard;

		public RichTextLogger Logger { get; } = new RichTextLogger();

		public ObservableCollection<object> Documents { get; } = new();

		[RelayCommand]
		async void CreateCaptureSession()
		{
			if (SelectedToolbox == null || SelectedDisplayEngine == null || SelectedDisplayInput == null)
			{
				ModernWpf.MessageBox.Show("You must select a DisplayEngine, Toolbox, Capture Card and Input.");
				return;
			}

			var mapping = await Task.Run(() =>
				Framework.GetSourceToSinkMappings(
					true,
					SelectedDisplayEngine.Engine,
					SelectedToolbox.Toolbox,
					SelectedCaptureCard.Controller,
					SelectedDisplayInput.Input));

			if (mapping.Count == 0)
			{
				ModernWpf.MessageBox.Show("Unable to map the display outputs to the captured inputs for these selections.");
				return;
			}
			if (mapping.Count != 1)
			{
				ModernWpf.MessageBox.Show("Multiple display outputs were mapped to the same capture input. Please double check your setup.");
				return;
			}

			var newSession = new CaptureSessionViewModel(this, SelectedDisplayEngine.Engine, mapping.First(), SelectedToolbox);
			Documents.Add(newSession);
		}

		class UiRuntimeSettings : IRuntimeSettings
		{
			Dictionary<string, object> values = new();

			public object GetSettingValue(string settingName)
			{
				return values.TryGetValue(settingName, out var value) ? value : null;
			}

			public bool GetSettingValueAsBool(string settingName)
			{
				return values.TryGetValue(settingName, out var value) ? (value as bool? ?? false) : false;
			}

			public string GetSettingValueAsString(string settingName)
			{
				return values.TryGetValue(settingName, out var value) ? value as string : null;
			}
		}

		public WorkspaceViewModel()
		{
			testFramework = new Core(Logger, new UiRuntimeSettings());
			dispatcher = Dispatcher.CurrentDispatcher;
			var command = this.LoadFromConfigFileCommand;

			Documents.Add(this);

			// Start the discovery async
			_ = Task.Run(async () => await DiscoverInstalledPlugins());
		}

		public async Task DiscoverInstalledPlugins()
		{
			await Task.Run(() =>
			{
				testFramework.DiscoverInstalledPlugins();
			});

			try
			{
				await Application.Current.Dispatcher.Invoke(async () =>
				{
					await RefreshAllPlugins();
				});
			}
			catch (Exception ex)
			{
				ModernWpf.MessageBox.Show("Failed to load discovered plugins.\r\n" + ex.Message);
			}
		}

		private async Task RefreshAllPlugins()
		{
			CaptureCards.Clear();
			Toolboxes.Clear();
			DisplayEngines.Clear();

			// For each type of plugin, intialize the view models on a background thread and
			// then add them to the ObservableCollections on the UI thread

			(await Task.Run(() => testFramework.GetCaptureCards()?
				.Select((card) => new CaptureCardViewModel(this, card))?
				.ToList()))?
				.ForEach((card) => CaptureCards.Add(card));

			(await Task.Run(() => testFramework.GetConfigurationToolboxes()?
				.Select((toolbox) => new ToolboxViewModel(toolbox))?
				.ToList()))?
				.ForEach((toolbox) => Toolboxes.Add(toolbox));

			(await Task.Run(() => testFramework.GetDisplayEngines()?
				.Select((engine) => new DisplayEngineViewModel(engine))?
				.ToList()))?
				.ForEach((engine) => DisplayEngines.Add(engine));


			// Add a mock controller for Non-Capture Testing
			CaptureCards.Add(new CaptureCardViewModel(this, new MockController()));

			// Select the first capture card
			SelectedToolbox = Toolboxes.FirstOrDefault();
			SelectedDisplayEngine = DisplayEngines.FirstOrDefault();

			// Select the first input on the first capture card
			foreach (var card in CaptureCards)
			{
				if (card.Inputs != null && card.Inputs.Count > 0)
				{
					SelectedDisplayInput = card.Inputs.FirstOrDefault();
					SelectedCaptureCard = card;
					break;
				}
			}

			if (SelectedCaptureCard == null)
				SelectedCaptureCard = CaptureCards.FirstOrDefault();
		}

		[RelayCommand]
		async void LoadFromConfigFile()
		{
			var dialog = new OpenFileDialog(); //file picker
			dialog.Title = "Load a Configuration";
			if (dialog.ShowDialog() == true)
			{
				try
				{
					await Task.Run(() =>
					{
						// Create a new framework from scratch
						var newInstance = new Core(Logger, null);
						newInstance.LoadConfigFile(dialog.FileName);

						// Now that the configuration was successfully loaded, update the view models
						testFramework = newInstance;
					});

					await Application.Current.Dispatcher.Invoke(async () =>
					{
						await RefreshAllPlugins();
					});
				}
				catch (Exception ex)
				{
					ModernWpf.MessageBox.Show("Failed to load configuration.\r\n" + ex.Message);
				}
			}
		}

		// Loading Display Manager file
		[RelayCommand]
		async Task LoadDisplayEngineFromFile()
		{
			var dialog = new OpenFileDialog(); //file picker
			dialog.Title = "Load Display Engine file";
			if (dialog.ShowDialog() == true)
			{
				try
				{
					var filename = dialog.FileName.ToString();
					IDisplayEngine engine = await Task.Run(() =>
						{
							return testFramework.LoadDisplayEngine(filename);
						});

					DisplayEngines.Add(new DisplayEngineViewModel(engine));
				}
				catch (Exception ex)
				{
					ModernWpf.MessageBox.Show("Failed to load the display engine.\r\n" + ex.Message);
				}
			}
		}

		// Loading Capture Plugin file
		[RelayCommand]
		async Task LoadCaptureCardFromFile()
		{
			var dialog = new OpenFileDialog();
			dialog.Title = "Load Capture Plugin file";
			if (dialog.ShowDialog() == true)
			{
				try
				{
					var filename = dialog.FileName.ToString();
					CaptureCardViewModel? controller = null;

					await Task.Run(() =>
					{
						controller = new CaptureCardViewModel(this, testFramework.LoadCapturePlugin(filename));
					});

					CaptureCards.Add(controller!);
				}
				catch (Exception ex)
				{
					ModernWpf.MessageBox.Show("Failed to load the capture plugin.\r\n" + ex.Message);
				}

			}
		}

		// Loading Toolbox file
		[RelayCommand]
		async Task LoadToolboxFromFile()
		{
			var dialog = new OpenFileDialog();
			dialog.Title = "Load Toolbox file";
			if (dialog.ShowDialog() == true)
			{
				try
				{
					var filename = dialog.FileName.ToString();
					IConfigurationToolbox? toolbox = null;
					await Task.Run(() =>
					{
						toolbox = testFramework.LoadToolbox(filename);
					});
					Toolboxes.Add(new ToolboxViewModel(toolbox!));
				}
				catch (Exception ex)
				{
					ModernWpf.MessageBox.Show("Failed to load the toolbox.\r\n" + ex.Message);
				}
			}
		}
	}
}
