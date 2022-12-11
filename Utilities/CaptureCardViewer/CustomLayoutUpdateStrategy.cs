using AvalonDock.Layout;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CaptureCardViewer
{
	/// <summary>
	/// This class is used to customize the layout of the AvalonDock control when panes are added.
	/// </summary>
	class CustomLayoutUpdateStrategy : ILayoutUpdateStrategy
	{
		public void AfterInsertAnchorable(LayoutRoot layout, LayoutAnchorable anchorableShown)
		{
		}

		public void AfterInsertDocument(LayoutRoot layout, LayoutDocument anchorableShown)
		{
			// Set the newly inserted document as the active (focused) pane
			layout.ActiveContent = anchorableShown;
		}

		public bool BeforeInsertAnchorable(LayoutRoot layout, LayoutAnchorable anchorableToShow, ILayoutContainer destinationContainer)
		{
			return false;
		}

		public bool BeforeInsertDocument(LayoutRoot layout, LayoutDocument anchorableToShow, ILayoutContainer destinationContainer)
		{
			return false;
		}
	}
}
