using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace AudioIO_DLL.Utils
{
    class EventWriterDLL
    {
#if !WSA81
        private IntPtr eventSrcHandle;
#endif

        private StringBuilder buildAString = new StringBuilder();

        public enum SeverityTypes : ushort
        {
            Success = 0x0,
            Error = 0x1,
            Warning = 0x2,
            Information = 0x4,
            AuditSuccess = 0x8,
            AuditFailure = 0x10
        }

        public void Initialize(string myName)
        {
#if !WSA81
            this.eventSrcHandle = EventWriterDLL.RegisterEventSource(null, myName);
#endif
        }

        public void Uninitialize()
        {
#if !WSA81
            EventWriterDLL.DeregisterEventSource(eventSrcHandle);
#endif
        }

        private string Format = "{0:HH\\:mm\\:ss\\:ffff}\t=> '{1}'\n";

        public void BuildLine(string msg)
        {
            buildAString.AppendFormat(Format, DateTime.Now, msg);
        }

        public void FlushBuildString(SeverityTypes severity, uint id)
        {
            this.WriteLine(severity, id, buildAString.ToString());
#if WSA81
            buildAString.Clear();
#endif
        }

        public void WriteLine(SeverityTypes severity, uint id, string msg)
        {
            try
            {
                string[] message = new string[1];
                message[0] = msg;
#if !WSA81
                bool bresult = EventWriterDLL.ReportEvent(this.eventSrcHandle, (ushort)severity, 0, id, null, 1, 0, message, null);
#endif
            }
            catch (Exception e)
            {
                Debug.WriteLine("Exception in EventWriterDLL::WriteLine: " + e.ToString());
            }
        }

#if !WSA81
        [DllImport("ADVAPI32", CharSet = System.Runtime.InteropServices.CharSet.Unicode, SetLastError = true)]
        internal static extern bool ReportEvent(IntPtr hEventLog, ushort type, ushort category,
                                                uint eventID, byte[] userSID, ushort numStrings, uint dataLen, string[] strings,
                                                byte[] rawData);

        [DllImport("ADVAPI32", CharSet = System.Runtime.InteropServices.CharSet.Unicode, SetLastError = true)]
        internal static extern IntPtr RegisterEventSource(string uncServerName, string sourceName);

        [DllImport("ADVAPI32", CharSet = System.Runtime.InteropServices.CharSet.Unicode, SetLastError = true)]
        internal static extern IntPtr DeregisterEventSource(IntPtr uncServerName);
#endif
    }
}
