using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

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
	}
}
