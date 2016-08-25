// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#ifndef _NAME_Z_H
#define _NAME_Z_H
#include <String_Z.h>

#include <BeginDef_Z.h>
//------------------------------------------------------------------------------------------
//										Name_Z
//------------------------------------------------------------------------------------------
#define	_NAME_Z_STRING		//Default add the String 
 
#ifdef	_DEBUG
//	#undef	_NAME_Z_STRING	// Debug
#endif
#ifdef	_RELEASE
//	#undef	_NAME_Z_STRING	// Release
#endif
#if defined(_MASTER) && !defined(_BIGFILE) && !defined(_SUBMISSION)
	#undef	_NAME_Z_STRING	// Master
#endif
#if defined(_MASTER) && defined(_BIGFILE) && !defined(_SUBMISSION)
	#undef	_NAME_Z_STRING	// MasterBF
#endif
#if defined(_MASTER) && defined(_BIGFILE) && defined(_SUBMISSION)
	#undef	_NAME_Z_STRING	// Submission
#endif

//------------------------------------------------------------------------------------------
//										Class Name_Z
//------------------------------------------------------------------------------------------


struct  Name_Z{
	U32		ID;
	Char	Name[ARRAY_CHAR_MAX];
public:
	            Name_Z()												{ID = 0; Name[0]=0;}
	            Name_Z(const Char *_Name)								{SetName(_Name);}
    explicit    Name_Z(int _ID)											{ID = (U32) _ID; Name[0]=0;}
	            Name_Z(const Name_Z &_Name)								{ID = _Name.ID; strcpy_s(Name, ARRAY_CHAR_MAX, _Name.Name);}
	            Name_Z(const String_Z<ARRAY_CHAR_MAX> &_String)			{SetName(_String);}
			void	SetName(const Char *_Name)				            {ID = 0; if(!_Name) { Name[0]=0; return; }ASSERTC_Z(strlen(_Name)<ARRAY_CHAR_MAX,_Name); strcpy_s(Name, ARRAY_CHAR_MAX, _Name); _strupr_s(Name, ARRAY_CHAR_MAX); ID = GetID(Name);}
	static	U32		GetID(const Char *_Name,U32	ContinueCRC = 0);
	static	U32		GetID(const U8 *_Data,U32 Len,U32 ContinueCRC = 0);
	static	U64		GetID64(const Char *_Name);
	static	U64		GetID64(const U8 *_Data,U32 Len);
			U32		GetID(void)	const						            { return ID; }
	inline	Name_Z &operator =(const Name_Z &_Name)			            {ID = _Name.ID; strcpy_s(Name, ARRAY_CHAR_MAX, _Name.Name);return (*this);}
	inline	Bool	operator <(const Name_Z &_Name) const				{return strcmp( Name, _Name.Name) < 0;}
	inline	Bool	operator >(const Name_Z &_Name) const				{return strcmp( Name, _Name.Name) > 0;}
	inline	Bool	operator ==(const Name_Z &_Name)
	{
		if(!strcmp(_Name.Name,Name))
		{
			EXCEPTIONC_Z(ID==_Name.ID,"Bug CRC Name_Z: %s, %s", GetString(0), _Name.GetString(1));
			return TRUE;
		}
		EXCEPTIONC_Z(ID!=_Name.ID,"Bug CRC Name_Z: %s, %s", GetString(0), _Name.GetString(1));
		return FALSE;
	}
	inline	Bool	operator ==(const Name_Z &_Name) const
	{
		if(!strcmp(_Name.Name,Name))
		{
			EXCEPTIONC_Z(ID==_Name.ID,"Bug CRC Name_Z: %s, %s", GetString(0), _Name.GetString(1));
			return TRUE;
		}
		EXCEPTIONC_Z(ID!=_Name.ID,"Bug CRC Name_Z: %s, %s", GetString(0), _Name.GetString(1));
		return FALSE;
	}
	inline	Bool	operator !=(const Name_Z &_Name)
	{
		if(!strcmp(_Name.Name,Name))
		{
			EXCEPTIONC_Z(ID==_Name.ID,"Bug CRC Name_Z: %s, %s", GetString(0), _Name.GetString(1));
			return FALSE;
		}
		EXCEPTIONC_Z(ID!=_Name.ID,"Bug CRC Name_Z: %s, %s", GetString(0), _Name.GetString(1));
		return TRUE;
	}
	inline	Bool	operator !=(const Name_Z &_Name) const 
	{
		if(!strcmp(_Name.Name,Name))
		{
			EXCEPTIONC_Z(ID==_Name.ID,"Bug CRC Name_Z: %s, %s", GetString(0), _Name.GetString(1));
			return FALSE;
		}
		EXCEPTIONC_Z(ID!=_Name.ID,"Bug CRC Name_Z: %s, %s", GetString(0), _Name.GetString(1));
		return TRUE;
	}
	inline	void	operator +=(const char *_Name)	
	{
		strcat_s(Name,ARRAY_CHAR_MAX,_Name);
		_strupr_s(Name, ARRAY_CHAR_MAX);
		U32 testid=GetID(Name);
		ID = GetID(_Name,ID);
		EXCEPTIONC_Z(ID==testid,"+= de Name_Z error");
	}
	const Char*		GetString(S32 DefaultStringArrayID=0) const;
	static	const	S32	StrSize=256;
	friend	class	ClassManager_Z;
	friend	class	ResourceIO_Z;
	friend	class	BF_ClassManager_Z;
};

void InitNameZTable();

typedef DynArray_Z<Name_Z,32,FALSE,TRUE,4>		Name_ZDA;
typedef DynArray_Z<Name_ZDA,32,FALSE,TRUE,4>	Name_ZDADA;

EXTERN_Z const Name_Z	NAME_NULL;

#endif
