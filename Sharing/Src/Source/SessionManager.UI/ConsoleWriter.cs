// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;
using System.IO;
using System.Text;

namespace SessionManager.UI
{
    public class ConsoleWriter : TextWriter
    {
        public event Action<string> ConsoleLine;
        private TextWriter existingTextWriter;
        private StringBuilder stringBuilder = new StringBuilder();

        public ConsoleWriter()
        {
            this.existingTextWriter = Console.Out;
            Console.SetOut(this);
        }

        public override void Write(char value)
        {
            this.existingTextWriter.Write(value);

            stringBuilder.Append(value);

            // A bit ugly but oh well.
            if (System.Environment.NewLine.Contains(value.ToString())) 
            {
                string newString = stringBuilder.ToString();
                if (newString.EndsWith(System.Environment.NewLine))
                {
                    stringBuilder.Clear();

                    if (ConsoleLine != null)
                    {
                        ConsoleLine(newString);
                    }
                }
            }
        }

        public override void Write(string value)
        {
            this.existingTextWriter.Write(value);

            if (ConsoleLine != null)
            {
                ConsoleLine(value);
            }
        }

        public override Encoding Encoding
        {
            get { return Encoding.ASCII; }
        }
    }
}
