// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ProfilerX
{
    public class ProfilerApp : IDisposable
    {
        private bool disposed;

        public bool Recording = true;

        public HoloToolkit.Sharing.ProfilerStreamManager Manager { get; private set; }

        public ProfilerApp()
        {
            this.Manager = HoloToolkit.Sharing.ProfilerStreamManager.Create();
            this.disposed = false;
        }

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
            this.Manager.Update();
        }
    }
}
