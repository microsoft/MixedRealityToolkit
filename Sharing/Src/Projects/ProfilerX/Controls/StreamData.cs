// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace ProfilerX
{
    public class StreamData : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        private ObservableCollection<FrameData> frames;
        public ObservableCollection<FrameData> Frames
        {
            get { return this.frames; }
        }

        private ObservableCollection<LogData> logs;
        public ObservableCollection<LogData> Logs
        {
            get { return this.logs; }
        }

        public HoloToolkit.Sharing.ProfilerStream stream;
        private HoloToolkit.Sharing.ProfilerStreamAdapter adapter;
        private string remoteSystemName;
        private readonly int MaxFrames = 600; // 10 seconds @ 60fps

        public string RemoteSystemName { get { return this.remoteSystemName; } }
        

        public bool IsConnected 
        { 
            get { return stream.IsConnected(); } 
            set
            {
                OnPropertyChanged();
            }
        }

        public StreamData(HoloToolkit.Sharing.ProfilerStream newStream, string remoteName)
        {
            this.remoteSystemName = remoteName;

            this.frames = new ObservableCollection<FrameData>();
            this.logs = new ObservableCollection<LogData>();

            this.adapter = new HoloToolkit.Sharing.ProfilerStreamAdapter();
            this.adapter.ConnectedEvent += OnConnected;
            this.adapter.ConnectFailedEvent += OnConnectFailed;
            this.adapter.DisconnectedEvent += OnDisconnected;
            this.adapter.ReceiveFrameEvent += OnReceiveProfileFrame;

            this.stream = newStream;
            this.stream.AddListener(this.adapter);
            this.stream.Connect();
        }

        private void OnConnected()
        {
            OnPropertyChanged("IsConnected");
        }

        private void OnConnectFailed()
        {
            // Try connecting again
            this.stream.Connect();
        }

        private void OnDisconnected()
        {
            OnPropertyChanged("IsConnected");

            // Try to reconnect
            this.stream.Connect();
        }

        private void OnReceiveProfileFrame(HoloToolkit.Sharing.ProfileFrame newFrame)
        {
            if (App.ProfilerApp.Recording)
            {
                this.frames.Add(new FrameData(newFrame));

                while (this.frames.Count > MaxFrames)
                {
                    this.frames.RemoveAt(0);
                }

                for(int i = 0; i < newFrame.GetLogMessageCount(); ++i)
                {
                    this.logs.Add(new LogData(newFrame.GetLogMessage(i)));
                }
            }
        }

        // Create the OnPropertyChanged method to raise the event 
        protected void OnPropertyChanged([CallerMemberName] string name = null)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(name));
            }
        }
    }
}
