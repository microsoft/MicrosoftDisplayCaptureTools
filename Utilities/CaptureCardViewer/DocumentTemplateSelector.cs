using AvalonDock.Layout;
using CaptureCardViewer.ViewModels;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;

namespace CaptureCardViewer
{
	public class DocumentTemplateSelector : DataTemplateSelector
	{
		public override DataTemplate SelectTemplate(object item, DependencyObject container)
		{
			if (item is CaptureSessionViewModel)
			{
				return CaptureSessionTemplate;
			}
			else if (item is WorkspaceViewModel)
			{
				return WelcomeTemplate;
			}
			else if (item is LayoutDocument document)
			{
				return SelectTemplate(document.Content, document);
			}
			else
			{
				return base.SelectTemplate(item, container);
			}
		}

		public DataTemplate CaptureSessionTemplate { get; set; }
		public DataTemplate WelcomeTemplate { get; set; }
	}
}
