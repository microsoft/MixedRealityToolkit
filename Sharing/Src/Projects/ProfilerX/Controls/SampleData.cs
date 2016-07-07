// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;

namespace ProfilerX
{
    public class SampleData : INotifyPropertyChanged
    {
        private ObservableCollection<SampleData> samples;
        private HoloToolkit.Sharing.ProfileSample xSample;
        private HoloToolkit.Sharing.ProfileThread xThread;
        private int sampleIndex;
        bool _isExpanded;
        bool _isSelected;

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

        public string Name
        {
            get { return this.xSample.GetName(); }
        }

        public ulong StartTime
        {
            get { return this.xSample.GetStartTime(); }
        }

        public float Duration
        {
            get 
            { 
                ulong durationNanoseconds = this.xSample.GetDuration();
                float durationMilliseconds = (float)((double)durationNanoseconds / 1000000.0); // Convert Nanoseconds to Milliseconds
                return durationMilliseconds;
            } 
        }

        public SampleData(HoloToolkit.Sharing.ProfileSample sample, int index, HoloToolkit.Sharing.ProfileThread thread)
        {
            this.xSample = sample;
            this.xThread = thread;
            this.sampleIndex = index;
        }

        private void InitializeSamples()
        {
            this.samples = new ObservableCollection<SampleData>();
            for (int i = 0; i < this.xThread.GetSampleCount(); ++i)
            {
                HoloToolkit.Sharing.ProfileSample sample = this.xThread.GetSample(i);
                if (sample.GetParentIndex() == this.sampleIndex)
                {
                    this.samples.Add(new SampleData(sample, i, this.xThread));
                }
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged([CallerMemberName] string propertyName = null)
        {
            if (this.PropertyChanged != null)
                this.PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
