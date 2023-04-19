using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using Microsoft.Win32;
using MicrosoftDisplayCaptureTools.Framework;
using System;
using System.IO;
using System.Threading;
using System.Threading.Channels;
using System.Windows;
using System.Windows.Documents;
using System.Windows.Media;
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
		ILoggerMode loggingMode = new RichTextLoggerMode();

		private class RichTextLoggerMode : ILoggerMode
		{
			public bool HasErrored()
			{
				return true;
			}
		}

		/// <summary>
		/// Represents a single log entry for passing to the UI thread to be processed into
		/// the FlowDocument blocks.
		/// </summary>
		record LogEntry(string Message, Color Color, DateTime Time);

		public RichTextLogger()
		{
			dispatcher = Dispatcher.CurrentDispatcher;
			document = new FlowDocument()
			{
				FontFamily = new FontFamily("Consolas"),
				FontSize = 14,
				TextAlignment = System.Windows.TextAlignment.Left
			};
			entriesChannel = Channel.CreateUnbounded<LogEntry>();
		}

		/// <summary>
		/// The log's FlowDocument which is bound to the UI.
		/// </summary>
		public FlowDocument Document => document;

		[ICommand]
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
					ModernWpf.MessageBox.Show("Failed to save log file:\r\n" + ex.Message);
				}
			}
		}

		[ICommand]
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
				// In theory, dispatcher.BeginInvoke should work like a queue, but the ordering may
				// not be a strict guarantee. To ensure this won't ever be a problem, we use a Channel
				// to pass the entry to the UI thread and then the UI thread processes Channel entries
				// in guaranteed sequence.

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
			SubmitEntry(new LogEntry("ASSERT: " + error, FluentBrushes.PeachBorderActiveColor, DateTime.Now));
		}

		public void LogConfig(string config)
		{
			SubmitEntry(new LogEntry(config, FluentBrushes.BlueBorderActiveColor, DateTime.Now));
		}

		public void LogError(string error)
		{
			SubmitEntry(new LogEntry(error, FluentBrushes.CranberryActiveColor, DateTime.Now));
		}

		public void LogNote(string note)
		{
			SubmitEntry(new LogEntry(note, Colors.White, DateTime.Now));
		}

		public void LogWarning(string warning)
		{
			SubmitEntry(new LogEntry(warning, FluentBrushes.YellowForegroundColor, DateTime.Now));
		}

		public ILoggerMode LogErrorsAsWarnings()
		{
			return loggingMode;
		}
	}
}
