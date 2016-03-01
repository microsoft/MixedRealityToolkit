//
// Copyright (C) Microsoft. All rights reserved.
//

namespace HoloToolkit.Sharing
{
    /// <summary>
    /// Allows users of NetworkConnection to register to receive event callbacks without
    /// having their classes inherit directly from NetworkConnectionListener
    /// </summary>
    public class NetworkConnectionAdapter : NetworkConnectionListener
    {
        public delegate void ConnectedDelegate(NetworkConnection connection);
        public delegate void ConnectionFailedDelegate(NetworkConnection connection);
        public delegate void DisconnectedDelegate(NetworkConnection connection);
        public delegate void MessageReceivedDelegate(NetworkConnection connection, NetworkInMessage message);

        public ConnectedDelegate ConnectedCallback;
        public ConnectionFailedDelegate ConnectionFailedCallback;
        public DisconnectedDelegate DisconnectedCallback;
        public MessageReceivedDelegate MessageReceivedCallback;

        public NetworkConnectionAdapter()
        {

        }

        public override void OnConnected(NetworkConnection connection)
        {
            Profile.BeginRange("OnConnected");
            if (this.ConnectedCallback != null)
            {
                this.ConnectedCallback(connection);
            }
            Profile.EndRange();
        }

        public override void OnConnectFailed(NetworkConnection connection)
        {
            Profile.BeginRange("OnConnectFailed");
            if (this.ConnectionFailedCallback != null)
            {
                this.ConnectionFailedCallback(connection);
            }
            Profile.EndRange();
        }

        public override void OnDisconnected(NetworkConnection connection)
        {
            Profile.BeginRange("OnDisconnected");
            if (this.DisconnectedCallback != null)
            {
                this.DisconnectedCallback(connection);
            }
            Profile.EndRange();
        }

        public override void OnMessageReceived(NetworkConnection connection, NetworkInMessage message)
        {
            Profile.BeginRange("OnMessageReceived");
            if (this.MessageReceivedCallback != null)
            {
                this.MessageReceivedCallback(connection, message);
            }
            Profile.EndRange();
        }
    }
}