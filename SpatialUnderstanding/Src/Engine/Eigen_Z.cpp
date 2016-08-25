// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <pch.h>
#include <Eigen_Z.h>

////////////////////////////////////////////////////////////////////////////////
// File: jacobi_cyclic_method.c                                               //
// Routines:                                                                  //
//    Jacobi_Cyclic_Method                                                    //
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  void Jacobi_Cyclic_Method                                                 //
//            (double eigenvalues[], double *eigenvectors, double *A, int n)  //
//                                                                            //
//  Description:                                                              //
//     Find the eigenvalues and eigenvectors of a symmetric n x n matrix A    //
//     using the Jacobi method. Upon return, the input matrix A will have     //
//     been modified.                                                         //
//     The Jacobi procedure for finding the eigenvalues and eigenvectors of a //
//     symmetric matrix A is based on finding a similarity transformation     //
//     which diagonalizes A.  The similarity transformation is given by a     //
//     product of a sequence of orthogonal (rotation) matrices each of which  //
//     annihilates an off-diagonal element and its transpose.  The rotation   //
//     effects only the rows and columns containing the off-diagonal element  //
//     and its transpose, i.e. if a[i][j] is an off-diagonal element, then    //
//     the orthogonal transformation rotates rows a[i][] and a[j][], and      //
//     equivalently it rotates columns a[][i] and a[][j], so that a[i][j] = 0 //
//     and a[j][i] = 0.                                                       //
//     The cyclic Jacobi method considers the off-diagonal elements in the    //
//     following order: (0,1),(0,2),...,(0,n-1),(1,2),...,(n-2,n-1).  If the  //
//     the magnitude of the off-diagonal element is greater than a treshold,  //
//     then a rotation is performed to annihilate that off-diagnonal element. //
//     The process described above is called a sweep.  After a sweep has been //
//     completed, the threshold is lowered and another sweep is performed     //
//     with the new threshold. This process is completed until the final      //
//     sweep is performed with the final threshold.                           //
//     The orthogonal transformation which annihilates the matrix element     //
//     a[k][m], k != m, is Q = q[i][j], where q[i][j] = 0 if i != j, i,j != k //
//     i,j != m and q[i][j] = 1 if i = j, i,j != k, i,j != m, q[k][k] =       //
//     q[m][m] = cos(phi), q[k][m] = -sin(phi), and q[m][k] = sin(phi), where //
//     the angle phi is determined by requiring a[k][m] -> 0.  This condition //
//     on the angle phi is equivalent to                                      //
//               cot(2 phi) = 0.5 * (a[k][k] - a[m][m]) / a[k][m]             //
//     Since tan(2 phi) = 2 tan(phi) / (1.0 - tan(phi)^2),                    //
//               tan(phi)^2 + 2cot(2 phi) * tan(phi) - 1 = 0.                 //
//     Solving for tan(phi), choosing the solution with smallest magnitude,   //
//       tan(phi) = - cot(2 phi) + sgn(cot(2 phi)) sqrt(cot(2phi)^2 + 1).     //
//     Then cos(phi)^2 = 1 / (1 + tan(phi)^2) and sin(phi)^2 = 1 - cos(phi)^2 //
//     Finally by taking the sqrts and assigning the sign to the sin the same //
//     as that of the tan, the orthogonal transformation Q is determined.     //
//     Let A" be the matrix obtained from the matrix A by applying the        //
//     similarity transformation Q, since Q is orthogonal, A" = Q'AQ, where Q'//
//     is the transpose of Q (which is the same as the inverse of Q).  Then   //
//         a"[i][j] = Q'[i][p] a[p][q] Q[q][j] = Q[p][i] a[p][q] Q[q][j],     //
//     where repeated indices are summed over.                                //
//     If i is not equal to either k or m, then Q[i][j] is the Kronecker      //
//     delta.   So if both i and j are not equal to either k or m,            //
//                                a"[i][j] = a[i][j].                         //
//     If i = k, j = k,                                                       //
//        a"[k][k] =                                                          //
//           a[k][k]*cos(phi)^2 + a[k][m]*sin(2 phi) + a[m][m]*sin(phi)^2     //
//     If i = k, j = m,                                                       //
//        a"[k][m] = a"[m][k] = 0 =                                           //
//           a[k][m]*cos(2 phi) + 0.5 * (a[m][m] - a[k][k])*sin(2 phi)        //
//     If i = k, j != k or m,                                                 //
//        a"[k][j] = a"[j][k] = a[k][j] * cos(phi) + a[m][j] * sin(phi)       //
//     If i = m, j = k, a"[m][k] = 0                                          //
//     If i = m, j = m,                                                       //
//        a"[m][m] =                                                          //
//           a[m][m]*cos(phi)^2 - a[k][m]*sin(2 phi) + a[k][k]*sin(phi)^2     //
//     If i= m, j != k or m,                                                  //
//        a"[m][j] = a"[j][m] = a[m][j] * cos(phi) - a[k][j] * sin(phi)       //
//                                                                            //
//     If X is the matrix of normalized eigenvectors stored so that the ith   //
//     column corresponds to the ith eigenvalue, then AX = X Lamda, where     //
//     Lambda is the diagonal matrix with the ith eigenvalue stored at        //
//     Lambda[i][i], i.e. X'AX = Lambda and X is orthogonal, the eigenvectors //
//     are normalized and orthogonal.  So, X = Q1 Q2 ... Qs, where Qi is      //
//     the ith orthogonal matrix,  i.e. X can be recursively approximated by  //
//     the recursion relation X" = X Q, where Q is the orthogonal matrix and  //
//     the initial estimate for X is the identity matrix.                     //
//     If j = k, then x"[i][k] = x[i][k] * cos(phi) + x[i][m] * sin(phi),     //
//     if j = m, then x"[i][m] = x[i][m] * cos(phi) - x[i][k] * sin(phi), and //
//     if j != k and j != m, then x"[i][j] = x[i][j].                         //
//                                                                            //
//  Arguments:                                                                //
//     double  eigenvalues                                                    //
//        Array of dimension n, which upon return contains the eigenvalues of //
//        the matrix A.                                                       //
//     double* eigenvectors                                                   //
//        Matrix of eigenvectors, the ith column of which contains an         //
//        eigenvector corresponding to the ith eigenvalue in the array        //
//        eigenvalues.                                                        //
//     double* A                                                              //
//        Pointer to the first element of the symmetric n x n matrix A. The   //
//        input matrix A is modified during the process.                      //
//     int     n                                                              //
//        The dimension of the array eigenvalues, number of columns and rows  //
//        of the matrices eigenvectors and A.                                 //
//                                                                            //
//  Return Values:                                                            //
//     Function is of type void.                                              //
//                                                                            //
//  Example:                                                                  //
//     #define N                                                              //
//     double A[N][N], double eigenvalues[N], double eigenvectors[N][N]       //
//                                                                            //
//     (your code to initialize the matrix A )                                //
//                                                                            //
//     Jacobi_Cyclic_Method(eigenvalues, (double*)eigenvectors,               //
//                                                          (double *) A, N); //
////////////////////////////////////////////////////////////////////////////////
//                                                                            //

