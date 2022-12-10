using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;

namespace CaptureCardViewer.Converters
{
    class FirmwareUpdateStatusConverter : TypeConverter
    {
		public override object? ConvertTo(ITypeDescriptorContext? context, CultureInfo? culture, object? value, Type destinationType)
		{
			return base.ConvertTo(context, culture, value, destinationType);
		}
		
		/*public Color FirmwareUpdateNotificationColor
		{
			get
			{
				switch (Firmware?.FirmwareState ?? ControllerFirmwareState.UpToDate)
				{
					case ControllerFirmwareState.ManualUpdateNeeded:
						return Colors.Red;
					case ControllerFirmwareState.AutoUpdateNeeded:
						return Colors.Red;
					case ControllerFirmwareState.UpToDate:
						return Colors.Green;
					case
				}
			}
		}*/

	}
}
