//
// Copyright (C) Microsoft. All rights reserved.
//

namespace XTools
{
    /// <summary>
    /// Allows users of UserPresenceManager to register to receive event callbacks without
    /// having their classes inherit directly from UserPresenceManagerListener
    /// </summary>
    public class UserPresenceManagerAdapter : UserPresenceManagerListener
    {
        public delegate void UserPresenceChangedDelegate(User user);

        public UserPresenceChangedDelegate UserPresenceStateChangedCallback;

        public override void OnUserPresenceChanged(User user)
        {
            if (this.UserPresenceStateChangedCallback != null)
            {
                this.UserPresenceStateChangedCallback(user);
            }
        }
    }
}
