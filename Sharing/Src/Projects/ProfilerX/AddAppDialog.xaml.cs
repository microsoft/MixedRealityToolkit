// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Threading;
using HoloToolkit.Sharing;

namespace ProfilerX
{
    class RemoteSystemEntry : StackPanel
    {
        private DiscoveredSystem _remoteSystem;
        public DiscoveredSystem RemoteSystem
        {
            get { return _remoteSystem; }
            set
            {
                _remoteSystem = value;
                nameLabel.Content = value.GetName();
                addressLabel.Content = value.GetAddress();
                roleLabel.Content = value.GetRole().ToString();
            }
        }

        private Label nameLabel;
        private Label addressLabel;
        private Label roleLabel;

        public RemoteSystemEntry()
        {
            this.Orientation = Orientation.Horizontal;

            nameLabel = new Label();
            addressLabel = new Label();
            roleLabel = new Label();

            this.Children.Add(nameLabel);
            this.Children.Add(addressLabel);
            this.Children.Add(roleLabel);
        }

        public bool IsEqual(DiscoveredSystem remoteSystem)
        {
            return
                (_remoteSystem.GetName() == remoteSystem.GetName()) &&
                (_remoteSystem.GetAddress() == remoteSystem.GetAddress()) &&
                (_remoteSystem.GetRole() == remoteSystem.GetRole());
        }
    }

    /// <summary>
    /// Interaction logic for AddAppDialog.xaml
    /// </summary>
    public partial class AddAppDialog : Window
    {
        public string RemoteAppName { get; private set; }
        public SystemRole RemoteAppRole { get; private set; }

        private DispatcherTimer updateTimer;
        private DispatcherTimer refreshTimer;

        private DiscoveryClient discoveryClient;
        private DiscoveryClientAdapter listener;

        private readonly int pingRefreshRate = 2;   // seconds

        public AddAppDialog()
        {
            InitializeComponent();

            listener = new DiscoveryClientAdapter();
            listener.DiscoveredEvent += OnSystemDiscovered;
            listener.LostEvent += OnSystemLost;

            discoveryClient = DiscoveryClient.Create();
            discoveryClient.AddListener(listener);
            discoveryClient.Ping();
            
            updateTimer = new DispatcherTimer();
            updateTimer.Tick += UpdateDiscoveredApps;
            updateTimer.Interval = new TimeSpan(0, 0, 0, 0, 1000 / 30); // 30 updates per second
            updateTimer.Start();

            refreshTimer = new DispatcherTimer();
            refreshTimer.Tick += RefreshPing;
            refreshTimer.Interval = new TimeSpan(0, 0, 0, pingRefreshRate, 0);
            refreshTimer.Start();
        }

        protected override void OnClosed(EventArgs e)
        {
            updateTimer.Stop();
            refreshTimer.Stop();

            discoveryClient.RemoveListener(listener);

            discoveryClient.Dispose();
            discoveryClient = null;
        }

        private void OKButton_Clicked(object sender, RoutedEventArgs e)
        {
            this.DialogResult = true;
            this.RemoteAppName = addressBox.Text;

            Array roleValues = Enum.GetValues(typeof(SystemRole));

            this.RemoteAppRole = (SystemRole)roleValues.GetValue(roleComboBox.SelectedIndex);
        }

        private void CancelButton_Clicked(object sender, RoutedEventArgs e)
        {
            this.DialogResult = false;
        }

        private void UpdateDiscoveredApps(object sender, EventArgs e)
        {
            discoveryClient.Update();
        }

        private void RefreshPing(object sender, EventArgs e)
        {
            discoveryClient.Ping();
        }

        private void OnSystemDiscovered(DiscoveredSystem newSystem)
        {
            RemoteSystemEntry newEntry = new RemoteSystemEntry();
            newEntry.RemoteSystem = newSystem;

            remoteClientList.Items.Add(newEntry);
        }

        private void OnSystemLost(DiscoveredSystem lostSystem)
        {
            for(int i = 0; i < remoteClientList.Items.Count; ++i)
            {
                if ((remoteClientList.Items[i] as RemoteSystemEntry).IsEqual(lostSystem))
                {
                    remoteClientList.Items.RemoveAt(i);
                    break;
                }
            }
        }

        private void remoteClientList_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            RemoteSystemEntry selectedItem = remoteClientList.SelectedItem as RemoteSystemEntry;

            addressBox.Text = selectedItem.RemoteSystem.GetName();

            Array systemRoleValues = Enum.GetValues(typeof(SystemRole));
            for (int i = 0; i < systemRoleValues.Length; ++i)
            {
                if((SystemRole)systemRoleValues.GetValue(i) == selectedItem.RemoteSystem.GetRole())
                {
                    roleComboBox.SelectedIndex = i;
                    break;
                }
            }
        }
    }
}
