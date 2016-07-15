// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Ogre3D_DX9_BackbufferGrabber.h"

void Ogre3D_DX9_BackbufferGrabber::InitBackbufferGrabber(Ogre::RenderWindow* renderWindow, int _width, int _height)
{
	LPDIRECT3DDEVICE9 DX9Device;
	renderWindow->getCustomAttribute("D3DDEVICE", &DX9Device);
	DX9_BackbufferGrabber::InitBackbufferGrabber(DX9Device, _width, _height);
}
