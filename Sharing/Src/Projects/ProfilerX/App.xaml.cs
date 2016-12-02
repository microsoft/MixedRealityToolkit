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

namespace ProfilerX
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        static public ProfilerApp ProfilerApp;
        static private DispatcherTimer dispatcherTimer;

        protected override void OnExit(ExitEventArgs e)
        {
            base.OnExit(e);

            if (dispatcherTimer != null)
            {
                dispatcherTimer.Stop();
                dispatcherTimer = null;
            }

            if (ProfilerApp != null)
            {
                ProfilerApp.Dispose();
                ProfilerApp = null;
            }
        }

        protected override void OnStartup(StartupEventArgs e)
        {
            base.OnStartup(e);

            ProfilerApp = new ProfilerApp();

            dispatcherTimer = new DispatcherTimer();
            dispatcherTimer.Tick += NetworkUpdate;
            dispatcherTimer.Interval = new TimeSpan(0, 0, 0, 0, 1000 / 30); // 30 updates per second
            dispatcherTimer.Start();
        }

        private void NetworkUpdate(object sender, EventArgs e)
        {
            ProfilerApp.Update();
        }
    }
}
