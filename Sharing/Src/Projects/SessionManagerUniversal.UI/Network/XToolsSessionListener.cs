// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using HoloToolkit.Sharing;
using System;

namespace SessionManagerUniversal.UI.Network
{
    public class XToolsSessionListener : SessionListener
    {
        public event Action<Session> JoiningSession;
        public event Action<Session> JoinSucceeded;
        public event Action<Session> JoinFailed;
        public event Action<Session> SessionDisconnected;

        private Session session;

        public XToolsSessionListener(Session session)
        {
            this.session = session;
        }

        public override void OnJoiningSession()
        {
            JoiningSession?.Invoke(this.session);
        }
        public override void OnJoinSucceeded()
        {
            JoinSucceeded?.Invoke(this.session);
        }

        public override void OnJoinFailed()
        {
            JoinFailed?.Invoke(this.session);
        }

        public override void OnSessionDisconnected()
        {
            SessionDisconnected?.Invoke(this.session);
        }
    }
}
