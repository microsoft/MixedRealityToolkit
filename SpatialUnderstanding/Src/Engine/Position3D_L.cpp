// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <pch.h>
#include <Position3D_L.h>

const Position3D_L POSITION3D_NULL = Position3D_L(VEC3F_NULL, Quat(1.f,0.f,0.f,0.f)/*QUAT_NULL*/);

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
Position3D_L::Position3D_L()
{
	Reset();
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
Position3D_L::Position3D_L(const Vec3f& _vVal, const Quat& _qVal)
{
	m_vPos = _vVal;
	m_qQuat=_qVal;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void	Position3D_L::Reset()
{
	m_vPos = VEC3F_NULL;
	m_qQuat= QUAT_NULL;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
Bool Position3D_L::IsPosNull() const
{
	if (m_vPos!=VEC3F_NULL)
		return FALSE;
	return TRUE;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
Bool Position3D_L::IsRotNull() const
{
	if (m_qQuat!=QUAT_NULL)
		return FALSE;
	return TRUE;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
Bool Position3D_L::IsPosAndRotNull() const
{
	if(IsPosNull()&&IsRotNull())
	{
		return TRUE;
	}

	return FALSE;
}
