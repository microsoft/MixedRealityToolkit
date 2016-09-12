// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef HASH_TABLE_Z_H
#define HASH_TABLE_Z_H

/** \file
 * Hash table:
 * ----------
 * 
 * table[Value] = Ref:
 * 	- "Value" is the key used to index an element: it corresponds to a given slot. There can't be 2 keys referencing the same element.
 * 	- "Ref" is a value (-_-), that is supposed to be some reference to an external data structure (e.g Ref could be an index into a DynArray_Z)
 * Because the hash table uses 2 times more memory than needed to just store the elements, it should be used
 * only to store references to bigger objects that are themselves stored in an external data structure.
 * Example:
 * 	DynArray_Z<BigObject>	objects;
 * 	HashName_ZTable_Z		ht;
 * 
 * 	S32 i = objects.Add();
 * 	ht.Insert(Name_ZHash_Z(Name_Z("MyBigObjectName"), i));
 * 
 * 	[...]
 * 
 * 	Name_ZHash_Z	hash;
 * 	hash.Value = Name_Z("MyBigObjectName");
 * 	const Name_ZHash_Z*	pFound = ht.Search(hash);
 * 	if(pFound)
 * 	{
 * 		BigObject*	pFoundObj = &objects[pFound->Ref];
 * 	}
 * 
 * HashTable_Z<T> uses open addressing: conflicts (= same key for different objects) are resolved by multiple hashing:
 * the hash table finds a new hash using T::HashIncrement() and forces the result to be odd.
 * Because the hash increment is odd and the size of the hash table is a power of 2, we are sure to run through the whole hash table
 * in the worst case while searching an element.
 * Removed elements are marked as "Shadow". They then correspond to "bridges" that do not break the chain while searching.
 * 
 * Example: searching for Value=1337:
 * 	hashinc = (odd(HashIncrement(1337)) == odd(2) == 3);
 * 	HashBase(1337)=0
 * 
 *          [Value=42, Ref=0]	        [Value=1337, Ref=3]	    [Value=139, Ref=5]	 [Value=666, Ref=SHADOW_REF=-1]
 *            HashBase(42)=0              HashBase(1337)=0       HashBase(139)=2             HashBase(666)=3
 * Step 1:  (0+hashinc)%4=3  ---------->-------------->-------------->-------------->-------->---------> *
 * Step 2:                                                                                     (3+hashinc)%4=1 ------>-----x
 *                 x-------->--------->-----> FOUND
 * 
 * An element is either:
 * - empty:  Status->GetBit(hashid) == 0 AND IsEmpty()==TRUE
 * - shadow: Status->GetBit(hashid) == 0 AND IsEmpty()==FALSE
 * - used:   Status->GetBit(hashid) == 1: we don't care for IsEmpty()
 * 
 * -------------------------------
 * During Resize(), the hash table is filled with zeroes => the "empty" value for Ref HAS to be 0!
 * Therefore:
 * 
 * 	Requirements for the hash class:
 * 	--------------------------------
 * 	- "Value" member
 * 	- "Ref" member
 * 	- SetShadow() { Ref = any particular value that is NOT 0; }
 * 	- IsEmpty() { if(Ref==0) return TRUE; else return FALSE; }
 * 	- HashBase(): whatever value works, but we should choose a function that maximizes randomness
 * 	- HashIncrement(): whatever value works, as the return value is forced to be odd, but we should choose a function that maximizes randomness
 */

#define	HASHTABLE_DEFAULT_SIZE		16		// 2 fois plus en Alloc (voir code resize)

#include <CheckSumCRC32_Z.h>
#include <BitArray_Z.h>

#include <BeginDef_Z.h>
#include <Name_Z.h>

class ResourceIO_Z;

