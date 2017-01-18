// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using HoloToolkit.Sharing;
using System;
using System.Diagnostics;

namespace SessionManagerUniversal.UI.Helpers
{
    public class ConsoleLogWriter : LogWriter
    {
        public Action<LogSeverity, string> Log; 

        public override void WriteLogEntry(LogSeverity severity, string message)
        {
            base.WriteLogEntry(severity, message);

            if (System.Diagnostics.Debugger.IsAttached)
            {
                System.Diagnostics.Debug.WriteLine(message);
            }

            Log?.Invoke(severity, message);
        }
    }
}
