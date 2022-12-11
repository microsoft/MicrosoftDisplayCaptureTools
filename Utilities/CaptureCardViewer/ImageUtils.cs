using System;
using System.Runtime.InteropServices;

namespace CaptureCardViewer
{
	//
	// Add the interface so that we can access the raw pixel data.
	//
	[ComImport]
	[Guid("5B0D3235-4DBA-4D44-865E-8F1D0E4FD04D")]
	[InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
	unsafe interface IMemoryBufferByteAccess
	{
		void GetBuffer(out byte* buffer, out uint capacity);
	}
	
	class ImageUtils
    {
    }
}
