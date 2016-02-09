using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ProfilerX
{
    public class FrameData
    {
        private XTools.ProfileFrame profileFrame;

        private ObservableCollection<ThreadData> threads = new ObservableCollection<ThreadData>();
        public ObservableCollection<ThreadData> Threads
        {
            get { return this.threads; }
        }


        public float MaxDuration
        {
            get 
            {
                float maxDuration = 0f;
                int threadCount = this.profileFrame.GetThreadCount();
                for (int i = 0; i < threadCount; ++i)
                {
                    XTools.ProfileThread thread = this.profileFrame.GetThread(i);
                    if(thread.GetSampleCount() > 0)
                    {
                        float threadDuration = (float)thread.GetSample(0).GetDuration() / 1000000f;
                        maxDuration = (threadDuration > maxDuration) ? threadDuration : maxDuration;
                    }
                }

                return maxDuration;
            }
        }

        public FrameData(XTools.ProfileFrame xFrame)
        {
            this.profileFrame = xFrame;

            for(int i = 0; i < this.profileFrame.GetThreadCount(); ++i)
            {
                this.threads.Add(new ThreadData(this.profileFrame.GetThread(i)));
            }
        }
    }
}
