// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef _STRING_Z_H
#define _STRING_Z_H

#include <DynArray_Z.h>
#include <BeginDef_Z.h>



U8 EXTERN_Z		g_ChartoLower[256];
#define fstricmp(str1,str2)		stricmp(str1,str2)
#define fstrcmp(str1,str2)		strcmp(str1,str2)

template<S32 Size> class String_Z{
protected:
	Char	str[Size];
public:
	static	const	S32 ArraySize=Size;
	String_Z(){Empty();}
	String_Z(const Char* text)				{ StrCpy(text); }
	String_Z(const String_Z<Size> &Src)		{ StrCpy(Src.Get());}
	String_Z(S32 Id)						{ Sprintf("%d",Id); }
					S32 GetSize() const		{return ArraySize;}
					const Char*	Get() const {return str;}
					Char*	Get()			{return str;}
					void*	GetPtr()		{return str;}
	operator const	Char	*() const		{return str;}
	operator const	U8		*() const		{return (U8 *)str;}
	operator		Char	*()				{return str;}

	Char	&operator [](int Id)			{ASSERTC_Z(Id>=0 && Id<Size,str); return str[Id];}

	template <S32 T> String_Z<Size> operator + ( const String_Z<T>& other ) const
	{
		String_Z<Size> newString;
		newString.StrCat(str );
		newString.StrCat(other.Get() );
		return newString;
	}
	template <S32 T> String_Z<Size> &operator = ( const String_Z<T>& other )
	{
		StrCpy(other.Get());
		return *this;
	}
	template <S32 T> String_Z<Size> &operator += ( const String_Z<T>& other )
	{
		StrCat(other.Get());
		return *this;
	}
	template <S32 S> Bool operator < ( const String_Z<S>& other ) const
	{
		return strcmp( str, other.str ) < 0;
	}
	template <S32 S> Bool operator > ( const String_Z<S>& other ) const
	{
		return strcmp( str, other.str ) > 0;
	}
    template <S32 S> Bool operator == (const String_Z<S>& _str2) const
    {
        return strcmp( str, _str2.str) == 0;
    }
    template <S32 S> Bool operator != (const String_Z<S>& _str2) const
    {
        return !operator==(_str2);
    }

	void	Sprintf(const Char *_Str,...)
			{
				va_list Marker;
				va_start( Marker,_Str);
				Vsprintf(_Str,Marker);
				va_end( Marker );
			}
	void	Vsprintf(const Char *_Str,va_list va)
			{
				EXCEPTION_Z(_Str!=Get());
				S32 error = vsnprintf(str,Size-1,_Str,va);
				str[Size-1] = 0;		// vsnprintf work like strncpy => It Don't set the 0 if String is equal or greater than Size-1.
				EXCEPTIONC_Z(error >= 0,"Overflow String_Z<%d>::VSprintf overflow %s",Size,_Str);
			}
	void	SprintfCat(const Char *_Str,...)
			{
				EXCEPTION_Z(_Str!=Get());
				int len=(int)strlen(str);
				va_list Marker;
				va_start( Marker,_Str);
				S32 error = vsnprintf(str+len,Size-len-1,_Str,Marker);
				str[Size-1] = 0;		// vsnprintf work like strncpy => It Don't set the 0 if String is equal or greater than Size-1.
				EXCEPTIONC_Z(error >= 0,"Overflow String_Z<%d>::SprintfCat %s",Size,_Str);
				va_end( Marker );
			}
 	void	NSprintf(const Char *_Str,...)
			{
				va_list Marker;
				va_start( Marker,_Str);
				S32 error = vsnprintf(str,Size-1,_Str,Marker);
				str[Size-1] = 0;		// vsnprintf work like strncpy => It Don't set the 0 if String is equal or greater than Size-1.
				va_end( Marker );		// No exception on owerflow of size
			}

	int		StrLen(void) const				{int n=(int)strlen(str);EXCEPTIONC_Z(n<Size,"Overflow de String_Z<%d>::StrLen");return n;}
	int		DataLen(void) const				{return StrLen()+1;}
	int		StrReserve(void) const			{return Size;}
	void	SetName(const Char *_Str)		{StrCpy(_Str);}

	Bool	RemoveLastChar( )
			{
				int	n			 = StrLen();
				if(n>0)
				{
					str[n-1] = 0;
					return TRUE;
				}
				return FALSE;
			}

	void	StrCpy(const Char *_Str)		
			{
				EXCEPTIONC_Z( strlen(_Str)<Size,"Overflow de String_Z<%d>::StrCpy %s",Size,_Str); 
				strcpy_s(str, Size,_Str);
			}

	void	StrCat(const Char *_Str)
			{
				EXCEPTIONC_Z(strlen(str)+strlen(_Str)<Size,"Overflow de String_Z<%d>::StrCat(%s) on (%s)",Size,_Str, str); 
				strcat_s(str,Size,_Str);
			}
	void	CharCat(Char c)
			{
				int	n=StrLen();
				EXCEPTIONC_Z(n<Size-1,"Overflow de String_Z<%d>::CharCat(%c) %s",Size,c,str); 
				str[n]=c;
				str[n+1]=0;
			}
	void	StrnCat(const Char *_Str,int _n)
			{
				int	l=(int)strlen(str);
				EXCEPTIONC_Z(l+_n<Size,"Overflow de String_Z<%d>::StrnCat %s",Size,_Str); 
				strncpy(str+l,_Str,_n);
				str[l+_n]=0; 
			}
	Bool	StrRemove(const Char *_Str)
			{
				Char *pCur=strstr(str,_Str);
				if(!pCur)	return FALSE;
				strcpy(pCur,pCur+(int)strlen(_Str));
				return	TRUE;
			}
	void	StrnCpy(const Char *_Str,S32 n)
			{
				EXCEPTIONC_Z(n<Size,"Overflow de String_Z<%d>::StrnCpy %s",Size,_Str); 
				strncpy(str,_Str,n);
				str[n]=0;
			}
	int		Trim(const Char *_Str)
			{
				int	n=(int)strlen(_Str);
				Char	*pCur=StrStr(_Str);
				while(pCur)
				{
					strcpy(pCur,pCur+n);
					pCur=StrStr(_Str);
				}
				return	StrLen();
			}
	int		ReverseFind(Char aChar)const
			{
				int	n=StrLen();
				for(n--;n>=0;n--)
					if(str[n]==aChar)
						break;
				return n;
			}
	void	Replace(Char aC,Char toC,S32 Start=0,S32 End=-1)
			{
				int	CurLen=StrLen();
				if(!CurLen)	return;
				EXCEPTION_Z(Start>=0 && Start<CurLen);
				Char	*pStr=&str[Start];
				for(;*pStr&&Start!=End;pStr++,Start++)
					if(*pStr==aC)
						*pStr=toC;
			}
	S32		Replace(const Char *aStr,const Char *toStr)
			{
				const int SrcLen=(int)strlen(aStr);
				if(!SrcLen)	return 0;
				const int RepLen=(int)strlen(toStr);
				Char*	pStr=&str[0];
				Char*	pFound=strstr(pStr,aStr);
				S32		ReplacedCount=0;
				while(pFound)
				{
					++ReplacedCount;
					if(SrcLen<RepLen)
					{
						int n=(int)strlen(pFound)+1;
						EXCEPTION_Z(Size >= (n-1+pFound-pStr));
						memmove(pFound+(RepLen-SrcLen),pFound,n);
						memcpy(pFound,toStr,RepLen);
					}
					else
					{
						int n=(int)strlen(pFound+SrcLen)+1;
						memcpy(pFound,toStr,RepLen);
						memcpy(pFound+RepLen,pFound+SrcLen,n);
					}
					pFound=strstr(pFound+RepLen,aStr);
				}
				return ReplacedCount;
			}
	String_Z<Size> Mid(S32 iFirst,S32 nCount) const
			{
				if(iFirst<0)	iFirst=0;
				if(nCount<0)	nCount=0;
				S32	Len=StrLen();
				if(iFirst+nCount>Len)
					nCount=Len-iFirst;
				if(iFirst>Len)
					nCount=0;
				if(iFirst==0 && (iFirst+nCount)==Len)
					return *this;
				String_Z<Size>	ret;
				ret.StrnCat(Get()+iFirst,nCount);
				return ret;
			}

	int		StrCmp(const Char *_Str) const			{return strcmp(str,_Str);}

	int		StrnCmp(const Char *_Str,int _n) const	{return strncmp(str,_Str,_n);}

	int		StrniCmp(const Char *_Str,int _n) const	{return strnicmp(str,_Str,_n);}

	Char *	StrUpr()								{return strupr(str);}

    Char*   StrLwr()                                {return strlwr(str);}

	Char *	StrStr(const Char *_Str) const			{return (Char*)strstr(str,_Str);}

	Char *	StrChr(int _c) const					{return (Char*)strchr(str,_c);}

	Char *	StrrChr(int _c)const					{return (Char*)strrchr(str,_c);}

	Bool	FStrCmp(const Char *_Str) const			{return	fstrcmp(str,_Str);}

	Bool	FStriCmp(const Char *_Str) const		{return	fstricmp(str,_Str);}

    Bool    IsEmpty() const                         { return str[0] == '\0'; }

	void	MakeUp(void)							{strupr(str);}

	void	Empty(void)								{ *str=0; }
};

EXTERN_Z const String_Z<8>	STRING_EMPTY;

typedef	DynArray_Z<String_Z<ARRAY_CHAR_MAX>,16>		String_ZDA;

#endif
