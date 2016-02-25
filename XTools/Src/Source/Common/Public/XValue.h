//////////////////////////////////////////////////////////////////////////
// XValue.h
//
// Convenient class for holding an arbitrary value
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class XValue
{
public:
	enum Type : byte
	{
		Unknown = 0,
		Int,
		UInt,
		Int64,
		Float,
		Double,
		String
	};

	XValue() {}

	template<typename T>
	XValue(const T& value)
	{
		static_assert(ValueMapper<T>::value != Unknown, "Cannot use XValue with undefined types");
		m_wrappedValue = new ValueT<T>(value);
	}

#if !defined(XTOOLS_PLATFORM_OSX)
	template<>
	XValue(const XStringPtr& value)
	{
		m_wrappedValue = new ValueT<std::string>(value->GetString());
	}
#endif

	bool IsEmpty() const { return (m_wrappedValue == NULL); }

	Type GetType() const { return (m_wrappedValue) ? m_wrappedValue->GetType() : Unknown; }

	// Get the value.  Returns NULL if you attempt to get the wrong type
	template<typename T>
	const T* Get() const
	{
		if (m_wrappedValue && ValueMapper<T>::value == m_wrappedValue->GetType())
		{
			return static_cast<ValueT<T>*>(m_wrappedValue.get())->Get();
		}
		else
		{
			return NULL;
		}
	}

	void Serialize(const NetworkOutMessagePtr& msg) const
	{
		const Type myType = GetType();
		msg->Write((byte)myType);

		if (myType != Unknown)
		{
			m_wrappedValue->Serialize(msg);
		}
	}

	void Deserialize(NetworkInMessage& msg);

	bool Equals(const XValue& other) const
	{
		return (
			((m_wrappedValue == nullptr) == (other.m_wrappedValue == nullptr)) && 
			( m_wrappedValue == nullptr || m_wrappedValue->Equals(other.m_wrappedValue.get()))
			);
	}

	std::string ToString() const;

private:
	template<typename T> struct ValueMapper			{ static const Type value = Unknown; };
	
	template<Type T> struct TypeMapper				{ typedef void ValueType; };

#if !defined(XTOOLS_PLATFORM_OSX)
	template<> struct ValueMapper < int32 >			{ static const Type value = Int; };
	template<> struct ValueMapper < uint32 >		{ static const Type value = UInt; };
	template<> struct ValueMapper < int64 >			{ static const Type value = Int64; };
	template<> struct ValueMapper < float >			{ static const Type value = Float; };
	template<> struct ValueMapper < double >		{ static const Type value = Double; };
	template<> struct ValueMapper < std::string >	{ static const Type value = String; };

	template<> struct TypeMapper<Int>				{ typedef int32 ValueType; };
	template<> struct TypeMapper<UInt>				{ typedef uint32 ValueType; };
	template<> struct TypeMapper<Int64>				{ typedef int64 ValueType; };
	template<> struct TypeMapper<Float>				{ typedef float ValueType; };
	template<> struct TypeMapper<Double>			{ typedef double ValueType; };
	template<> struct TypeMapper<String>			{ typedef std::string ValueType; };
#endif
	
	class ValueWrapper : public RefCounted
	{
	public:
		virtual Type GetType() const = 0;
		virtual void Serialize(const NetworkOutMessagePtr& msg) const = 0;
		virtual bool Equals(const ValueWrapper* other) const = 0;
	};

	template<typename T>
	class ValueT : public ValueWrapper
	{
	public:
		ValueT(const T& value) : m_value(value) {}
		
		virtual Type GetType() const { return ValueMapper<T>::value; }
		
		virtual void Serialize(const NetworkOutMessagePtr& msg) const { msg->Write(m_value); }

		virtual bool Equals(const ValueWrapper* other) const
		{
			return (static_cast<const ValueT<T>*>(other)->m_value == m_value);
		}

		const T* Get() const { return &m_value; }
	private:
		T m_value;
	};

	ref_ptr<ValueWrapper> m_wrappedValue;
};

#if defined(XTOOLS_PLATFORM_OSX)
template<>
inline XValue::XValue(const XStringPtr& value)
{
	m_wrappedValue = new ValueT<std::string>(value->GetString());
}

template<> struct XValue::ValueMapper < int32 >			{ static const Type value = Int; };
template<> struct XValue::ValueMapper < uint32 >		{ static const Type value = UInt; };
template<> struct XValue::ValueMapper < int64 >			{ static const Type value = Int64; };
template<> struct XValue::ValueMapper < float >			{ static const Type value = Float; };
template<> struct XValue::ValueMapper < double >		{ static const Type value = Double; };
template<> struct XValue::ValueMapper < std::string >	{ static const Type value = String; };

template<> struct XValue::TypeMapper< XValue::Type::Int >		{ typedef int32 ValueType; };
template<> struct XValue::TypeMapper< XValue::Type::UInt >      { typedef uint32 ValueType; };
template<> struct XValue::TypeMapper< XValue::Type::Int64 >		{ typedef int64 ValueType; };
template<> struct XValue::TypeMapper< XValue::Type::Float >     { typedef float ValueType; };
template<> struct XValue::TypeMapper< XValue::Type::Double >	{ typedef double ValueType; };
template<> struct XValue::TypeMapper< XValue::Type::String >	{ typedef std::string ValueType; };
#endif

inline bool operator==(const XValue& lhs, const XValue& rhs)
{
	return lhs.Equals(rhs);
}

XTOOLS_NAMESPACE_END
