using HoloToolkit.Sharing;
using SessionManagerUniversal.UI.Network;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls.Primitives;

namespace SessionManagerUniversal.UI.Views
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage
    {
        XToolsApp SharingStage;

        public MainPage()
        {

            this.InitializeComponent();
            SharingStage = new XToolsApp();
        }

        private void ConnectButton_Click(object sender, RoutedEventArgs e)
        {
            if (SharingStage.IsServerConnected)
            {
                SharingStage.Disconnect();
                ConnectButton.Content = "Connect";

            }
            else
            {
                ConnectButton.Content = "Disconnect";
                SharingStage.Connect(IPText.Text);
            }
        }

        private void JoinButton_Click(object sender, RoutedEventArgs e)
        {
            SharingStage.JoinSession("Default");
        }

        private void SpinBool_Checked(object sender, RoutedEventArgs e)
        {
         
        }

        private void MoleculeRotate(NetworkInMessage msg)
        {
        }

        private void SpinBool_Unchecked(object sender, RoutedEventArgs e)
        {
          
        }

        private void Slider_ValueChanged(object sender, RangeBaseValueChangedEventArgs e)
        {
          
        }

    }
}
