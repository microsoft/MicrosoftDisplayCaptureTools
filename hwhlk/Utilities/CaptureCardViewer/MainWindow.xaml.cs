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
using Microsoft.Win32;
using MicrosoftDisplayCaptureTools.Framework;

namespace CaptureCardViewer
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        Core? testFramework;

        public MainWindow()
        {
            InitializeComponent();
        }

        private void LoadPlugin_Click(object sender, RoutedEventArgs e)
        {
            var dialog = new OpenFileDialog();
            dialog.Filter = "Plugin DLLs|*.dll";
            dialog.Title = "Load a Capture Plugin";

            if (dialog.ShowDialog() ?? false)
            {
                try
                {
					// TODO: How do we determine the controller class name?
                    testFramework = new Core();
					testFramework.LoadPlugin(dialog.FileName, "TestPlugin.Controller");
                }
                catch (Exception)
                {

                }
            }
        }
    }
}
