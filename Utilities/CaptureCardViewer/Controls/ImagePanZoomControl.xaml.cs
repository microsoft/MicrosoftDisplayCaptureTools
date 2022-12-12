using Microsoft.Win32;
using System;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace CaptureCardViewer.Controls
{
	/// <summary>
	/// Interaction logic for ImagePanZoomControl.xaml
	/// </summary>
	public partial class ImagePanZoomControl : UserControl
	{
		public ImagePanZoomControl()
		{
			InitializeComponent();
		}

		public static readonly DependencyProperty ImageSourceProperty = DependencyProperty.Register(
			"ImageSource", typeof(ImageSource), typeof(ImagePanZoomControl), new PropertyMetadata(default(ImageSource)));

		public ImageSource ImageSource
		{
			get => (ImageSource)GetValue(ImageSourceProperty);
			set => SetValue(ImageSourceProperty, value);
		}

		public static readonly DependencyProperty ZoomFactorProperty = DependencyProperty.Register(
			"ZoomFactor", typeof(double), typeof(ImagePanZoomControl), new PropertyMetadata(1.0));

		public double ZoomFactor
		{
			get => (double)GetValue(ZoomFactorProperty);
			set => SetValue(ZoomFactorProperty, value);
		}

		private void ZoomToActual_Click(object sender, RoutedEventArgs e)
		{
			if (ImageSource != null)
			{
				ZoomFactor = ImageSource.Width / imageView.RenderSize.Width;
			}
		}

		private void ZoomToFit_Click(object sender, RoutedEventArgs e)
		{
			ZoomFactor = 1;
		}

		private void SaveImage_Click(object sender, RoutedEventArgs e)
		{
			var bitmap = ImageSource as BitmapSource;

			if (bitmap != null)
			{
				var saveDialog = new SaveFileDialog();
				saveDialog.Filter = "PNG Files (*.png)|*.png";
				if (saveDialog.ShowDialog() ?? false)
				{
					try
					{
						PngBitmapEncoder encoder = new PngBitmapEncoder();
						encoder.Frames.Add(BitmapFrame.Create(bitmap));

						using (var stream = new FileStream(saveDialog.FileName, FileMode.Create, FileAccess.ReadWrite))
							encoder.Save(stream);
					}
					catch (Exception ex)
					{
						MessageBox.Show("Failed to save image.\r\n" + ex.Message);
					}
				}
			}
		}

		private void OnMouseWheel(object sender, MouseWheelEventArgs e)
		{
			if (Keyboard.Modifiers == ModifierKeys.Control)
			{
				ZoomFactor += e.Delta / 1000.0;
				e.Handled = true;
			}

		}
    }
}
