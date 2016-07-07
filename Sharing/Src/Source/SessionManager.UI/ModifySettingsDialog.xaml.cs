// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;
using System.Windows;

namespace SessionManager.UI
{
    /// <summary>
    /// Interaction logic for ModifySettingsDialog.xaml
    /// </summary>
    public partial class ModifySettingsDialog : Window
    {
        public string UserName { get; private set; }
        public string ServerAddress { get; private set; }
        public string ViewerAddress { get; private set; }

        public ModifySettingsDialog(string defaultUserName, string defaultServerAddress, string defaultViewerAddress)
        {
            this.UserName = defaultUserName;
            this.ServerAddress = defaultServerAddress;
            this.ViewerAddress = defaultViewerAddress;

            InitializeComponent();

            this.userName.Focus();
        }

        private void OKButton_Clicked(object sender, RoutedEventArgs e)
        {
            this.DialogResult = true;
        }

        private void CancelButton_Clicked(object sender, RoutedEventArgs e)
        {
            this.DialogResult = false;
        }
    }
}
