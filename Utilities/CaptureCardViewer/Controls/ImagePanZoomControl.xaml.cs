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
				imageView.Stretch = Stretch.None;
				ZoomFactor = 1;
			}
		}

		private void ZoomToFit_Click(object sender, RoutedEventArgs e)
		{
			ZoomFactor = 1;
			imageView.Stretch = Stretch.Uniform;
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
						ModernWpf.MessageBox.Show("Failed to save image.\r\n" + ex.Message);
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

		Point? mouseDragStartPoint;
		Point mouseDragInitialOffset;

		private void OnImageMouseDown(object sender, MouseButtonEventArgs e)
		{
			if (e.LeftButton == MouseButtonState.Pressed)
			{
				mouseDragStartPoint = e.GetPosition(this);
				mouseDragInitialOffset = new Point(imageScrollViewer.HorizontalOffset, imageScrollViewer.VerticalOffset);
				((FrameworkElement)sender).CaptureMouse();
			}
		}

		private void OnImageMouseMove(object sender, MouseEventArgs e)
		{
			if (e.LeftButton == MouseButtonState.Pressed && mouseDragStartPoint.HasValue)
			{
				var newPoint = e.GetPosition(this);
				imageScrollViewer.ScrollToHorizontalOffset(mouseDragStartPoint.Value.X - newPoint.X + mouseDragInitialOffset.X);
				imageScrollViewer.ScrollToVerticalOffset(mouseDragStartPoint.Value.Y - newPoint.Y + mouseDragInitialOffset.Y);
			}
		}

		private void OnImageMouseUp(object sender, MouseButtonEventArgs e)
		{
			mouseDragStartPoint = null;
			((FrameworkElement)sender).ReleaseMouseCapture();
		}
	}
}
