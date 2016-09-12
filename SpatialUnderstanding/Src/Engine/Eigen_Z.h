// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef _EIGEN_Z_H
#define _EIGEN_Z_H

#include <BeginDef_Z.h>

void Jacobi_Cyclic_Method(float eigenvalues[], float *eigenvectors,float *A, int n);
void GenericEigen3D(const Vec3f *vectors,S32 nbvectors,Vec3f eigenvectors[3],Float eigenvalues[3]);

#endif //_EIGEN_Z_H