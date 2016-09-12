// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef __SHAPE_ANALYZER_CHARACTERISTICS_H__
#define __SHAPE_ANALYZER_CHARACTERISTICS_H__

namespace ShapeReco
{
//------------------------------------------------------------------------------------
// SlotCharacteristics
//------------------------------------------------------------------------------------
//
struct SlotCharacteristic
{
public :
    virtual ~SlotCharacteristic() {}
};

namespace SlotCharacteristics
{
enum
{
    AREA_INDICE = 0,
    ALL_POS_INDICE,
    AVG_POS_INDICE,
    RECTANGLE_INDICE,
    SQUARE_INDICE,
    CIRCLE_INDICE,

    COUNT
};

struct Area : public SlotCharacteristic
{
    static const U32 Indice = AREA_INDICE;

    Float m_value;
};

struct AllPos : public SlotCharacteristic
{
    static const U32 Indice = ALL_POS_INDICE;

    Vec3fDA m_poss;
};

struct AvgPos : public SlotCharacteristic
{
    static const U32 Indice = AVG_POS_INDICE;

    Vec3f m_pos;
};

struct Rectangle : public SlotCharacteristic
{
    static const U32 Indice = RECTANGLE_INDICE;

    Float m_similarity;
    Vec3f m_center;
    Vec3f m_lengthDir;
    Float m_length;
    Float m_width;
};

struct Square : public SlotCharacteristic
{
    static const U32 Indice = SQUARE_INDICE;

    Float m_similarity;
    Vec3f m_center;
    Vec3f m_dir;
    Float m_size;
};

struct Circle : public SlotCharacteristic
{
    static const U32 Indice = CIRCLE_INDICE;

    Float m_similarity;
    Vec3f m_center;
    Float m_radius;
};
}

//------------------------------------------------------------------------------------
// ShapeCharacteristics
//------------------------------------------------------------------------------------
//
struct ShapeCharacteristic
{
public :
    virtual ~ShapeCharacteristic() {}
};

namespace ShapeCharacteristics
{
enum
{
    TOTAL_NB_SURFACES_INDICE = 0,
    AVG_POS_INDICE,
    ORIENTATION_INDICE,

    COUNT
};

struct TotalNbSurfaces : public ShapeCharacteristic
{
    static const U32 Indice = TOTAL_NB_SURFACES_INDICE;

    Float m_value;
};

struct AvgPos : public ShapeCharacteristic
{
    static const U32 Indice = AVG_POS_INDICE;

    Vec3f m_pos;
};

struct Orientation : public ShapeCharacteristic
{
    static const U32 Indice = ORIENTATION_INDICE;

    Vec3f m_front;
};
}
}

#endif