// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;
using System.Windows;
using System.Windows.Controls;

using SessionManager.Network;
using SessionManager.DataModel;
using System.Collections.Specialized;

namespace SessionManager.UI
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        static int logCount = 0;

        public NetworkConnectionData NetworkConnectionData { get; private set; }
        public SessionsList Sessions { get; private set; }
        public SessionData CurrentSession { get; private set; }
        private string CurrentSessionName { get; set; }

        public MainWindow()
        {
            this.NetworkConnectionData = new NetworkConnectionData();
            this.Sessions = new SessionsList();
            this.CurrentSession = new SessionData(null);
            this.CurrentSessionName = string.Empty;

            InitializeComponent();

            this.currentSessionControl.DataContext = this.CurrentSession;

            if (App.XToolsApp.HasArgument("joinsession"))
            {
                Sessions.CollectionChanged += Sessions_CollectionChanged;
            }
            App.XToolsApp.SessionManagerListener.SessionAdded += SessionManagerListener_SessionAdded;

            App.ConsoleLineWriter.ConsoleLine += ConsoleWriter_ConsoleLine;
        }

        private void Sessions_CollectionChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            if (!CurrentSession.IsValid && e.Action == NotifyCollectionChangedAction.Add)
            {
                SessionData session = (SessionData)e.NewItems[0];
                if (session.Name == App.XToolsApp.GetArgumentOrDefault("joinsession", null))
                {
                    this.CurrentSession = session;
                    this.currentSessionControl.DataContext = this.CurrentSession;
                    this.CurrentSessionName = this.CurrentSession.Name;
                    App.XToolsApp.JoinSession(this.CurrentSessionName);
                }
            }
        }

        public void UpdateCurrentSession()
        {
            Dispatcher.BeginInvoke(new Action<MainWindow>((sender) =>
            {
                SessionData newSession = Sessions.FindSession(this.CurrentSessionName);
                if (newSession != null && newSession != this.CurrentSession )
                {
                    this.CurrentSession = newSession;
                    this.currentSessionControl.DataContext = this.CurrentSession;
                }
            }), new object[] { this });
        }

        private void SessionManagerListener_SessionAdded(HoloToolkit.Sharing.Session session)
        {
            UpdateCurrentSession();
        }

        private void ConsoleWriter_ConsoleLine(string newString)
        {
            Dispatcher.BeginInvoke(new Action<MainWindow>((sender) =>
            {
                logCount++;

                if (this.statusTextList.Items.Count > 1000)
                {
                    this.statusTextList.Items.RemoveAt(0);
                }

                this.statusTextList.Items.Add(new TextBlock()
                    {
                        Text = string.Format("{0:00000}: {1}", logCount, newString),
                        TextWrapping = TextWrapping.Wrap
                    });
                this.statusScroller.ScrollToBottom();
            }), new object[] { this });
        }

        private void CreateButton_Clicked(object sender, RoutedEventArgs e)
        {
            CreateSessionDialog dialog = new CreateSessionDialog(App.XToolsApp.SessionManager.GetCurrentUser().GetName().GetString() + "'s Session");
            dialog.Owner = this;

            bool? result = dialog.ShowDialog();

            if (result.HasValue && result.Value)
            {
                this.CurrentSessionName = dialog.SessionName;
                App.XToolsApp.CreateSession(dialog.SessionName);
            }
        }

        private void JoinButton_Clicked(object sender, RoutedEventArgs e)
        {
            Button button = (Button)sender;
            if (button.DataContext is SessionData)
            {
                this.CurrentSession = (SessionData)button.DataContext;
                this.currentSessionControl.DataContext = this.CurrentSession;
                this.CurrentSessionName = this.CurrentSession.Name;
                App.XToolsApp.JoinSession(this.CurrentSessionName);
            }
        }

        private void LeaveButton_Clicked(object sender, RoutedEventArgs e)
        {
            App.XToolsApp.LeaveSession();
            this.CurrentSessionName = string.Empty;
            this.CurrentSession = new SessionData(null);
            this.currentSessionControl.DataContext = this.CurrentSession;
        }

        private void SettingsButton_Clicked(object sender, RoutedEventArgs e)
        {
            ModifySettingsDialog dialog = new ModifySettingsDialog(this.NetworkConnectionData.UserName, this.NetworkConnectionData.ServerAddress, this.NetworkConnectionData.ViewerAddress);
            dialog.Owner = this;

            bool? result = dialog.ShowDialog();

            if (result.HasValue && result.Value)
            {
                if (!dialog.UserName.Equals(this.NetworkConnectionData.UserName))
                {
                    this.NetworkConnectionData.UserName = dialog.UserName;
                }

                if (!dialog.ServerAddress.Equals(this.NetworkConnectionData.ServerAddress))
                {
                    this.NetworkConnectionData.ServerAddress = dialog.ServerAddress;
                }

                if (!dialog.ViewerAddress.Equals(this.NetworkConnectionData.ViewerAddress))
                {
                    this.NetworkConnectionData.ViewerAddress = dialog.ViewerAddress;
                }
            }
        }

        private void MuteButton_Clicked(object sender, RoutedEventArgs e)
        {
            this.NetworkConnectionData.UserMuteState = !this.NetworkConnectionData.UserMuteState;
        }
    }
}
