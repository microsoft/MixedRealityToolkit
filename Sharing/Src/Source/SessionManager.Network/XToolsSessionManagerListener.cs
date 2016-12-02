// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;
using HoloToolkit.Sharing;

namespace SessionManager.Network
{
    public class XToolsSessionManagerListener : SessionManagerListener
    {
        public event Action<Session> CreateSucceeded;
        public event Action<XString> CreateFailed;
        public event Action<Session> SessionAdded;
        public event Action<Session> SessionClosed;
        public event Action<Session, User> SessionUserJoined;
        public event Action<Session, User> SessionUserLeft;
        public event Action<Session, User> SessionUserChanged;
        public event Action SessionServerConnected;
        public event Action SessionServerDisconnected;

        public override void OnCreateSucceeded(Session newSession)
        {
            LogWriteLine("********* SESSION " + newSession.GetName().GetString() + " CREATED ******");

            if (CreateSucceeded != null)
            {
                CreateSucceeded(newSession);
            }
        }

        public override void OnCreateFailed(XString reason)
        {
            LogWriteLine("********* SESSION CREATE FAILED: " + reason.GetString() + " ******");

            if (CreateFailed != null)
            {
                CreateFailed(reason);
            }
        }

        public override void OnSessionAdded(Session session)
        {
            Console.WriteLine("********* SESSION " + session.GetName().GetString() + " ADDED ******");

            if (SessionAdded != null)
            {
                SessionAdded(session);
            }
        }

        public override void OnSessionClosed(Session session)
        {
            LogWriteLine("********* SESSION " + session.GetName().GetString() + " CLOSED ******");

            if (SessionClosed != null)
            {
                SessionClosed(session);
            }
        }

        public override void OnUserJoinedSession(Session session, User user)
        {
            LogWriteLine(string.Format("********* USER {0} ({1}) JOINED SESSION  {2} ******", user.GetName().GetString(), user.GetID(), session.GetName().GetString()));

            if (SessionUserJoined != null)
            {
                SessionUserJoined(session, user);
            }
        }

        public override void OnUserLeftSession(Session session, User user)
        {
            LogWriteLine(string.Format("********* USER {0} ({1}) LEFT SESSION  {2} ******", user.GetName().GetString(), user.GetID(), session.GetName().GetString()));

            if (SessionUserLeft != null)
            {
                SessionUserLeft(session, user);
            }
        }

        public override void OnUserChanged(Session session, User user)
        {
            if (SessionUserChanged != null)
            {
                SessionUserChanged(session, user);
            }
        }

        public override void OnServerConnected()
        {
            if (SessionServerConnected != null)
            {
                SessionServerConnected();
            }
        }

        public override void OnServerDisconnected()
        {
            if (SessionServerDisconnected != null)
            {
                SessionServerDisconnected();
            }
        }

        private void LogWriteLine(string message)
        {
            System.Diagnostics.Debug.WriteLine(message);
            Console.WriteLine(message);
        }
    }
}
