// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Threading.Tasks;
using System.Linq;

using SessionManager.UI;
using SessionManager.Network;

namespace SessionManager.DataModel
{
    public class User : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;
        private string userName;
        private int id;
        private bool muteState;

        public User(HoloToolkit.Sharing.User XUser, string userName, int id, bool muteState)
        {
            this.XUser = XUser;
            this.userName = userName;
            this.id = id;
            this.muteState = muteState;
        }

        // User name
        public string Name
        {
            get { return userName; }

            set
            {
                if (!userName.Equals(value))
                {
                    userName = value;
                    OnPropertyChanged("Name");
                }
            }
        }

        // Unique id for user
        public int Id
        {
            get { return id; }

            set
            {
                if (id != value)
                {
                    id = value;
                    OnPropertyChanged("Id");
                }
            }
        }

        // Mute state
        public bool MuteState
        {
            get { return muteState; }

            set
            {
                if (muteState != value)
                {
                    muteState = value;
                    OnPropertyChanged("MuteState");
                }
            }
        }

        // XTool user reference
        public HoloToolkit.Sharing.User XUser { get; private set; }

        // Create the OnPropertyChanged method to raise the event 
        protected void OnPropertyChanged(string name)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(name));
            }
        }
    }

    // Session data tracks useful information for each session that is known about.  It doesn't track users.
    public class SessionData : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;
        public XToolsSessionListener SessionListener;
        private ObservableCollection<User> sessionUsers;

        public SessionData(HoloToolkit.Sharing.Session session)
        {
            this.Session = session;
            this.sessionUsers = new ObservableCollection<User>();

            if (this.Session != null)
            {
                App.XToolsApp.SessionDisconnected += XToolsApp_ServerDisconnected;

                App.XToolsApp.SessionManagerListener.SessionUserJoined += SessionManagerListener_SessionUserJoined;
                App.XToolsApp.SessionManagerListener.SessionUserLeft += SessionManagerListener_SessionUserLeft;
                App.XToolsApp.SessionManagerListener.SessionUserChanged += SessionManagerListener_SessionUserChanged;

                this.SessionListener = new XToolsSessionListener(this.Session);
                this.Session.AddListener(this.SessionListener);

                this.SessionListener.JoinSucceeded += SessionListener_SessionChanged;
                this.SessionListener.SessionDisconnected += SessionListener_SessionChanged;
                this.SessionListener.JoinFailed += SessionListener_SessionChanged;
                this.SessionListener.JoiningSession += SessionListener_SessionChanged;

                BuildUserList();
            }
        }

        ~SessionData()
        {
            if (App.XToolsApp != null && this.Session != null)
            {
                App.XToolsApp.SessionDisconnected -= XToolsApp_ServerDisconnected;
                App.XToolsApp.SessionManagerListener.SessionUserJoined -= SessionManagerListener_SessionUserJoined;
                App.XToolsApp.SessionManagerListener.SessionUserLeft -= SessionManagerListener_SessionUserLeft;
                App.XToolsApp.SessionManagerListener.SessionUserChanged -= SessionManagerListener_SessionUserChanged;

                this.SessionListener.JoinSucceeded -= SessionListener_SessionChanged;
                this.SessionListener.SessionDisconnected -= SessionListener_SessionChanged;
                this.SessionListener.JoinFailed -= SessionListener_SessionChanged;
                this.SessionListener.JoiningSession -= SessionListener_SessionChanged;
            }
        }

        public bool IsValid
        {
            get { return this.Session != null; }
        }

        public string Name
        {
            get { return this.IsValid ? this.Session.GetName().GetString() : string.Empty; }
        }

        public bool Joinable
        {
            get { return this.IsValid ? this.Session.GetMachineSessionState() == HoloToolkit.Sharing.MachineSessionState.DISCONNECTED : false; }
        }

        public int UserCount
        {
            get { return this.IsValid ? this.Session.GetUserCount() : 0; }
        }

        public string SessionType
        {
            get { return this.IsValid ? System.Threading.Thread.CurrentThread.CurrentCulture.TextInfo.ToTitleCase(this.Session.GetSessionType().ToString().ToLower()) : "Invalid"; }
        }

        public string Status
        {
            get { return this.IsValid ? System.Threading.Thread.CurrentThread.CurrentCulture.TextInfo.ToTitleCase(this.Session.GetMachineSessionState().ToString().ToLower()) : "Invalid"; }
        }

        public HoloToolkit.Sharing.Session Session { get; private set; }

        public ObservableCollection<User> Users
        {
            get { return this.sessionUsers; }
        }

        private void XToolsApp_ServerDisconnected(HoloToolkit.Sharing.NetworkConnection connection)
        {
            if (this.IsValid)
            {
                OnPropertyChanged("IsValid");
                ClearUsers();
            }
        }

        private void SessionManagerListener_SessionUserJoined(HoloToolkit.Sharing.Session session, HoloToolkit.Sharing.User XUser)
        {
            if (this.IsValid && this.Session.GetName().IsEqual(session.GetName()))
            {
                OnPropertyChanged("UserCount");
                AddUser(XUser);
            }
        }

        private void SessionManagerListener_SessionUserLeft(HoloToolkit.Sharing.Session session, HoloToolkit.Sharing.User XUser)
        {
            if (this.IsValid && this.Session.GetName().IsEqual(session.GetName()))
            {
                OnPropertyChanged("UserCount");
                RemoveUser(XUser);
            }
        }

        private void SessionManagerListener_SessionUserChanged(HoloToolkit.Sharing.Session session, HoloToolkit.Sharing.User XUser)
        {
            if (this.IsValid && this.Session.GetName().IsEqual(session.GetName()))
            {
                UpdateUser(XUser);
            }
        }

        private void SessionListener_SessionChanged(HoloToolkit.Sharing.Session session)
        {
            OnPropertyChanged("Joinable");
            OnPropertyChanged("Status");
            OnPropertyChanged("IsValid");
        }

        private void BuildUserList()
        {
            ClearUsers();

            // Make sure any existing users are added to the session
            for (int i = 0; i < this.Session.GetUserCount(); i++)
            {
                HoloToolkit.Sharing.User XUser = this.Session.GetUser(i);
                AddUser(XUser);
            }
        }

        private void ClearUsers()
        {
            if (this.sessionUsers.Count > 0)
            {
                App.Current.Dispatcher.BeginInvoke(new Action<SessionData>((sender) =>
                {
                    this.sessionUsers.Clear();
                }), new object[] { this });
            }
        }

        private void AddUser(HoloToolkit.Sharing.User XUser)
        {
            string userName = XUser.GetName().GetString();
            int id = XUser.GetID();
            bool muteState = XUser.GetMuteState();

            App.Current.Dispatcher.BeginInvoke(new Action<SessionData>((sender) =>
            {
                User existingUser = this.sessionUsers.FirstOrDefault(x => x.Id == id);
                if (existingUser == null)
                {
                    this.sessionUsers.Add(new User(XUser, userName, id, muteState));
                }
            }), new object[] { this });
        }

        private void RemoveUser(HoloToolkit.Sharing.User XUser)
        {
            int id = XUser.GetID();

            App.Current.Dispatcher.BeginInvoke(new Action<SessionData>((sender) =>
            {
                User existingUser = this.sessionUsers.FirstOrDefault(x => x.Id == id);
                if (existingUser != null)
                {
                    this.sessionUsers.Remove(existingUser);
                }
            }), new object[] { this });
        }

        private void UpdateUser(HoloToolkit.Sharing.User XUser)
        {
            string userName = XUser.GetName().GetString();
            int id = XUser.GetID();
            bool muteState = XUser.GetMuteState();

            App.Current.Dispatcher.BeginInvoke(new Action<SessionData>((sender) =>
            {
                User existingUser = this.sessionUsers.FirstOrDefault(x => x.Id == id);
                if (existingUser != null)
                {
                    existingUser.Name = userName;
                    existingUser.Id = id;
                    existingUser.MuteState = muteState;
                }
            }), new object[] { this });
        }

        // Create the OnPropertyChanged method to raise the event 
        protected void OnPropertyChanged(string name)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(name));
            }
        }
    }


    // Contains a list of all active sessions the server knows about.
    public class SessionsList : ObservableCollection<SessionData>
    {
        public SessionsList()
        {
            App.XToolsApp.SessionManagerListener.SessionServerDisconnected += XToolsApp_ServerDisconnected;
            App.XToolsApp.SessionManagerListener.SessionAdded += SessionListener_SessionManagerAdded;
            App.XToolsApp.SessionManagerListener.SessionClosed += SessionManagerListener_SessionClosed;

            // Make sure all sessions are added
            for (int i = 0; i < App.XToolsApp.SessionManager.GetSessionCount(); i++)
            {
                Add(new SessionData(App.XToolsApp.SessionManager.GetSession(i)));
            }
        }

        ~SessionsList()
        {
            if (App.XToolsApp != null)
            {
                App.XToolsApp.SessionManagerListener.SessionServerDisconnected -= XToolsApp_ServerDisconnected;
                App.XToolsApp.SessionManagerListener.SessionAdded -= SessionListener_SessionManagerAdded;
                App.XToolsApp.SessionManagerListener.SessionClosed -= SessionManagerListener_SessionClosed;
            }
        }

        public SessionData FindSession(HoloToolkit.Sharing.Session session)
        {
            SessionData existingSessionData = session != null ? this.FirstOrDefault(x => x.Session.GetName().IsEqual(session.GetName())) : null;
            return existingSessionData;
        }

        public SessionData FindSession(string sessionName)
        {
            SessionData existingSessionData = this.FirstOrDefault(x => x.Session.GetName().GetString().Equals(sessionName));
            return existingSessionData;
        }

        private void XToolsApp_ServerDisconnected()
        {
            App.Current.Dispatcher.BeginInvoke(new Action<SessionsList>((sender) =>
            {
                this.Clear();
            }), new object[] { this });
        }

        private void SessionListener_SessionManagerAdded(HoloToolkit.Sharing.Session session)
        {
            App.Current.Dispatcher.BeginInvoke(new Action<SessionsList>((sender) =>
            {
                // Register the new session
                SessionData existingSessionData = FindSession(session);
                if (existingSessionData == null)
                {
                    Add(new SessionData(session));
                }
            }), new object[] { this }).Wait();
        }

        private void SessionManagerListener_SessionClosed(HoloToolkit.Sharing.Session session)
        {
            App.Current.Dispatcher.BeginInvoke(new Action<SessionsList>((sender) =>
            {
                SessionData existingSessionData = FindSession(session);
                if (existingSessionData != null)
                {
                    Remove(existingSessionData);
                }
            }), new object[] { this }).Wait();
        }
    }


    // Handles data associated with network connection status
    public class NetworkConnectionData : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        public NetworkConnectionData()
        {
            App.XToolsApp.SessionManagerListener.SessionServerConnected += XToolsApp_ServerConnectionChanged;
            App.XToolsApp.SessionManagerListener.SessionServerDisconnected += XToolsApp_ServerConnectionChanged;
            App.XToolsApp.ViewerConnected += XToolsApp_ViewerConnectionChanged;
            App.XToolsApp.ViewerDisconnected += XToolsApp_ViewerConnectionChanged;
        }

        public bool IsServerConnected
        {
            get { return App.XToolsApp.SessionManager.IsServerConnected(); }
        }
        public bool IsViewerConnected
        {
            get { return App.XToolsApp.IsViewerConnected; }
        }

        public string UserName
        {
            get { return App.XToolsApp.Manager.GetSessionManager().GetCurrentUser().GetName().GetString(); }

            set
            {
                if (!UserName.Equals(value))
                {
                    App.XToolsApp.Manager.SetUserName(new HoloToolkit.Sharing.XString(value));
                    OnPropertyChanged("UserName");
                }
            }
        }

        public bool UserMuteState
        {
            get { return App.XToolsApp.Manager.GetUserPresenceManager().GetMuteState(); }

            set
            {
                if (value != UserMuteState)
                {
                    App.XToolsApp.Manager.GetUserPresenceManager().SetMuteState(value);
                    OnPropertyChanged("UserMuteState");
                }
            }
        }

        public string ServerAddress
        {
            get { return string.Format("{0}:{1}", App.XToolsApp.Manager.GetSettings().GetServerAddress().GetString(), App.XToolsApp.Manager.GetSettings().GetServerPort()); }

            set
            {
                string hostname = App.XToolsApp.Manager.GetSettings().GetServerAddress().GetString();
                uint port = (uint)App.XToolsApp.Manager.GetSettings().GetServerPort();
                GetNetworkAddressParts(value, ref hostname, ref port);

                App.XToolsApp.Manager.SetServerConnectionInfo(new HoloToolkit.Sharing.XString(hostname), port);
                OnPropertyChanged("ServerAddress");
            }
        }

        public string ViewerAddress
        {
            get { return string.Format("{0}:{1}", App.XToolsApp.Manager.GetSettings().GetViewerAddress().GetString(), App.XToolsApp.Manager.GetSettings().GetViewerPort()); }

            set
            {
                string hostname = App.XToolsApp.Manager.GetSettings().GetViewerAddress().GetString();
                uint port = (uint)App.XToolsApp.Manager.GetSettings().GetViewerPort();
                GetNetworkAddressParts(value, ref hostname, ref port);

                // TODO: add support for the new pairing model
                //App.XToolsApp.Manager.SetPairedConnectionInfo(new XTools.XString(hostname), port);
                OnPropertyChanged("ViewerAddress");
            }
        }

        private void XToolsApp_ServerConnectionChanged()
        {
            OnPropertyChanged("IsServerConnected");
        }

        private void XToolsApp_ViewerConnectionChanged(HoloToolkit.Sharing.NetworkConnection obj)
        {
            OnPropertyChanged("IsViewerConnected");
        }

        // Parses the host name and port from a network address
        private void GetNetworkAddressParts(string address, ref string hostname, ref uint port)
        {
            string[] addressValues = address.Split(new char[] { ':' });

            if (addressValues.Length > 0)
            {
                hostname = addressValues[0];
            }

            if (addressValues.Length > 1)
            {
                uint.TryParse(addressValues[1], out port);
            }
        }

        // Create the OnPropertyChanged method to raise the event 
        protected void OnPropertyChanged(string name)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(name));
            }
        }
    }
}
