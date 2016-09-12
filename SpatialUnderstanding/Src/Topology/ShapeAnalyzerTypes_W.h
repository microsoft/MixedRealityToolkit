// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef __SHAPE_ANALYZER_TYPES_H__
#define __SHAPE_ANALYZER_TYPES_H__

#include <NonCopyable_Z.h>

namespace ShapeReco
{
class ShapeAnalyzer_W;

//------------------------------------------------------------------------------------
// Slot
//------------------------------------------------------------------------------------
// A slot is a set of surface which satisfy some constraints
// During the analysis some characteristics can be assigned to the slot (see ShapeAnalyzerCharacteristics_W.h)
class Slot : public NonCopyable_Z
{
public:
    void ResetCharacteristics()
    {
        for (S32 i = 0; i < m_characteristics.GetSize(); ++i)
            m_characteristics[i].Reset();
    }

    template <class T> Bool Have() const
    {
        SHAPE_ANALYZER_ASSERT(T::Indice < m_characteristics.GetSize());

        return m_characteristics[T::Indice].Get() != NULL;
    }

    template <class T> const T& Get() const
    {
        SHAPE_ANALYZER_ASSERT(Have<T>());

        return *reinterpret_cast<const T*>(m_characteristics[T::Indice].Get());
    }

    template <class T> T& Create() const // m_characteristics is a kind of cache so this method is const
    {
        SHAPE_ANALYZER_ASSERT(!Have<T>());

        m_characteristics[T::Indice] = New_Z T();
        return *reinterpret_cast<T*>(m_characteristics[T::Indice].Get());
    }

public: //private:
    Name_Z m_name;
    Color m_debugColor;
    DynArray_Z<const TopologyAnalyzer_W::Surface*> m_surfaces;

    mutable SafeArray_Z<SharedPtr<SlotCharacteristic>, SlotCharacteristics::COUNT> m_characteristics; // m_characteristics is a kind of cache so mutable is ok ?
};

//------------------------------------------------------------------------------------
// Shape
//------------------------------------------------------------------------------------
// A shape is a set of slot which satisfy some constraints
// During the analysis some characteristics can be assigned to the shape (see ShapeAnalyzerCharacteristics_W.h)
class Shape : public NonCopyable_Z
{
public :
    void ResetCharacteristics()
    {
        for (S32 i = 0; i < m_characteristics.GetSize(); ++i)
            m_characteristics[i].Reset();
    }

    template <class T> Bool Have() const
    {
        SHAPE_ANALYZER_ASSERT(T::Indice < m_characteristics.GetSize());

        return m_characteristics[T::Indice].Get() != NULL;
    }

    template <class T> const T& Get() const
    {
        SHAPE_ANALYZER_ASSERT(Have<T>());

        return *reinterpret_cast<const T*>(m_characteristics[T::Indice].Get());
    }

    template <class T> T& Create() const // m_characteristics is a kind of cache so this method is const
    {
        SHAPE_ANALYZER_ASSERT(!Have<T>());

        m_characteristics[T::Indice] = New_Z T();
        return *reinterpret_cast<T*>(m_characteristics[T::Indice].Get());
    }

public: //private:
    Name_Z m_name;
    const DynArray_Z<const TopologyAnalyzer_W::Surface*>* m_originalGroup; // All connected surfaces whose shape is originated
    
    DynArray_Z<Slot> m_slots;

    mutable SafeArray_Z<SharedPtr<ShapeCharacteristic>, ShapeCharacteristics::COUNT> m_characteristics; // This is a kind of cache so mutable is ok ?
};

//------------------------------------------------------------------------------------
// SlotConstraintImpl
//------------------------------------------------------------------------------------
// Implementation of a constraint which must be satisfy by a slot
// Each constraint on a slot must inherit from this class
class SlotConstraintImpl : public NonCopyable_Z
{
public:
    virtual ~SlotConstraintImpl() {}
    
    virtual SlotConstraintImpl* clone() const = 0;

