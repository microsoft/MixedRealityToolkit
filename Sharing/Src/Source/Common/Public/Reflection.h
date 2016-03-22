//////////////////////////////////////////////////////////////////////////
// Reflection.h
//
// Provides the interface for enabling reflection on XTools types.
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#if defined(SWIG)
#define XTOOLS_REFLECTION_DECLARE(typeName)\
	private:

#else

#include <Public/Environment.h>

#include <string>
#include <list>
#include <type_traits>

XTOOLS_NAMESPACE_BEGIN

template<typename T>
struct stripped_type
{
	typedef typename std::remove_const <
		typename std::remove_reference <
			typename std::remove_pointer<T>::type
		> ::type
	> ::type type;
};

NAMESPACE_BEGIN(Reflection)

struct ToBase
{
	virtual void* operator()(void* derived) = 0;
	virtual const void* operator()(const void* derived) = 0;
};

template<typename DerivedType, typename BaseType>
struct ToBaseT : public ToBase
{
	virtual const void* operator()(const void* derived)
	{
		return static_cast<const BaseType*>(reinterpret_cast<const DerivedType*>(derived));
	}

	virtual void* operator()(void* derived)
	{
		return static_cast<BaseType*>(reinterpret_cast<DerivedType*>(derived));
	}
};

//////////////////////////////////////////////////////////////////////////
// Provides type information useful for reflection.
class TypeInfo
{
public:
	TypeInfo(std::string typeName);

	// Returns the integer used to uniquely identify this type.
	int GetID() const;

	// Returns the name of this type.
	std::string GetName() const;

	// Returns true if this type inherits from the type with the given ID
	bool IsDerivedFrom(int baseTypeID) const;

protected:
	void* CastToBase(void* derived, int baseTypeID) const;
	const void* CastToBase(const void* derived, int baseTypeID) const;

	// Returns the next available type ID, and then increments.
	static int GetNextTypeID();

	struct BaseClassInfo
	{
		const TypeInfo* m_typeInfo;
		ToBase*			m_baseCaster;
	};

	int							m_typeID;
	std::string					m_typeName;
	std::list<BaseClassInfo>	m_baseTypes;
};

NAMESPACE_END(Reflection)

#pragma region Operators

template<typename T, typename C>
bool IsA(C obj)
{
    return ::XTools::get_pointer(obj)->GetTypeInfo().GetID() == T::MyTypeInfo().GetID();
}


template<typename T, typename C>
bool IsFrom(C obj)
{
    return ::XTools::get_pointer(obj)->GetTypeInfo().IsDerivedFrom(T::MyTypeInfo().GetID());
}


template<typename T, typename C>
T* reflection_cast(C obj)
{
    return reinterpret_cast<T*>(::XTools::get_pointer(obj)->CastToBase(T::MyTypeInfo().GetID()));
}

#pragma endregion

NAMESPACE_BEGIN(Reflection)

template<typename T>
class TypeInfoT : public TypeInfo
{
public:
	typedef TypeInfoT<T> ThisType;

	static ThisType CreateTypeInfo(std::string name)
	{
		return ThisType(name);
	}

	TypeInfoT(std::string typeName) : TypeInfo(typeName) {}

	template<typename B>
	ThisType& BaseClass()
	{
		BaseClassInfo info;
		info.m_typeInfo = &B::MyTypeInfo();
		info.m_baseCaster = new ToBaseT<T, B>();

		m_baseTypes.push_back(info);
		return *this;
	}

	void* CastTo(T* derived, int baseTypeInfo)
	{
		// Make sure this is the end of the inheritance chain
		XTASSERT(IsA<T>(derived));
		return CastToBase(derived, baseTypeInfo);
	}

	const void* CastTo(const T* derived, int baseTypeInfo)
	{
		// Make sure this is the end of the inheritance chain
		XTASSERT(IsA<T>(derived));
		return CastToBase(derived, baseTypeInfo);
	}
};


#pragma region Macros

#define TYPEINFO_MEMBERNAME(typeName) m_typeInfo##typeName

#define XTOOLS_REFLECTION_DECLARE(typeName)\
private:\
	static ::XTools::Reflection::TypeInfoT<typeName>	TYPEINFO_MEMBERNAME(typeName);\
public:\
	static const ::XTools::Reflection::TypeInfo& MyTypeInfo() { return TYPEINFO_MEMBERNAME(typeName); }\
	virtual const ::XTools::Reflection::TypeInfo& GetTypeInfo() const { return TYPEINFO_MEMBERNAME(typeName); }\
	virtual void* CastToBase(int typeID); \
	virtual const void* CastToBase(int typeID) const; \
private:

#define XTOOLS_REFLECTION_DEFINE(typeName)\
	void* typeName::CastToBase(int typeID)\
	{\
		return TYPEINFO_MEMBERNAME(typeName).CastTo(this, typeID);\
	}\
	const void* typeName::CastToBase(int typeID) const\
	{\
		return TYPEINFO_MEMBERNAME(typeName).CastTo(this, typeID);\
	}\
	::XTools::Reflection::TypeInfoT<typeName> typeName::TYPEINFO_MEMBERNAME(typeName) = ::XTools::Reflection::TypeInfoT<typeName>::CreateTypeInfo(#typeName)


#pragma endregion


//////////////////////////////////////////////////////////////////////////
// This should be the base class for all types using Reflection.
class XTObject
{
public:
	virtual ~XTObject() {}

	XTOOLS_REFLECTION_DECLARE(XTObject)
};

NAMESPACE_END(Reflection)
XTOOLS_NAMESPACE_END

#endif // defined(SWIG)
