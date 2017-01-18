// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;
using System.Collections.Generic;
using HoloToolkit.Sharing;

namespace SessionManager.Network
{
    public class XToolsApp : IDisposable
    {
        private bool disposed;
        private NetworkConnection viewerConnection;
        private NetworkConnection serverConnection;
        private ObjectElement rootObject;
        private ConsoleSyncReporter syncListener;
        private ConsoleLogWriter logWriter;
        private Dictionary<string, string> parsedArguments;

        public HoloToolkit.Sharing.SessionManager SessionManager { get; private set; }

        public SharingManager Manager { get; private set; }

        public bool IsViewerConnected { get; private set; }

        public bool IsServerConnected { get; private set; }

        public NetworkConnectionAdapter ViewerListener { get; private set; }
        public NetworkConnectionAdapter ServerListener { get; private set; }
        public XToolsSessionManagerListener SessionManagerListener { get; private set; }
        public PairingListener PairingListener { get; private set; }

        public event Action<NetworkConnection> ViewerConnected;
        public event Action<NetworkConnection> ViewerConnectionFailed;
        public event Action<NetworkConnection> ViewerDisconnected;
        public event Action<NetworkConnection> SessionConnected;
        public event Action<NetworkConnection> SessionConnectionFailed;
        public event Action<NetworkConnection> SessionDisconnected;

        public XToolsApp(string[] args = null)
        {
            parsedArguments = ParseCommandLine(args);
            this.logWriter = new ConsoleLogWriter();

            ClientConfig config = new ClientConfig(ClientRole.Primary);
            config.SetServerAddress(GetArgumentOrDefault("sessionserver", "localhost"));
            config.SetLogWriter(this.logWriter);

            this.Manager = SharingManager.Create(config);
            this.syncListener = new ConsoleSyncReporter();

            this.viewerConnection = this.Manager.GetPairedConnection();
            this.serverConnection = this.Manager.GetServerConnection();
            this.SessionManager = this.Manager.GetSessionManager();

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

            this.rootObject = this.Manager.GetRootSyncObject();
            this.rootObject.AddListener(this.syncListener);

            // Listen for new sessions
            SessionManagerListener = new XToolsSessionManagerListener();
            this.SessionManager.AddListener(SessionManagerListener);
        }

        private Dictionary<string, string> ParseCommandLine(string[] args)
        {
            Dictionary<string, string> commandLine = new Dictionary<string, string>();
            if (args != null)
            {
                foreach (string arg in args)
                {
                    string[] parts = arg.Split(':');
                    if (parts.Length > 1)
                    {
                        commandLine[parts[0].TrimStart('/', '-').ToLowerInvariant()] = parts[1];
                    }
                    else
                    {
                        commandLine[parts[0].TrimStart('/', '-').ToLowerInvariant()] = string.Empty;
                    }
                }
            }
            return commandLine;
        }

        public string GetArgumentOrDefault(string argument, string defaultValue)
        {
            string value;
            if (this.parsedArguments.TryGetValue(argument, out value))
            {
                return value;
            }

            return defaultValue;
        }

        public bool HasArgument(string argument)
        {
            return this.parsedArguments.ContainsKey(argument);
        }

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
                    if (this.syncListener != null)
                    {
                        this.syncListener.Dispose();
                        this.syncListener = null;
                    }

                    if (this.viewerConnection != null)
                    {
                        this.viewerConnection.Dispose();
                        this.viewerConnection = null;
                    }

                    if (this.serverConnection != null)
                    {
                        this.serverConnection.Dispose();
                        this.serverConnection = null;
                    }

                    if (this.rootObject != null)
                    {
                        this.rootObject.Dispose();
                        this.rootObject = null;
                    }

                    if (this.SessionManager != null)
                    {
                        this.SessionManager.Dispose();
                        this.SessionManager = null;
                    }

                    if (this.Manager != null)
                    {
                        this.Manager.Dispose();
                        this.Manager = null;
                    }
                }

                this.disposed = true;
            }
        }

        // Pumps the network message loop
        public void Update()
        {
            Manager.Update();
        }

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

        private void OnSessionConnected(NetworkConnection connection)
        {
            this.IsServerConnected = true;
            LogWriteLine("Server connected.");

            if (SessionConnected != null)
            {
                SessionConnected(connection);
            }
        }

        private void OnSessionConnectionFailed(NetworkConnection connection)
        {
            this.IsServerConnected = false;
            LogWriteLine("Server connection failed.");

            if (SessionConnectionFailed != null)
            {
                SessionConnectionFailed(connection);
            }
        }

        private void OnSessionDisconnected(NetworkConnection connection)
        {
            this.IsServerConnected = false;
            LogWriteLine("Server connection disconnected.");

            if (SessionDisconnected != null)
            {
                SessionDisconnected(connection);
            }
        }

        private void OnViewerConnected(NetworkConnection connection)
        {
            this.IsViewerConnected = true;
            LogWriteLine("App connected.");

            if (ViewerConnected != null)
            {
                ViewerConnected(connection);
            }
        }

        private void OnViewerConnectionFailed(NetworkConnection connection)
        {
            this.IsViewerConnected = false;
            LogWriteLine("App connection failed.");

            if (ViewerConnectionFailed != null)
            {
                ViewerConnectionFailed(connection);
            }
        }

        private void OnViewerDisconnected(NetworkConnection connection)
        {
            this.IsViewerConnected = false;
            LogWriteLine("App connection disconnected.");

            if (ViewerDisconnected != null)
            {
                ViewerDisconnected(connection);
            }

            BeginPairing();
        }

        private static void LogWriteLine(string message)
        {
            System.Diagnostics.Debug.WriteLine(message);
            Console.WriteLine(message);
        }

        private bool BeginPairing()
        {
            this.PairingListener = new DirectPairingListener(this);

            return this.Manager.GetPairingManager().BeginPairing(new DirectPairReceiver(), PairingListener) == PairingResult.Ok;
        }

        private class ConsoleSyncReporter : ObjectElementListener
        {

        }

        private class ConsoleLogWriter : LogWriter
        {
            public override void WriteLogEntry(LogSeverity severity, string message)
            {
                base.WriteLogEntry(severity, message);

                if (System.Diagnostics.Debugger.IsAttached)
                {
                    System.Diagnostics.Debug.WriteLine(message);
                }

                System.Console.WriteLine(message);
            }
        }

        private class DirectPairingListener : PairingListener
        {
            private readonly XToolsApp app;

            public DirectPairingListener(XToolsApp app)
            {
                this.app = app;
            }

            public override void PairingConnectionSucceeded()
            {
                LogWriteLine("Pairing succeeded");
                app.PairingListener = null;
            }

            public override void PairingConnectionFailed(PairingResult reason)
            {
                LogWriteLine("Pairing connection failed");
                app.BeginPairing();
            }
        }
    }
}
