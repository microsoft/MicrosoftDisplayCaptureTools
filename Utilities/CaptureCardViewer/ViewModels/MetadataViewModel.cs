using System.Collections.Generic;
using System.Linq;

namespace CaptureCardViewer.ViewModels
{
	/// <summary>
	/// Contains a single key-value pair that can be exposed to the UI.
	/// </summary>
	public class MetadataViewModel
	{
		public string Name { get; }
		public string Value { get; }

		public MetadataViewModel(string name, object value)
		{
			Name = name;
			if (value is IReadOnlyList<byte> bytes)
			{
				Value = string.Join(" ", bytes.Select(b => b.ToString("X2")));
			}
			else
			{
				Value = value?.ToString() ?? "null";
			}
		}
	}
}
