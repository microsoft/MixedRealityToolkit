// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.


using System;
using Windows.UI;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Shapes;

namespace MicDemoApp
{
    public sealed partial class MainPage : Page
    {
        public static MainPage Instance;
        Color ActiveMicColor = Colors.Red;
        float micGain = 1f; // default is 1, and default is probably too quiet. my microphone needs about 50, for instance.

        public MainPage()
        {
            this.InitializeComponent();
            Instance = this;

            micGain = (float)((Slider)this.FindName("slider")).Value;   // these lines automatically sets the mic volume based off of the XAML slider element's starting point
        }

        private void comboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            var comboBox = sender as ComboBox;
            string value = comboBox.SelectedItem as string;
            switch (value)
            {
                case "Speech":
                    MicStreamSelector.streamType = MicStreamSelector.StreamCategory.SPEECH;
                    break;
                case "Communications":
                    MicStreamSelector.streamType = MicStreamSelector.StreamCategory.COMMUNICATIONS;
                    break;
                case "Media":
                    MicStreamSelector.streamType = MicStreamSelector.StreamCategory.MEDIA;
                    break;
            }
        }

        private void slider_ValueChanged(object sender, RangeBaseValueChangedEventArgs e)
        {
            micGain = (float)e.NewValue;
            MicStreamSelector.MicSetGain(micGain); // changes volume immediately if already running
        }

        private void StartStream_Click(object sender, RoutedEventArgs e)
        {
            MicStreamSelector.MicSetGain(micGain);
            MicStreamSelector.StartStream();
            ActiveMicColor = Colors.Green;
        }

        private void StartRecording_Click(object sender, RoutedEventArgs e)
        {
            MicStreamSelector.MicSetGain(micGain);
            MicStreamSelector.StartRecording("myfilenamewithextension.wav");
            ActiveMicColor = Colors.Green;
        }

        private void StopRecording_Click(object sender, RoutedEventArgs e)
        {
            MicStreamSelector.StopMicDevice();
            ActiveMicColor = Colors.Red;
            SetVolumeMonitor(0f); // forces one more color update to show RED mic monitor
        }

        private void satya_PointerEntered(object sender, PointerRoutedEventArgs e)
        {
            Image i = sender as Image;
            i.Opacity = 0;
        }

        private void satya_PointerExited(object sender, PointerRoutedEventArgs e)
        {
            Image i = sender as Image;
            i.Opacity = 1;
        }

        public void SetVolumeMonitor(float f)
        {
            this.Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.High, () =>
            {
                Ellipse e = (Ellipse)this.FindName("Volume");
                e.Opacity = f + .05f;
                e.Fill = new SolidColorBrush(ActiveMicColor);
            }).AsTask();
        }
    }
}