template<typename RefType>
class  S32HashTOps_Z
{
public:
	static	inline	Bool	IsEmpty(const RefType& Ref);
	static	inline	void	SetShadow(RefType& Ref);
};
template<typename RefType>
class  HashTOps_Z
{
public:
	static	inline	Bool	IsEmpty(const RefType& Ref);
	static	inline	void	SetShadow(RefType& Ref);
};
template<> class  HashTOps_Z<S32>
{
public:
	static  inline  Bool    IsEmpty(const S32 &Ref)   { return !Ref; }
	static  inline  void    SetShadow(S32 & Ref)      { Ref=-1; }
};
template<> class  S32HashTOps_Z<S32>
{
public:
	static  inline  Bool    IsEmpty(const S32 &Ref)   { return !Ref; }
	static  inline  void    SetShadow(S32 & Ref)      { Ref=-1; }
};

template<typename RefType=S32>
class  S32HashT_Z
{
public:
	S32	Value;
	RefType	Ref;

	S32HashT_Z(){}
	S32HashT_Z(S32 _Value)			{Value=_Value;}
	S32HashT_Z(S32 _Value,RefType _Ref)	{Value=_Value;Ref=_Ref;}

	inline	Bool	IsEmpty()const				{return S32HashTOps_Z<RefType>::IsEmpty(Ref);}
	inline	void	SetShadow()					{S32HashTOps_Z<RefType>::SetShadow(Ref);}

	inline	S32		HashBase() const			{return	Value;}
	inline	S32		HashIncrement()	const
	{
		U32	Val = Value;
		Val += (Val >> 4);
		Val += (Val >> 12);
		Val += (Val >> 24);
		return	Val;			// Ok for good randomize (Collision can not been managed).
	}
	Bool	operator==(const S32HashT_Z &Elem)	{return	Elem.Value==Value;}
	Bool	operator!=(const S32HashT_Z &Elem)	{return	Elem.Value!=Value;}
};
typedef S32HashT_Z<>			S32Hash_Z;

template<typename RefType=S32>
class Name_ZHashT_Z
{
public:
	Name_Z	Value;
	RefType	Ref;

	Name_ZHashT_Z(){}
	Name_ZHashT_Z(Name_Z _Value)				{Value=_Value;}
	Name_ZHashT_Z(Name_Z _Value,RefType _Ref)	{Value=_Value;Ref=_Ref;}

	inline	Bool	IsEmpty() const				{return S32HashTOps_Z<RefType>::IsEmpty(Ref);}
	inline	void	SetShadow()					{S32HashTOps_Z<RefType>::SetShadow(Ref);}

	inline	S32		HashBase() const			{return	Value.GetID();}
	inline	S32		HashIncrement()	const
	{
		U32 val = (U32)Value.GetID();
		return	(S32)((val >> 16) + (val << 16));		// Ok for Name_Z because Upper Word is really different than Lower Word.
														// Not Ok for collision of Name_Z, but really rare.
	}
	Bool	operator==(const Name_ZHashT_Z &Elem)	{return	Elem.Value==Value;}
	Bool	operator!=(const Name_ZHashT_Z &Elem)	{return	Elem.Value!=Value;}
	friend ResourceIO_Z & operator >> (ResourceIO_Z &io,Name_ZHashT_Z<RefType> &aValue)
	{
		io >> aValue.Value;
		io >> aValue.Ref;
		return io;
	}
};
typedef	Name_ZHashT_Z<>		Name_ZHash_Z;

template<class TPtr,typename RefType=S32> 
class PtrHashT_Z
{
public:
	TPtr	Ptr;
	RefType Ref;
	PtrHashT_Z(){}
	PtrHashT_Z(TPtr _Ptr) : Ptr(_Ptr){}
	PtrHashT_Z(TPtr _Ptr,RefType _Ref) : Ptr(_Ptr),Ref(_Ref){}

	inline	Bool	IsEmpty() const				{return S32HashTOps_Z<RefType>::IsEmpty(Ref);}
	inline	void	SetShadow()					{S32HashTOps_Z<RefType>::SetShadow(Ref);}

	inline	S32		HashBase() const	
	{
		CheckSumCRC32_Z crc32;
		return (S32) crc32.Calc(0, &Ptr, sizeof(Ptr));
	}
	inline	S32		HashIncrement()	const
	{
		CheckSumCRC32_Z crc32;
		return (S32) crc32.Calc(0xb0d09822, &Ptr, sizeof(Ptr));
	}
	Bool	operator==(const PtrHashT_Z &Elem)	{return	Elem.Ptr==Ptr;}
	Bool	operator!=(const PtrHashT_Z &Elem)	{return	Elem.Ptr!=Ptr;}
};


