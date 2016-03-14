using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Linq;
using System.Text;


namespace AudioIO_DLL.Core.Interfaces
{

    [ComImport, InterfaceType(ComInterfaceType.InterfaceIsIUnknown), Guid("41D949AB-9862-444A-80F6-C261334DA5EB")]
    interface IActivateAudioInterfaceCompletionHandler
    {
        //virtual HRESULT STDMETHODCALLTYPE ActivateCompleted(/*[in]*/ _In_  
        //   IActivateAudioInterfaceAsyncOperation *activateOperation) = 0;
        void ActivateCompleted(IActivateAudioInterfaceAsyncOperation activateOperation);
    }

}