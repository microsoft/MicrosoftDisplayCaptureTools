using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

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
			Value = value?.ToString() ?? "null";
		}
	}
}
