using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;
using Windows.Graphics.Display;

namespace CaptureCardViewer
{
    public static class FluentBrushes
    {
		public static Brush CranberryActive { get; } = new SolidColorBrush(Color.FromRgb(0xdc, 0x62, 0x6d));
		public static Brush PeachBorderActive { get; } = new SolidColorBrush(Color.FromRgb(0xff, 0xba, 0x66));
		public static Brush LightGreenForeground1 { get; } = new SolidColorBrush(Color.FromRgb(0x5e, 0xc7, 0x5a));
		public static Brush YellowForeground1 { get; } = new SolidColorBrush(Color.FromRgb(0xfe, 0xee, 0x66));

		static FluentBrushes()
		{
			CranberryActive.Freeze();
			PeachBorderActive.Freeze();
			LightGreenForeground1.Freeze();
			YellowForeground1.Freeze();
		}
	}
}
