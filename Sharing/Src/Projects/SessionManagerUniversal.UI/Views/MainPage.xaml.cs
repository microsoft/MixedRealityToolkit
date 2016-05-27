using System;
using HoloToolkit.Sharing;
using SessionManagerUniversal.UI.Network;
using Windows.UI;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Media;
using System.Collections.ObjectModel;
using SessionManagerUniversal.UI.DataModel;
using System.Linq;

namespace SessionManagerUniversal.UI.Views
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage
    {
        private XToolsApp SharingStage;
        private ObservableCollection<SessionData> _sessions;

        private bool _isConnected;

        public MainPage()
        {
            this.InitializeComponent();
            SharingStage = new XToolsApp();
            SharingStage.LogWriter.Log += LogAction;
        }

        #region Connection

        private void ConnectButton_Click(object sender, RoutedEventArgs e)
        {
            if (string.IsNullOrEmpty(IPText.Text) || string.IsNullOrEmpty(UserText.Text))
                return;

            ConnectButton.IsEnabled = false;
            //if (SharingStage?.SharingManager?.GetSessionManager()?.IsServerConnected() == true)
            if (_isConnected)
            {
                SharingStage.Disconnect();
                _sessions.Clear();
            }
            else
            {
                SharingStage.Connect(IPText.Text, UserText.Text);
                InitializeSessions();
            }
        }

        #endregion


        #region Create Session

        private void CreateButton_Click(object sender, RoutedEventArgs e)
        {
            if (string.IsNullOrEmpty(SessionName.Text))
                return;

            SharingStage.CreateSession(SessionName.Text);
        }


        #endregion


        #region Sessions

        private void InitializeSessions()
        {
            SharingStage.SessionManagerListener.SessionAdded += OnSessionAdded;
            SharingStage.SessionManagerListener.SessionClosed += OnSessionClosed;
            SharingStage.SessionManagerListener.SessionUserChanged += OnSessionUserChanged;
            SharingStage.SessionManagerListener.SessionUserJoined += OnSessionUserJoined;
            SharingStage.SessionManagerListener.SessionUserLeft += OnSessionUserLeft;
            SharingStage.SessionManagerListener.SessionServerConnected += OnSessionServerConnected;
            SharingStage.SessionManagerListener.SessionServerDisconnected += OnSessionServerDisconnected;
            _sessions = new ObservableCollection<SessionData>();
            this.SessionsList.ItemsSource = _sessions;
        }

        private void OnSessionServerDisconnected()
        {
            Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, new Windows.UI.Core.DispatchedHandler(() =>
            {
                ConnectButton.IsEnabled = true;
                _isConnected = false;
                StatusIcon.Fill = new SolidColorBrush(Colors.Red);
                ConnectButton.Content = "Connect";
            }));
        }

        private void OnSessionServerConnected()
        {
            Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, new Windows.UI.Core.DispatchedHandler(() =>
            {
                ConnectButton.IsEnabled = true;
                _isConnected = true;
                StatusIcon.Fill = new SolidColorBrush(Colors.Green);
                ConnectButton.Content = "Disconnect";
            }));
        }

        private void OnSessionUserLeft(Session session, User user)
        {
            Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, new Windows.UI.Core.DispatchedHandler(() =>
            {
                FindSession(session)?.RemoveUser(user);
            }));
        }

        private void OnSessionUserJoined(Session session, User user)
        {
            Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, new Windows.UI.Core.DispatchedHandler(() =>
            {
                FindSession(session)?.AddUser(user);
            }));
        }

        private void OnSessionUserChanged(Session session, User user)
        {
            Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, new Windows.UI.Core.DispatchedHandler(() =>
            {
                FindSession(session)?.UpdateUser(user);
            }));
        }

        private void OnSessionClosed(Session obj)
        {
            Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, new Windows.UI.Core.DispatchedHandler(() =>
            {
                //TODO: Any other way to identify a session? How do we handle duplicate session names?
                var currentSession = FindSession(obj);
                if (currentSession != null)
                    _sessions.Remove(currentSession);
            }));
        }

        private void OnSessionAdded(Session obj)
        {
            Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, new Windows.UI.Core.DispatchedHandler(() =>
            {
                _sessions.Add(new SessionData(obj));
            }));
        }

        private SessionData FindSession(Session session)
        {
            if (session == null)
                //TODO: log?
                return null;

            return _sessions.FirstOrDefault(s => s.SessionName == session.GetName());
        }


        private void JoinButton_Click(object sender, RoutedEventArgs e)
        {
            if (this.SessionsList.SelectedIndex < 0)
                return;

            SharingStage.JoinSession(_sessions[this.SessionsList.SelectedIndex].SessionName);
        }

        private void LeaveButton_Click(object sender, RoutedEventArgs e)
        {
            SharingStage.LeaveSession();
        }

        #endregion

        private async void LogAction(LogSeverity s, string msg)
        {
            await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, new Windows.UI.Core.DispatchedHandler(() =>
            {
                //logCount++;
                if (this.LogTextList.Items.Count > 1000)
                {
                    this.LogTextList.Items.RemoveAt(0);
                }

                this.LogTextList.Items.Add(new Windows.UI.Xaml.Controls.TextBlock()
                {
                    Text = string.Format("{0} - {1}", System.DateTime.Now.ToString("HH:mm:ss"), msg),
                    Foreground = s == LogSeverity.Error ? new SolidColorBrush(Colors.Red) : (s == LogSeverity.Warning ? new SolidColorBrush(Colors.Orange) : new SolidColorBrush(Colors.Black)),
                    TextWrapping = TextWrapping.Wrap,
                });
                this.LogScroller.ScrollToVerticalOffset(this.LogScroller.ScrollableHeight);
            }));
        }

    }
}
