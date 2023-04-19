using System;
using System.Windows;
using System.Windows.Interop;

namespace CaptureCardViewer
{
	static class TitlebarCustomizations
	{
		public static bool GetDarkModeTitlebar(DependencyObject obj)
		{
			return (bool)obj.GetValue(DarkModeTitlebarProperty);
		}

		public static void SetDarkModeTitlebar(DependencyObject obj, bool value)
		{
			obj.SetValue(DarkModeTitlebarProperty, value);
		}

		// Using a DependencyProperty as the backing store for DarkModeTitlebar.  This enables animation, styling, binding, etc...
		public static readonly DependencyProperty DarkModeTitlebarProperty =
			DependencyProperty.RegisterAttached("DarkModeTitlebar", typeof(bool), typeof(TitlebarCustomizations), new PropertyMetadata(false, (a, b) =>
			{
				if (b.NewValue is bool newValue && a is Window window)
				{
					// Get the HWND from a WPF window:
					IntPtr hwnd = new WindowInteropHelper(window).EnsureHandle();

					int nativeBool = newValue ? 1 : 0;
					unsafe
					{
						Windows.Win32.PInvoke.DwmSetWindowAttribute(new Windows.Win32.Foundation.HWND(hwnd), Windows.Win32.Graphics.Dwm.DWMWINDOWATTRIBUTE.DWMWA_USE_IMMERSIVE_DARK_MODE, &nativeBool, sizeof(int));
					}

				}
			}));
	}
}
