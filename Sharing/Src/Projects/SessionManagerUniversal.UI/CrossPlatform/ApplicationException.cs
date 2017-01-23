// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

namespace System
{
    public class ApplicationException : Exception
    {
        public ApplicationException() {}
        public ApplicationException(string message) : base(message) {}
        public ApplicationException(string message, Exception innerException) : base(message, innerException) {}
    }
}