#define VAL_FLT_EPSILON     2.2204460492503131e-016f//1.0e-4f//1.192092896e-07F        /* smallest such that 1.0+FLT_EPSILON != 1.0 */

void Jacobi_Cyclic_Method(float eigenvalues[], float *eigenvectors,float *A, int n)
{
   int i, j, k, m;
   float *pAk, *pAm, *p_r, *p_e;
   double threshold_norm;
   double threshold;
   double tan_phi, sin_phi, cos_phi, tan2_phi, sin2_phi, cos2_phi;
   double sin_2phi, cos_2phi, cot_2phi;
   double dum1;
   double dum2;
   double dum3;
   double max;

                  // Take care of trivial cases

   if ( n < 1) return;
   if ( n == 1) {
      eigenvalues[0] = *A;
      *eigenvectors = 1.f;
      return;
   }

          // Initialize the eigenvalues to the identity matrix.

   for (p_e = eigenvectors, i = 0; i < n; i++)
      for (j = 0; j < n; p_e++, j++)
         if (i == j) *p_e = 1.f; else *p_e = 0.f;
  
            // Calculate the threshold and threshold_norm.
 
   for (threshold = 0.f, pAk = A, i = 0; i < ( n - 1 ); pAk += n, i++) 
      for (j = i + 1; j < n; j++) threshold += *(pAk + j) * *(pAk + j);
   threshold = sqrt(threshold + threshold);
   threshold_norm = threshold * VAL_FLT_EPSILON;
   max = threshold + 1.f;
   while (threshold > threshold_norm)
   {
//   for (int il=0; il<100; il++) {
      threshold /= 10.f;
      if (max < threshold)
		  continue;
      max = 0.f;
      for (pAk = A, k = 0; k < (n-1); pAk += n, k++) {
         for (pAm = pAk + n, m = k + 1; m < n; pAm += n, m++) {
			double fabspAkpm=fabs(*(pAk + m));
            if ( fabspAkpm < threshold )
				continue;

                 // Calculate the sin and cos of the rotation angle which
                 // annihilates A[k][m].

            cot_2phi = 0.5f * ( *(pAk + k) - *(pAm + m) ) / *(pAk + m);
            dum1 = sqrt( cot_2phi * cot_2phi + 1.f);
            if (cot_2phi < 0.f) dum1 = -dum1;
            tan_phi = -cot_2phi + dum1;
            tan2_phi = tan_phi * tan_phi;
            sin2_phi = tan2_phi / (1.f + tan2_phi);
            cos2_phi = 1.f - sin2_phi;
            sin_phi = sqrt(sin2_phi);
            if (tan_phi < 0.f) sin_phi = - sin_phi;
            cos_phi = sqrt(cos2_phi); 
            sin_2phi = 2.f * sin_phi * cos_phi;
            cos_2phi = cos2_phi - sin2_phi;

                     // Rotate columns k and m for both the matrix A 
                     //     and the matrix of eigenvectors.

            p_r = A;
            dum1 = *(pAk + k);
            dum2 = *(pAm + m);
            dum3 = *(pAk + m);
            *(pAk + k) = dum1 * cos2_phi + dum2 * sin2_phi + dum3 * sin_2phi;
            *(pAm + m) = dum1 * sin2_phi + dum2 * cos2_phi - dum3 * sin_2phi;
            *(pAk + m) = 0.f;
            *(pAm + k) = 0.f;
            for (i = 0; i < n; p_r += n, i++) {
               if ( (i == k) || (i == m) ) continue;
               if ( i < k ) dum1 = *(p_r + k); else dum1 = *(pAk + i);
               if ( i < m ) dum2 = *(p_r + m); else dum2 = *(pAm + i);
               dum3 = dum1 * cos_phi + dum2 * sin_phi;
               if ( i < k ) *(p_r + k) = dum3; else *(pAk + i) = dum3;
               dum3 = - dum1 * sin_phi + dum2 * cos_phi;
               if ( i < m ) *(p_r + m) = dum3; else *(pAm + i) = dum3;
            }
            for (p_e = eigenvectors, i = 0; i < n; p_e += n, i++) {
               dum1 = *(p_e + k);
               dum2 = *(p_e + m);
               *(p_e + k) = dum1 * cos_phi + dum2 * sin_phi;
               *(p_e + m) = - dum1 * sin_phi + dum2 * cos_phi;
            }
         }
         for (i = 0; i < n; i++)
            if ( i == k ) continue;
            else if ( max < fabs(*(pAk + i))) max = fabs(*(pAk + i));
      }
   }
   for (pAk = A, k = 0; k < n; pAk += n, k++) eigenvalues[k] = *(pAk + k); 
}

