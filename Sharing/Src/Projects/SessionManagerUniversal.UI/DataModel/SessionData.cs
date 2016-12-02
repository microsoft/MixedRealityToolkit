// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using HoloToolkit.Sharing;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace SessionManagerUniversal.UI.DataModel
{
    public class SessionData : INotifyPropertyChanged
    {
        private List<User> _users;

        public event PropertyChangedEventHandler PropertyChanged;

        public string SessionName { get; set; }

        public string SessionUserCount { get; set; }

        public string SessionType { get; set; }

        public string UserNames { get; set; }

        public SessionData(Session s)
        {
            SessionName = s.GetName().ToString();
            SessionUserCount = s.GetUserCount().ToString();
            SessionType = s.GetSessionType().ToString();
            _users = new List<User>();
            for (var i = 0; i < s.GetUserCount(); i++)
            {
                _users.Add(s.GetUser(i));
            }
            UpdateUserNames();
        }


        public void AddUser(User user)
        {
            _users.Add(user);
            UpdateUserNames();
        }

        public void RemoveUser(User user)
        {
            _users.RemoveAll(u => u.GetID() == user.GetID());
            UpdateUserNames();
        }

        public void UpdateUser(User user)
        {
            var myUser = _users.FirstOrDefault(u => u.GetID() == user.GetID());
            if (myUser != null)
            {
                _users.Remove(myUser);
                _users.Add(user);
            }
            UpdateUserNames();
        }

        private void UpdateUserNames()
        {
            UserNames = string.Join(", ", _users.Select(u => u.GetName().ToString()).ToArray());
            SessionUserCount = _users.Count().ToString();
            OnPropertyChanged("UserNames");
            OnPropertyChanged("SessionUserCount");
        }


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
