using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using AudioIO_DLL.IO;

namespace AudioIO_DLL.Core.Interfaces
{
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Size = 16)]
    public struct AudioClientProperties
    {
        public uint cbSize;

        [MarshalAs(UnmanagedType.Bool)]
        public bool bIsOffload;

        public AudioStreamCategory eCategory;
        public AudioClientStreamOptions Options;
    }
}
