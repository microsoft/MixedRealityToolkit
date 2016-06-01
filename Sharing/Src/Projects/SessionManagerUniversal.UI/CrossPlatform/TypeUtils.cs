// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.
using System;
using System.Reflection;

public static class TypeUtils
{
    public static Type GetBaseType(this Type type)
    {
        return type.GetTypeInfo().BaseType;
    }
}