using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace ProfilerX
{
    public class LogData : INotifyPropertyChanged
    {
        private XTools.LogMessage logMsg;

        public event PropertyChangedEventHandler PropertyChanged;


        public XTools.LogSeverity Severity
        {
            get { return this.logMsg.GetSeverity(); }
        }

        public string Message
        {
            get { return this.logMsg.GetLogMessage(); }
        }

        public string MessageWithSeverity
        {
            get { return this.logMsg.GetSeverity().ToString() + ": " + this.logMsg.GetLogMessage(); }
        }

        public LogData(XTools.LogMessage msg)
        {
            this.logMsg = msg;
        }

        protected void OnPropertyChanged([CallerMemberName] string propertyName = null)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(propertyName));
            }
        }
    }
}
