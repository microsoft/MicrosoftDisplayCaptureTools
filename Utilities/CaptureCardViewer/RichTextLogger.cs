using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using Microsoft.Win32;
using MicrosoftDisplayCaptureTools.Framework;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Channels;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Documents;
using System.Windows.Media;
using System.Windows.Media.Converters;
using System.Windows.Threading;

namespace CaptureCardViewer
{
	/// <summary>
	/// Implements ILogger for the framework to support generating a FlowDocument that's
	/// directly bound to the UI.
	/// </summary>
	[ObservableObject]
	public partial class RichTextLogger : ILogger
	{
		FlowDocument document;
		Dispatcher dispatcher;
		Channel<LogEntry> entriesChannel;

		record LogEntry(string Message, Color Color, DateTime Time);

		public RichTextLogger()
		{
			dispatcher = Dispatcher.CurrentDispatcher;
			document = new FlowDocument()
			{
				FontFamily = new FontFamily("Consolas"),
				TextAlignment = System.Windows.TextAlignment.Left
			};
			entriesChannel = Channel.CreateUnbounded<LogEntry>();
		}

		public FlowDocument Document => document;

		[RelayCommand]
		public void SaveLog()
		{
			var saveDialog = new SaveFileDialog();
			saveDialog.Title = "Save Log";
			saveDialog.Filter = "Text File (*.txt)|*.txt|Rich Text File (*.rtf)|*.rtf";
			if (saveDialog.ShowDialog() ?? false)
			{
				try
				{
					using (var stream = new FileStream(saveDialog.FileName, FileMode.Create))
					{
						TextRange range = new TextRange(document.ContentStart, document.ContentEnd);
						range.Save(stream, saveDialog.FilterIndex == 1 ? DataFormats.Rtf : DataFormats.Text);
					}
				}
				catch (Exception ex)
				{
					MessageBox.Show("Failed to save log file:\r\n" + ex.Message);
				}
			}
		}

		[RelayCommand]
		public void ClearLog()
		{
			document.Blocks.Clear();
		}

		private void AppendEntry(LogEntry entry)
		{
			document.Blocks.Add(new Paragraph(new Run(
			$"{entry.Time.ToLongTimeString()}: {entry.Message}")
			{
				Foreground = new SolidColorBrush(entry.Color)
			}));
		}

		private void SubmitEntry(LogEntry entry)
		{
			if (dispatcher.Thread == Thread.CurrentThread)
			{
				AppendEntry(entry);
			}
			else
			{
				entriesChannel.Writer.TryWrite(entry);
				dispatcher.BeginInvoke(new Action(() =>
				{
					while (entriesChannel.Reader.TryRead(out var entry))
					{
						AppendEntry(entry);
					}
				}));
			}
		}

		public void LogAssert(string error)
		{
			SubmitEntry(new LogEntry("ASSERT: " + error, Colors.OrangeRed, DateTime.Now));
		}
		
		public void LogConfig(string config)
		{
			SubmitEntry(new LogEntry(config, Colors.Blue, DateTime.Now));
		}

		public void LogError(string error)
		{
			SubmitEntry(new LogEntry(error, Colors.Red, DateTime.Now));
		}

		public void LogNote(string note)
		{
			SubmitEntry(new LogEntry(note, Colors.White, DateTime.Now));
		}

		public void LogWarning(string warning)
		{
			SubmitEntry(new LogEntry(warning, Colors.Yellow, DateTime.Now));
		}
	}
}