struct VoidPtrHash_Z
{
	const void	*Address;
	S32         Ref;

	VoidPtrHash_Z(){}
	VoidPtrHash_Z(const void *_address) 
	{ Address=_address; }

	inline	Bool	IsEmpty() const				{return !Ref;}
	inline	void	SetShadow()					{Ref=-1;}

	inline S32 HashBase() const
	{
		CheckSumCRC32_Z crc32;
		return (S32) crc32.Calc(0, &Address, sizeof(Address));
	}

	inline S32 HashIncrement() const
	{
		CheckSumCRC32_Z crc32;
		return (S32) crc32.Calc(0xb0d09822, &Address, sizeof(Address));	// Another Hash, because another start value.
	}

	Bool operator== (const VoidPtrHash_Z &Elem) const { return !memcmp(&Elem.Address,&Address,sizeof(Address)); }
	Bool operator!= (const VoidPtrHash_Z &Elem) const { return !(*this == Elem);                             }
};


template<class T> class HashTableBase_Z
{
protected:
	BitArray_Z		*Status;
	T				*Hash;
	S32				NbElem;
	S32				NbFree;
	mutable	S32		ScanId;

	void			Resize(S32 NewSize)
	{
		S32	OldSize;
		S32 NextScan;

		// Puissance de 2.
		OldSize=1;
		while (OldSize<NewSize) OldSize<<=1;
		OldSize<<=1;
		NewSize=OldSize;

		// Alloc.
		BitArray_Z	*oStatus = Status;
		T			*oHash = Hash;
		Status = New_Z BitArray_Z(NewSize);
		Hash=(T*)Alloc_Z(sizeof(T)*NewSize);
		Status->ClearAllBits();
		memset(Hash,0,sizeof(T)*NewSize);

		// Copie ?
		S32	OldNbElem=NbElem;

		NbElem=0;
		NbFree=NewSize;

		if (!oStatus) return;

		// Reinsert les elements.

		NextScan = 0;
		while ((NextScan=oStatus->FindFirstBit(TRUE,NextScan)) >= 0)
		{
			Bool	bResult=Insert(*(oHash+NextScan));
			NextScan++;
			EXCEPTION_Z(bResult);
		}
		EXCEPTION_Z(OldNbElem==NbElem);
		Free_Z(oHash);
		Delete_Z oStatus;
	}

public:
	HashTableBase_Z() {
		NbElem=0;
		NbFree=0;
		ScanId=-1;
		Status=NULL;
		Hash=NULL;
	}
	HashTableBase_Z(S32 Size)
	{
		NbElem=0;
		NbFree=0;
		ScanId=-1;
		Status=NULL;
		Hash=NULL;
		Resize(Size);
	}
	~HashTableBase_Z()
	{
		Flush();
	}
	HashTableBase_Z(const HashTableBase_Z<T> &Src)
	{
		Status=NULL;
		Hash=NULL;
		Flush();
		Src.InitScan();
		T *pResult=Src.NextScan();
		while(pResult)
		{
			Insert(*pResult);
			pResult=Src.NextScan();
		}
	}
	HashTableBase_Z<T>	&operator=(const HashTableBase_Z<T> &Src)
	{
		Flush();
		Src.InitScan();
		T *pResult=Src.NextScan();
		while(pResult)
		{
			Insert(*pResult);
			pResult=Src.NextScan();
		}
		return *this;
	}
	void	Flush(void)
	{
		if(Status)
		{
			Delete_Z Status;
			Status=NULL;
		}
		if(Hash)
		{
			Free_Z	(Hash);
			Hash=NULL;
		}
		NbElem=0;
		NbFree=0;
		ScanId=-1;
	}
	void	Empty(Bool Deep = TRUE)	// Deep => Si il n'y a eu que des insertions, Deep inutile.
	{
		if(!Status)	return;
		Status->ClearAllBits();
		if(Deep) memset(Hash,0,sizeof(T)*(NbElem+NbFree));
		NbFree+=NbElem;
		NbElem=0;
		ScanId=-1;
	}
	void	Minimize()
	{
		AllocBestFit_Z(TRUE);
		Resize(NbElem);
		AllocBestFit_Z(FALSE);
	}

	Bool	Insert(const T &Element)
	{
		if (!Status) Resize(HASHTABLE_DEFAULT_SIZE);
		S32		hashsize=Status->GetSize()-1;
		S32		hashid=Element.HashBase()&hashsize;
		S32		hashinc=Element.HashIncrement();
		S32		shadowhashid=-1;
		if (!(hashinc & 0x1)) hashinc++;

		for(;;)
		{
			// Vide ou Shadow ?
			if(!Status->GetBit(hashid))
			{
				if (Hash[hashid].IsEmpty())
				{
					// Une case vide.
					if(shadowhashid<0)
						NbFree--;
					else
						hashid=shadowhashid;

					// Insertion.
					Status->SetBit(hashid);
					*(Hash+hashid)=Element;

					NbElem++;

					if ((NbFree==0) || (NbFree < (Status->GetSize() >> 2)))
						Resize(NbElem);

					return TRUE;
				}
				else
				{
					// Un Shadow
					if (shadowhashid<0) shadowhashid = hashid;
				}
			}
			else
			{
				// Un element
				if(Hash[hashid]==Element)
					return	FALSE;
			}
			hashid=(hashid + hashinc) & hashsize;
		}
	}
	//Insert et retourne l Element, (comme ca no peut utiliser le Ref de l Element, ex : compteur de l element) 
	T		*InsertRef(const T &Element,Bool &bInserted)
	{
		if (!Status) Resize(HASHTABLE_DEFAULT_SIZE);
		S32		hashsize=Status->GetSize()-1;
		S32		hashid=Element.HashBase()&hashsize;
		S32		hashinc=Element.HashIncrement();
		S32		shadowhashid=-1;
		if (!(hashinc & 0x1)) hashinc++;
		bInserted = TRUE;
		for(;;)
		{
			// Vide ou Shadow ?
			if(!Status->GetBit(hashid))
			{
				if (Hash[hashid].IsEmpty())
				{
					// Une case vide.
					if(shadowhashid<0)
						NbFree--;
					else
						hashid=shadowhashid;

					// Insertion.
					Status->SetBit(hashid);
					*(Hash+hashid)=Element;

					NbElem++;

					if ((NbFree==0) || (NbFree < (Status->GetSize() >> 2)))
					{
						Resize(NbElem);
						T	*ElePtr=(T *)Search(Element);
                        EXCEPTION_Z(ElePtr != NULL);
						EXCEPTION_Z(*ElePtr==Element);
						return	ElePtr;
					}
					else
						return	Hash+hashid;
				}
				else
				{
					// Un Shadow
					if (shadowhashid<0) shadowhashid = hashid;
				}
			}
			else
			{
				// Un element
				if(Hash[hashid]==Element)
				{
					bInserted = FALSE;
					return	&Hash[hashid];
				}
			}
			hashid=(hashid + hashinc) & hashsize;
		}
	}

	const T		*Search(const T &Element)const
	{
		if(!NbElem) return NULL;
		S32		hashsize=Status->GetSize()-1;
		S32		hashid=Element.HashBase()&hashsize;
		S32		hashinc=Element.HashIncrement();
		if (!(hashinc & 0x1)) hashinc++;

		for(;;)
		{
			// Vide ou Shadow ?
			if(!Status->GetBit(hashid))
			{
				if (Hash[hashid].IsEmpty()) return	NULL;
			}
			else
			{
				T	*ptr = Hash+hashid;
				if(*ptr==Element) return ptr;
			}
			hashid=(hashid + hashinc) & hashsize;
		}
		return	NULL;
	}

	Bool	Suppress (const T &Element)
	{
		if(!NbElem) return FALSE;
		S32	hashsize=Status->GetSize()-1;
		S32	hashid=Element.HashBase()&hashsize;
		S32	hashinc=Element.HashIncrement();
		if (!(hashinc & 0x1)) hashinc++;

		for(;;)
		{
			// Vide ou Shadow ?
			if(!Status->GetBit(hashid))
			{
				if (Hash[hashid].IsEmpty()) return	FALSE;
			}
			else
			{
				T	*ptr = Hash+hashid;
				if(*ptr==Element)
				{
					Status->ClearBit(hashid);
					ptr->SetShadow();
					NbElem--;
					return TRUE;
				}
			}
			hashid=(hashid + hashinc) & hashsize;
		}
		return	FALSE;
	}

	inline	void	InitScan (void) const
	{
		if(!NbElem) ScanId=-1;
		else	ScanId=0;
	}

	inline  T	*NextScan (void) const
	{
		// ScanId Valid ?.
		if(ScanId<0) return	NULL;
		if(ScanId>=Status->GetSize())
		{
			ScanId = -1;
			return	NULL;
		}

		S32	NextScan=Status->FindFirstBit(TRUE,ScanId);

		// Rien ?
		if(NextScan<0)
		{
			ScanId = -1;
			return	NULL;
		}

		// Un element.

		ScanId=NextScan+1;
		return Hash+NextScan;
	}
	inline	S32		GetNbElement(void) const {return	NbElem;}

	friend  ResourceIO_Z & operator >> (ResourceIO_Z &io,HashTableBase_Z<T> &aHT)
	{
		if(io.In())
		{
			S32 Nb=aHT.GetNbElement();
			io >> Nb;
			for (S32 i=0; i<Nb; i++)
			{
				T	hashElt;
				io >> hashElt;
				Bool bResult=aHT.Insert(hashElt);
				UNUSED_Z(bResult);
				ASSERT_Z(bResult);
			}
		}
		if(io.Out())
		{
			S32 Nb=aHT.GetNbElement();
			io >> Nb;
			aHT.InitScan();
			T* hashElt = (T*)aHT.NextScan();
			while(hashElt)
			{
				io >> *hashElt;
				hashElt = (T*)aHT.NextScan();
			}
		}
		return io;
	}

	class const_iterator
	{
	public:
		inline const_iterator() : m_scanId(-1), m_status(NULL), m_hash(NULL)	{}
		inline const_iterator(const S32 _scanID, BitArray_Z * _Status, T * _Hash)
			: m_scanId(_scanID), m_status(_Status), m_hash(_Hash) {
				EXCEPTION_Z(m_scanId < m_status->GetSize());
				if (_scanID != -1)
					m_scanId = m_status->FindFirstBit(TRUE, m_scanId);
		}

		inline const T& operator*() const
		{
			EXCEPTION_Z(m_scanId >= 0 && m_scanId < m_status->GetSize());
			return m_hash[m_scanId];
		}

		inline void operator++()
		{
			EXCEPTION_Z(m_scanId < m_status->GetSize());
			m_scanId++;
			if (m_scanId == m_status->GetSize())
				m_scanId = -1;
			else
				m_scanId = m_status->FindFirstBit(TRUE, m_scanId);
		}

		inline bool operator!=( const const_iterator& _it ) const
		{
			return m_scanId != _it.m_scanId;
		}

		inline bool operator==( const const_iterator& _it ) const
		{
			return m_scanId == _it.m_scanId;
		}

	private:
		S32			m_scanId;
		const	BitArray_Z	*m_status;
		const	T			*m_hash;
	};

	inline const_iterator	Begin() const		{ return const_iterator(0, Status, Hash); }
	inline const_iterator	End()	const		{ return const_iterator(-1, Status, Hash); }

	inline	S32		GetObjectSize()const
	{
		if(!Status)	return	0;
		S32	Size=Status->GetObjectSize()+sizeof(BitArray_Z);	
		Size+=Status->GetSize()*sizeof(T);
		return	Size;
	}
};

typedef HashTableBase_Z<S32Hash_Z>		HashS32Table_Z;
typedef HashTableBase_Z<Name_ZHash_Z>	HashName_ZTable_Z;

#endif //HASH_TABLE_Z_H

