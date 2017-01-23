// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;
using System.Windows;

namespace SessionManager.UI
{
    /// <summary>
    /// Interaction logic for CreateSessionDialog.xaml
    /// </summary>
    public partial class CreateSessionDialog : Window
    {
        public string SessionName { get; private set; }

        public CreateSessionDialog(string defaultSessionName)
        {
            this.SessionName = defaultSessionName;

            InitializeComponent();

            this.sessionName.Focus();
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
