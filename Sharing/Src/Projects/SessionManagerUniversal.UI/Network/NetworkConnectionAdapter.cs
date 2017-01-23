// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using HoloToolkit.Sharing;

namespace SessionManagerUniversal.UI.Network
{
    /// <summary>
    /// Allows users of NetworkConnection to register to receive event callbacks without
    /// having their classes inherit directly from NetworkConnectionListener
    /// </summary>
    public class NetworkConnectionAdapter : NetworkConnectionListener
    {
        public event System.Action<NetworkConnection> ConnectedCallback;
        public event System.Action<NetworkConnection> ConnectionFailedCallback;
        public event System.Action<NetworkConnection> DisconnectedCallback;
        public event System.Action<NetworkConnection, NetworkInMessage> MessageReceivedCallback;

        public NetworkConnectionAdapter() { }

        public override void OnConnected(NetworkConnection connection)
        {
            Profile.BeginRange("OnConnected");
            this.ConnectedCallback?.Invoke(connection);
            Profile.EndRange();
        }

        public override void OnConnectFailed(NetworkConnection connection)
        {
            Profile.BeginRange("OnConnectFailed");
            this.ConnectionFailedCallback?.Invoke(connection);
            Profile.EndRange();
        }

        public override void OnDisconnected(NetworkConnection connection)
        {
            Profile.BeginRange("OnDisconnected");
            this.DisconnectedCallback?.Invoke(connection);
            Profile.EndRange();
        }

        public override void OnMessageReceived(NetworkConnection connection, NetworkInMessage message)
        {
            Profile.BeginRange("OnMessageReceived");
            this.MessageReceivedCallback?.Invoke(connection, message);
            Profile.EndRange();
        }
    }
}
