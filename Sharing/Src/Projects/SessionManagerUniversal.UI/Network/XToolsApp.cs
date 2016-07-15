// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;
using System.Collections.Generic;
using HoloToolkit.Sharing;
using SessionManagerUniversal.UI.Helpers;
using System.Threading;

namespace SessionManagerUniversal.UI.Network
{
    public class XToolsApp : IDisposable
    {
        #region Fields

        private bool disposed;
        private ObjectElement rootObject;
        private ConsoleSyncReporter syncListener;

        private NetworkConnection viewerConnection;
        private NetworkConnection serverConnection;
        public ConsoleLogWriter LogWriter;

        private Timer networkMessageLoop;

        #endregion

        #region Events

        public event Action<NetworkConnection> ViewerConnected;
        public event Action<NetworkConnection> ViewerConnectionFailed;
        public event Action<NetworkConnection> ViewerDisconnected;
        public event Action<NetworkConnection> SessionConnected;
        public event Action<NetworkConnection> SessionConnectionFailed;
        public event Action<NetworkConnection> SessionDisconnected;

        #endregion

        public XToolsApp()
        {
            this.LogWriter = new ConsoleLogWriter();
        }

        public void Connect(string server, string userName = "TestUserUniversal", int port = 20602, ClientRole clientRole = ClientRole.Primary)
        {
            ClientConfig config = new ClientConfig(clientRole);
            config.SetServerAddress(server);
            config.SetServerPort(port);
            config.SetLogWriter(LogWriter);

            this.SharingManager = SharingManager.Create(config);
            this.SharingManager.SetUserName(userName);
            
            this.viewerConnection = this.SharingManager.GetPairedConnection();
            this.serverConnection = this.SharingManager.GetServerConnection();
            this.SessionManager = this.SharingManager.GetSessionManager();
            
            BeginPairing();

            ViewerListener = new NetworkConnectionAdapter();
            ViewerListener.ConnectedCallback += this.OnViewerConnected;
            ViewerListener.ConnectionFailedCallback += this.OnViewerConnectionFailed;
            ViewerListener.DisconnectedCallback += this.OnViewerDisconnected;
            viewerConnection.AddListener((byte)MessageID.StatusOnly, ViewerListener);

            ServerListener = new NetworkConnectionAdapter();
            ServerListener.ConnectedCallback += this.OnSessionConnected;
            ServerListener.ConnectionFailedCallback += this.OnSessionConnectionFailed;
            ServerListener.DisconnectedCallback += this.OnSessionDisconnected;
            serverConnection.AddListener((byte)MessageID.StatusOnly, ServerListener);

            this.syncListener = new ConsoleSyncReporter();
            this.rootObject = this.SharingManager.GetRootSyncObject();
            this.rootObject.AddListener(this.syncListener);

            SessionManagerListener = new XToolsSessionManagerListener(this.LogWriter);
            this.SessionManager.AddListener(SessionManagerListener);
            networkMessageLoop = new Timer(new TimerCallback((a) => Update()), null, 0, 1000);
        }

        public void Disconnect()
        {
            CleanupNetworkObjects();

            IsServerConnected = false;
            // Forces a garbage collection to try to clean up any additional reference to SWIG-wrapped objects
            System.GC.Collect();
        }

        // Pumps the network message loop
        public void Update()
        {
            SharingManager?.Update();
        }

        #region Session Management

        // Helper to join a session
        public bool JoinSession(string sessionName)
        {
            XString nameXString = new XString(sessionName);

            bool foundSession = false;

            for (int i = 0; i < this.SessionManager.GetSessionCount(); ++i)
            {
                Session session = this.SessionManager.GetSession(i);
                if (session.GetName().IsEqual(nameXString))
                {
                    LogWriteLine("Joining Session " + sessionName);

                    if (session.Join())
                    {
                        LogWriteLine("Join request sent");
                        foundSession = true;
                    }
                    else
                    {
                        LogWriteLine("Failed to join session!");
                    }
                    break;
                }
            }

            return foundSession;
        }

        // Helper to create a session
        public bool CreateSession(string sessionName)
        {
            XString nameXString = new XString(sessionName);

            LogWriteLine("Creating Session " + sessionName);

            if (!this.SessionManager.CreateSession(nameXString))
            {
                LogWriteLine("Failed to request a new session!");
                return false;
            }

            return true;
        }

        public void LeaveSession()
        {
            Session currentSession = this.SessionManager.GetCurrentSession();
            if (currentSession != null)
            {
                currentSession.Leave();
                LogWriteLine("Leaving current session...");
            }
        }

        #endregion

        #region Event Handling

        private void OnSessionConnected(NetworkConnection connection)
        {
            this.IsServerConnected = true;
            LogWriteLine("Server connected.");

            SessionConnected?.Invoke(connection);
        }

