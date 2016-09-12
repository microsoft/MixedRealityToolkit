// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <pch.h>
#include <Math_Z.h>


//----------------------------------------------------------------
// Gestion Eular.
//----------------------------------------------------------------

void	Mat3x3::GetEular(Vec3f &Eular)
{
	register float	cosy,siny;

	siny = -m.m[0][2];
	if (siny >= 1.f) Eular.y = (Pi * 0.5f);
	else if (siny <= -1.f) Eular.y = -(Pi * 0.5f);
	else Eular.y = ASin(siny);

	cosy = Cos(Eular.y);

	if (cosy > 1e-6f)
	{
		cosy = 1.f / cosy;
		Eular.x = Atan2(m.m[1][2]*cosy,m.m[2][2]*cosy);
		Eular.z = Atan2(m.m[0][1]*cosy,m.m[0][0]*cosy);
	}
	else
	{
		if (Eular.y > 0.f)
		{
			// Sinus Y positif
			// sin(x-z) = m[1][0]
			// cos(x-z) = m[1][1]
			Eular.z = 0.f;
			Eular.x = Atan2(m.m[1][0],m.m[1][1]);
		}
		else
		{
			// Sinus Y n�gatif
			// -sin(x+z) = m[1][0]
			//  cos(x+z) = m[1][1]
			Eular.z = 0.f;
			Eular.x = Atan2(-m.m[1][0],m.m[1][1]);
		}
	}
}