    virtual Bool TryMatchWith(Slot& _slot, Shape& _shape, const ShapeAnalyzer_W& _shapeAnalyzer) const = 0;
};

//------------------------------------------------------------------------------------
// SlotConstraint
//------------------------------------------------------------------------------------
// A constraint which must be satisfy by a slot
// This class is use to manage simply polymorphic constraints
class SlotConstraint : public CloneablePtr<SlotConstraintImpl>
{
public:
    SlotConstraint() {} // For use in vector
    SlotConstraint(const SlotConstraintImpl& _constraintImpl) : CloneablePtr<SlotConstraintImpl>(_constraintImpl) {} // For implicit convertion

    Bool TryMatchWith(Slot& _slot, Shape& _shape, const ShapeAnalyzer_W& _shapeAnalyzer) const
    {
        SHAPE_ANALYZER_ASSERT(m_ptr != NULL);
        return m_ptr->TryMatchWith(_slot, _shape, _shapeAnalyzer);
    }
};

//------------------------------------------------------------------------------------
// SlotDesc
//------------------------------------------------------------------------------------
// Description of a slot
// The descriptions contains all the constraints that must be satisfy by the slot
class SlotDesc
{
public:
    SlotDesc()
    {
		m_debugColor = COLOR_NULL;
    }
	~SlotDesc() {};

	void	SetName(const Name_Z& _name)
	{
		m_name = _name;
	}
    void AddConstraint(const SlotConstraint& _constraint)
    {
        m_constraints.Add(_constraint);
    }
  
public: //private:
    Name_Z m_name;
    Color m_debugColor;

    DynArray_Z<SlotConstraint> m_constraints;

    // For optimization
    struct Infos
    {
        Bool m_ok;
        SafeArray_Z<SharedPtr<SlotCharacteristic>, SlotCharacteristics::COUNT> m_characteristics;
        S32 m_nbUse;
    };

    mutable AVL<DynArray_Z<const TopologyAnalyzer_W::Surface*>, Infos> m_savedInfos;
};

//------------------------------------------------------------------------------------
// ShapeConstraintImpl
//------------------------------------------------------------------------------------
// Implementation of a constraint which must be satisfy by a shape
// Each constraint on a shape must inherit from this class
class ShapeConstraintImpl : public NonCopyable_Z
{
public:
    virtual ~ShapeConstraintImpl() {}
    
    virtual ShapeConstraintImpl* clone() const = 0;

    virtual Bool TryMatchWith(Shape& _shape, const ShapeAnalyzer_W& _shapeAnalyzer) const = 0;
};

//------------------------------------------------------------------------------------
// ShapeConstraint
//------------------------------------------------------------------------------------
// A constraint which must be satisfy by a shape
// This class is use to manage simply polymorphic constraints
class ShapeConstraint : public CloneablePtr<ShapeConstraintImpl>
{
public:
    ShapeConstraint() {} // For use in vector
    ShapeConstraint(const ShapeConstraintImpl& _constraintImpl) : CloneablePtr<ShapeConstraintImpl>(_constraintImpl) {} // For implicit convertion

    Bool TryMatchWith(Shape& _shape, const ShapeAnalyzer_W& _shapeAnalyzer) const
    {
        SHAPE_ANALYZER_ASSERT(m_ptr != NULL);
        return m_ptr->TryMatchWith(_shape, _shapeAnalyzer);
    }
};

//------------------------------------------------------------------------------------
// ShapeDesc
//------------------------------------------------------------------------------------
// Description of a slot
// The descriptions contains the descriptors of the slots that must compose the shape and all the constraints that must be satisfy by the shape
class ShapeDesc
{
public:
	~ShapeDesc() {}
	void	SetName(const Name_Z& _name)
	{
		m_name = _name;
	}

	void AddSlot(const SlotDesc& _slotDesc)
    {
#ifdef _DEBUG
        for (S32 i = 0; i < m_slotDescs.GetSize(); ++i)
            SHAPE_ANALYZER_ASSERT(m_slotDescs[i].m_name != _slotDesc.m_name);
#endif

        m_slotDescs.Add(_slotDesc);
    }

    void AddConstraint(const ShapeConstraint& _constraint)
    {
        m_constraints.Add(_constraint);
    }

public: //private:
    Name_Z m_name;

    DynArray_Z<SlotDesc> m_slotDescs;
    DynArray_Z<ShapeConstraint> m_constraints;
};
} // namespace ShapeReco

#endif