void GenericEigen3D(const Vec3f *vectors,S32 nbvectors,Vec3f eigenvectors[3],Float eigenvalues[3])
{
	int		i;

	if (nbvectors<1)
		return;

	Vec3f	BaryCentre=VEC4F_NULL;

	// Calculate mean (barycentre)
	for (i=0; i<nbvectors; i++)
	{
		BaryCentre+=vectors[i];
	}

	BaryCentre/=(Float)nbvectors;

	Float	MatCovariance[3][3];
	// Compute cloud point directing vector
	// 1st compute covariance matrix
	for (int x=0; x<3; x++)
	{
		for (int y=0; y<3; y++)
		{
			Float	totalcovariance=0.f;
			for (i=0; i<nbvectors; i++)
			{
				Float	valx=(Float)(((Float*)&vectors[i])[x]);
				Float	valy=(Float)(((Float*)&vectors[i])[y]);
				valx-=((Float*)&BaryCentre)[x];
				valy-=((Float*)&BaryCentre)[y];
				Float	covariance=valx*valy;
				totalcovariance+=covariance;
			}
			Float	covariance=totalcovariance/nbvectors;
			MatCovariance[x][y]=covariance;
		}
	}
	// Compute EigenVectors
	Jacobi_Cyclic_Method(eigenvalues,(float *)eigenvectors,(Float*)&MatCovariance,3);
	
	Swap(eigenvectors[0].y, eigenvectors[1].x);
	Swap(eigenvectors[0].z, eigenvectors[2].x);
	Swap(eigenvectors[1].z, eigenvectors[2].y);
}
