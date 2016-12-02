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
    public class ThreadData : INotifyPropertyChanged
    {
        private HoloToolkit.Sharing.ProfileThread thread;
        private ObservableCollection<SampleData> samples;
        private bool _isExpanded = true;
        private bool _isSelected;

        public event PropertyChangedEventHandler PropertyChanged;

        public ObservableCollection<SampleData> Samples
        {
            get 
            {
                if (samples == null)
                {
                    InitializeSamples();
                }

                return samples;
            }
        }

        /// <summary>
        /// Gets/sets whether the TreeViewItem 
        /// associated with this object is expanded.
        /// </summary>
        public bool IsExpanded
        {
            get { return _isExpanded; }
            set
            {
                if (value != _isExpanded)
                {
                    _isExpanded = value;
                    OnPropertyChanged();
                }
            }
        }



        /// <summary>
        /// Gets/sets whether the TreeViewItem 
        /// associated with this object is selected.
        /// </summary>
        public bool IsSelected
        {
            get { return _isSelected; }
            set
            {
                if (value != _isSelected)
                {
                    _isSelected = value;
                    OnPropertyChanged();
                }
            }
        }

        private ulong threadID;
        public ulong ThreadID
        {
            get { return threadID; }

            set
            {
                if (threadID != value)
                {
                    threadID = value;
                    OnPropertyChanged("ThreadID");
                }
            }
        }


        public ThreadData(HoloToolkit.Sharing.ProfileThread newThread)
        {
            this.thread = newThread;
            this.ThreadID = newThread.GetThreadID();
        }


        private void InitializeSamples()
        {
            this.samples = new ObservableCollection<SampleData>();
            for(int i = 0; i < this.thread.GetSampleCount(); ++i)
            {
                HoloToolkit.Sharing.ProfileSample sample = this.thread.GetSample(i);
                if(sample.GetParentIndex() == -1)
                {
                    SampleData newSample = new SampleData(sample, i, this.thread);
                    newSample.IsExpanded = true;
                    this.samples.Add(newSample);
                }
            }
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
