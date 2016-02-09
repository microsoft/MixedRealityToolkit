using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace XTools
{
    class DiscoveryClientAdapter : DiscoveryClientListener
    {
        public event Action<DiscoveredSystem> DiscoveredEvent;
        public event Action<DiscoveredSystem> LostEvent;

        public override void OnRemoteSystemDiscovered(DiscoveredSystem remoteSystem) 
        {
            if (this.DiscoveredEvent != null)
            {
                this.DiscoveredEvent(remoteSystem);
            }
        }

	    public override void OnRemoteSystemLost(DiscoveredSystem remoteSystem)
        {
            if (this.LostEvent != null)
            {
                this.LostEvent(remoteSystem);
            }
        }
    }
}
