// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef _POSITION3D_L_H
#define _POSITION3D_L_H

class Position3D_L
{
public:
	POSTALIGNED128_Z Quat		m_qQuat ALIGNED128_Z;
	Vec3f		m_vPos;

				Position3D_L();
				Position3D_L(const Vec3f& _vVal, const Quat& _qVal);

	Bool		operator == ( const Position3D_L& _pos ) const {return ( (m_vPos == _pos.GetPos()) && (m_qQuat == _pos.GetRot()) );}
	Bool		operator != ( const Position3D_L& _pos ) const {return !(_pos == *this);}

	virtual	void		Reset();

	const Vec3f& GetPos() const
	{
		return m_vPos;
	}
	const Quat& GetRot() const
	{
		return m_qQuat;
	}

	void SetPos(const Vec3f& _vPos)
	{
		m_vPos=_vPos;
	}
	void SetRot(const Quat& _qRot)
	{
		m_qQuat=_qRot;
	}

	Bool	IsPosNull() const;
	Bool	IsRotNull() const;
	Bool	IsPosAndRotNull() const;
};

const EXTERN_Z Position3D_L	POSITION3D_NULL;

#endif //_POSITION3D_L_H
