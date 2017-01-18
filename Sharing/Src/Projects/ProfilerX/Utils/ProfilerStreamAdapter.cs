// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace HoloToolkit.Sharing
{
    class ProfilerStreamAdapter : ProfilerStreamListener
    {
        public event Action ConnectedEvent;
        public event Action ConnectFailedEvent;
        public event Action DisconnectedEvent;
        public event Action<ProfileFrame> ReceiveFrameEvent;

        public override void OnConnected() 
        {
            if (this.ConnectedEvent != null)
            {
                this.ConnectedEvent();
            }
        }

	    public override void OnConnectFailed()
        {
            if (this.ConnectFailedEvent != null)
            {
                this.ConnectFailedEvent();
            }
        }

	    public override void OnDisconnected() 
        {
            if (this.DisconnectedEvent != null)
            {
                this.DisconnectedEvent();
            }
        }

	    public override void OnReceiveProfileFrame(ProfileFrame newFrame) 
        {
            if (this.ReceiveFrameEvent != null)
            {
                this.ReceiveFrameEvent(newFrame);
            }
        }
    }
}