        private void OnSessionConnectionFailed(NetworkConnection connection)
        {
            this.IsServerConnected = false;
            LogWriteLine("Server connection failed.");

            SessionConnectionFailed?.Invoke(connection);
        }

        private void OnSessionDisconnected(NetworkConnection connection)
        {
            this.IsServerConnected = false;
            LogWriteLine("Server connection disconnected.");

            SessionDisconnected?.Invoke(connection);
        }

        private void OnViewerConnected(NetworkConnection connection)
        {
            this.IsViewerConnected = true;
            LogWriteLine("App connected.");

            ViewerConnected?.Invoke(connection);
        }

        private void OnViewerConnectionFailed(NetworkConnection connection)
        {
            this.IsViewerConnected = false;
            LogWriteLine("App connection failed.");

            ViewerConnectionFailed?.Invoke(connection);
        }

        private void OnViewerDisconnected(NetworkConnection connection)
        {
            this.IsViewerConnected = false;
            LogWriteLine("App connection disconnected.");

            ViewerDisconnected?.Invoke(connection);

            BeginPairing();
        }

        #endregion

        private void LogWriteLine(string message)
        {
            if (LogWriter != null)
            {
                LogWriter.WriteLogEntry(LogSeverity.Info, message);
            }
            else
            {
                if (System.Diagnostics.Debugger.IsAttached)
                {
                    System.Diagnostics.Debug.WriteLine(message);
                }
            }
        }

        private bool BeginPairing()
        {
            this.PairingListener = new DirectPairingListener(this);

            return this.SharingManager.GetPairingManager().BeginPairing(new DirectPairReceiver(), PairingListener) == PairingResult.Ok;
        }

        private void CleanupNetworkObjects()
        {
            if (networkMessageLoop != null)
            {
                networkMessageLoop.Dispose();
                networkMessageLoop = null;
            }

            if (this.rootObject != null)
            {
                this.rootObject.RemoveListener(this.syncListener);
                this.rootObject.Dispose();
                this.rootObject = null;
            }

            if (this.syncListener != null)
            {
                this.syncListener.Dispose();
                this.syncListener = null;
            }

            if (viewerConnection != null)
            {
                viewerConnection.RemoveListener((byte)MessageID.StatusOnly, ViewerListener);
                viewerConnection.Disconnect();
                this.viewerConnection.Dispose();
                this.viewerConnection = null;
            }

            if (serverConnection != null)
            {
                serverConnection.RemoveListener((byte)MessageID.StatusOnly, ServerListener);
                serverConnection.Disconnect();
                this.serverConnection.Dispose();
                this.serverConnection = null;
            }

            if (this.SessionManager != null)
            {
                this.SessionManager.RemoveListener(this.SessionManagerListener);
                this.SessionManager.Dispose();
                this.SessionManager = null;
            }

            if (this.SessionManagerListener != null)
            {
                this.SessionManagerListener.Dispose();
                this.SessionManagerListener = null;
            }

            if (SharingManager != null)
            {
                SharingManager.Dispose();
                SharingManager = null;
            }
        }

        #region Properties

        public SessionManager SessionManager { get; private set; }

        public SharingManager SharingManager { get; private set; }

        public XToolsSessionManagerListener SessionManagerListener { get; private set; }

        public bool IsViewerConnected { get; private set; }

        public bool IsServerConnected { get; private set; }

        public NetworkConnectionAdapter ViewerListener { get; private set; }
        public NetworkConnectionAdapter ServerListener { get; private set; }
        public PairingListener PairingListener { get; private set; }

        #endregion

        #region Dispose

        /// <summary>
        /// Performs application-defined tasks associated with freeing, releasing, or resetting unmanaged resources.
        /// </summary>
        public void Dispose()
        {
            this.Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Releases unmanaged and - optionally - managed resources.
        /// </summary>
        /// <param name="disposing"><c>True</c> to release both managed and unmanaged resources; <c>False</c> to release only unmanaged resources.</param>
        protected virtual void Dispose(bool disposing)
        {
            if (!this.disposed)
            {
                if (disposing)
                {
                    CleanupNetworkObjects();
                }

                this.disposed = true;
            }
        }

        #endregion


        private class DirectPairingListener : PairingListener
        {
            private readonly XToolsApp app;

            public DirectPairingListener(XToolsApp app)
            {
                this.app = app;
            }

            public override void PairingConnectionSucceeded()
            {
                app.LogWriteLine("Pairing succeeded");
                app.PairingListener = null;
            }

            public override void PairingConnectionFailed(PairingResult reason)
            {
                app.LogWriteLine("Pairing connection failed");
                app.BeginPairing();
            }
        }

        private class ConsoleSyncReporter : ObjectElementListener
        {

        }
    }
}
