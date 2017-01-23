// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using HoloToolkit.Sharing;
using System;

namespace SessionManagerUniversal.UI.Network
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

        private readonly LogWriter logWriter;


        public XToolsSessionManagerListener(LogWriter log = null)
        {
            this.logWriter = log;
        }

        public override void OnCreateSucceeded(Session newSession)
        {
            LogWriteLine("********* SESSION " + newSession.GetName().GetString() + " CREATED ******");

            CreateSucceeded?.Invoke(newSession);
        }

        public override void OnCreateFailed(XString reason)
        {
            LogWriteLine("********* SESSION CREATE FAILED: " + reason.GetString() + " ******");
            CreateFailed?.Invoke(reason);
        }

        public override void OnSessionAdded(Session session)
        {
            LogWriteLine("********* SESSION " + session.GetName().GetString() + " ADDED ******");

            SessionAdded?.Invoke(session);
        }

        public override void OnSessionClosed(Session session)
        {
            LogWriteLine("********* SESSION " + session.GetName().GetString() + " CLOSED ******");

            SessionClosed?.Invoke(session);
        }

        public override void OnUserJoinedSession(Session session, User user)
        {
            LogWriteLine(string.Format("********* USER {0} ({1}) JOINED SESSION  {2} ******", user.GetName().GetString(), user.GetID(), session.GetName().GetString()));

            SessionUserJoined?.Invoke(session, user);
        }

        public override void OnUserLeftSession(Session session, User user)
        {
            LogWriteLine(string.Format("********* USER {0} ({1}) LEFT SESSION  {2} ******", user.GetName().GetString(), user.GetID(), session.GetName().GetString()));

            SessionUserLeft?.Invoke(session, user);
        }

        public override void OnUserChanged(Session session, User user)
        {
            SessionUserChanged?.Invoke(session, user);
        }

        public override void OnServerConnected()
        {
            SessionServerConnected?.Invoke();
        }

        public override void OnServerDisconnected()
        {
            SessionServerDisconnected?.Invoke();
        }

        private void LogWriteLine(string message)
        {
            if (logWriter != null)
            {
                logWriter.WriteLogEntry(LogSeverity.Info, message);
            }
            else
            {
                if (System.Diagnostics.Debugger.IsAttached)
                {
                    System.Diagnostics.Debug.WriteLine(message);
                }
            }
        }
    }
}
