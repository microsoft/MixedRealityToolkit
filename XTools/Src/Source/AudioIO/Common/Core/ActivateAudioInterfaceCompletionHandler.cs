using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;
using System.Threading;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using System.Linq;

using AudioIO_DLL.Core.Interfaces;

namespace AudioIO_DLL.Core
{
    class ActivateAudioInterfaceCompletionHandler :
       IActivateAudioInterfaceCompletionHandler, IAgileObject
    {
        private Action<IAudioClient2> completionAction;
        private TaskCompletionSource<IAudioClient2> tcs = new TaskCompletionSource<IAudioClient2>();

        public ActivateAudioInterfaceCompletionHandler(Action<IAudioClient2> completionAction)
        {
            this.completionAction = completionAction;
        }

        public void ActivateCompleted(IActivateAudioInterfaceAsyncOperation activateOperation)
        {
            // First get the activation results, and see if anything bad happened then
            int hr = 0;
            object unk = null;
            activateOperation.GetActivateResult(out hr, out unk);
            if (hr != 0)
            {
                tcs.TrySetException(Marshal.GetExceptionForHR(hr, new IntPtr(-1)));
                return;
            }

            var audioClient = (IAudioClient2)unk;

            // Next try to call the client's (synchronous, blocking) initialization method.
            try
            {
                this.completionAction(audioClient);
                tcs.SetResult(audioClient);
            }
            catch (Exception ex)
            {
                tcs.TrySetException(ex);
            }


        }

        public Task<IAudioClient2> GetTask()
        {
            return tcs.Task;
        }
    }
}