using MicrosoftDisplayCaptureTools.CaptureCard;
using System;
using System.ComponentModel;
using System.Globalization;
using System.Windows;
using System.Windows.Data;
using System.Windows.Media;

namespace CaptureCardViewer
{
	public class FirmwareUpdateStatusConverter : TypeConverter, IValueConverter
	{
		public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
		{
			return ConvertTo(value, targetType);
		}

		public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
		{
			throw new NotImplementedException();
		}

		public override object? ConvertTo(ITypeDescriptorContext? context, CultureInfo? culture, object? value, Type destinationType)
		{
			if (value is ControllerFirmwareState status)
			{
				if (destinationType == typeof(string) || destinationType == typeof(object))
				{
					return status switch
					{
						ControllerFirmwareState.UpToDate => "Up to date",
						ControllerFirmwareState.UpdateAvailable => "Update available",
						ControllerFirmwareState.ManualUpdateNeeded => "Manual update required",
						ControllerFirmwareState.UpdateRequired => "Update required",
						_ => throw new NotImplementedException(),
					};
				}
				else if (destinationType == typeof(Visibility))
				{
					return status switch
					{
						ControllerFirmwareState.UpToDate => Visibility.Collapsed,
						ControllerFirmwareState.UpdateAvailable => Visibility.Visible,
						ControllerFirmwareState.ManualUpdateNeeded => Visibility.Visible,
						ControllerFirmwareState.UpdateRequired => Visibility.Visible,
						_ => throw new NotImplementedException(),
					};
				}
				else if (destinationType == typeof(Brush))
				{
					return status switch
					{
						ControllerFirmwareState.UpToDate => Brushes.Transparent,
						ControllerFirmwareState.UpdateAvailable => FluentBrushes.YellowForeground1,
						ControllerFirmwareState.ManualUpdateNeeded => FluentBrushes.CranberryActive,
						ControllerFirmwareState.UpdateRequired => FluentBrushes.PeachBorderActive,
						_ => throw new NotImplementedException(),
					};
				}
			}
			return base.ConvertTo(context, culture, value, destinationType);
		}
	}
}
