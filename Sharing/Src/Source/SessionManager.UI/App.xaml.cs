// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Threading;

namespace SessionManager.UI
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        static public Network.XToolsApp XToolsApp;
        static private DispatcherTimer dispatcherTimer;
        static public ConsoleWriter ConsoleLineWriter { get; private set; }

        protected override void OnExit(ExitEventArgs e)
        {
            base.OnExit(e);

            if (dispatcherTimer != null)
            {
                dispatcherTimer.Stop();
                dispatcherTimer = null;
            }

            if (XToolsApp != null)
            {
                XToolsApp.Dispose();
                XToolsApp = null;
            }
        }
        protected override void OnStartup(StartupEventArgs e)
        {
            base.OnStartup(e);

            XToolsApp = new Network.XToolsApp(e.Args);

            dispatcherTimer = new DispatcherTimer();
            dispatcherTimer.Tick += NetworkUpdate;
            dispatcherTimer.Interval = new TimeSpan(0, 0, 0, 0, 60);
            dispatcherTimer.Start();

            ConsoleLineWriter = new ConsoleWriter();
        }
        private void NetworkUpdate(object sender, EventArgs e)
        {
            XToolsApp.Update();
        }
    }
}
