// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace ProfilerX
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private ObservableCollection<StreamData> streams;

        public ObservableCollection<StreamData> Streams
        {
            get { return this.streams; }
        }

        public MainWindow()
        {
            this.streams = new ObservableCollection<StreamData>();
            InitializeComponent();
        }

        private void AddButton_Clicked(object sender, RoutedEventArgs e)
        {
            AddAppDialog dialog = new AddAppDialog();
            dialog.Owner = this;

            bool? result = dialog.ShowDialog();

            if (result.HasValue && result.Value && dialog.RemoteAppName != null && dialog.RemoteAppName.Length > 0)
            {
                HoloToolkit.Sharing.ProfilerStream xStream = App.ProfilerApp.Manager.CreateStream(dialog.RemoteAppName, dialog.RemoteAppRole);

                StreamData streamData = new StreamData(xStream, dialog.RemoteAppName);
                this.streams.Add(streamData);
            }
        }

        private void RemoveButton_Clicked(object sender, RoutedEventArgs e)
        {
            StreamData data = Streams[this.AppList.SelectedIndex];
            App.ProfilerApp.Manager.CloseStream(data.stream);
            data.stream.Dispose();

            this.streams.RemoveAt(this.AppList.SelectedIndex);
        }

        private void pauseButton_Click(object sender, RoutedEventArgs e)
        {
            App.ProfilerApp.Recording = !App.ProfilerApp.Recording;
            if(App.ProfilerApp.Recording)
            {
                pauseButton.Content = "Pause";
            }
            else
            {
                pauseButton.Content = "Record";
            }
        }
    }
}
