/* (C) 2013-2015, The Regents of The University of Michigan
All rights reserved.

This software may be available under alternative licensing
terms. Contact Edwin Olson, ebolson@umich.edu, for more information.

   Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.
 */

#include <math.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

/** SVD 2x2.

    A = USV'

    U = [ cos(theta) -sin(theta) ]
    [ sin(theta)  cos(theta) ]

    S = [ e  0 ]
    [ 0  f ]

    V = [ cos(phi)   -sin(phi) ]
    [ sin(phi)   cos(phi)  ]


    Note that A'A = VS'U'USV' = VSSV', which we can expand. (This
    operation lets us just worry about phi, without having to think
    about theta simultaneously.) Let s = sin(phi), c = cos(phi):

    A'A = [ w x ] = [ e^2c^2 + f^2s^2      e^2sc - f^2sc  ]
    [ x y ]   [ e^2sc - f^2sc       e^2s^2 + f^2c^2 ]

    This gives us simultaneous equations:

    e^2c^2 + f^2s^2 = w    (1)
    e^2sc - f^2sc   = x    (2)
    e^2s^2 + f^2c^2 = y    (3)

    e^2 + f^2              = w + y   (1 + 3)
    (e^2 - f^2)(c^2 - s^2) = w - y   (1 - 3)
    (e^2 - f^2)sc          = x       (2)

    (e^2 - f^2)cos(2phi)   = w - y
    (e^2 - f^2)sin(2phi)   = 2x

    tan(2phi) = 2x / (w - y)

    /////////////////////////

    Similarly, we obtain for theta:

    AA' = USV'VSU' = USS'U'

    AA' = [ w' x' ] (though these are different w,x,y than for phi)
    [ x' y' ]

    tan(2theta) = 2x' / (w' - y')

    ////////////////////////

    We now recover S. We could first recover e^2 and f^2, from which
    we might be tempted into concluding that S = diag([e f]). But note
    that all of our math has been in terms of S'S, and there are
    values of S such that S'S = diag[e^2 f^2] even when S does not
    equal diag([e f]). We have done nothing to prevent such a case.

    In particular, this is possible when S can be written S = WD, S'S
    = D'W'WD = D'D. We can trivially recover WD by starting with the
    definition of SVD and using our U and V matrices already
    recovered. A=U(WD)V', thus WD = U'AV. We can factor WD into W and
    D, then incorporate the W term into U in order to obtain our SVD.

    (Consider the example case A = [12 13; 13 -12].) A'A=AA' =
    diag([313 313]). This will yield theta=phi=0, and thus WD=A (and
    is clearly not diagonal!).


    We then solve for the "singular values matrix" with WS = U'AV. WS
    is often equal to S, except in the case that Sx */

void svd22(const double A[4], double U[4], double S[2], double V[4])
{
    double w, x, y;

    // compute V's phi from A'A
    w = A[0]*A[0] + A[2]*A[2];
    x = A[0]*A[1] + A[2]*A[3];
    y = A[1]*A[1] + A[3]*A[3];

    double phi = 0.5*atan2(2*x, w - y);

    // compute U's theta from AA'
    w = A[0]*A[0] + A[1]*A[1];
    x = A[0]*A[2] + A[1]*A[3];
    y = A[2]*A[2] + A[3]*A[3];

    double theta = 0.5*atan2(2*x, w - y);

    // Now we'll actually construct U and V (which may be modified
    // later...) For now, they're pure rotations.
    double ct = cos(theta), st = sin(theta);

    U[0] = ct;
    U[1] = -st;
    U[2] = st;
    U[3] = ct;

    double cp = cos(phi), sp = sin(phi);

    V[0] = cp;
    V[1] = -sp;
    V[2] = sp;
    V[3] = cp;

    // Solve for the "singular value matrix" WS... if UWSV' = A, then
    // WS = U'AV.

    // WS = U'*A*V
    double T[4] = { A[0]*V[0] + A[1]*V[2],    // T = A*V
                    A[0]*V[1] + A[1]*V[3],
                    A[2]*V[0] + A[3]*V[2],
                    A[2]*V[1] + A[3]*V[3] };

    double WS[4] = { U[0]*T[0] + U[2]*T[2],   // WS = U'*T = U'*A*V
                     U[0]*T[1] + U[2]*T[3],
                     U[1]*T[0] + U[3]*T[2],
                     U[1]*T[1] + U[3]*T[3] };

    S[0] = sqrt(WS[0]*WS[0] + WS[1]*WS[1]);
    S[1] = sqrt(WS[2]*WS[2] + WS[3]*WS[3]);

    // Solve for W from WS, being careful to handle singular cases
    // such that W is unitary.
    double eps = 1E-6;
    double W[4];

    if (S[0] > eps) {
        W[0] = WS[0] / S[0];
        W[2] = WS[2] / S[0];
    } else {
        W[0] = 1;
        W[2] = 0;
    }

    if (S[1] > eps) {
        W[1] = WS[1] / S[1];
        W[3] = WS[3] / S[1];
    } else {
        W[1] = 0;
        W[3] = 1;
    }

    // updated U = UW
    double UW[4] = { U[0]*W[0] + U[1]*W[2],
                     U[0]*W[1] + U[1]*W[3],
                     U[2]*W[0] + U[3]*W[2],
                     U[2]*W[1] + U[3]*W[3] };

    memcpy(U, UW, 4*sizeof(double));

    assert(S[0] >= 0);
    assert(S[1] >= 0);

    // sort singular values.
    if (fabs(S[1]) > fabs(S[0])) {
        // Curiously, this code never seems to get invoked.  Why is it
        // that S[0] always ends up the dominant vector?  However,
        // this code has been tested (flipping the logic forces us to
        // sort the singular values in ascending order).
        //
        // P = [ 0 1 ; 1 0 ]
        // USV' = (UP)(PSP)(PV')
        //      = (UP)(PSP)(VP)'
        //      = (UP)(PSP)(P'V')'
        double t = S[0];
        S[0] = S[1];
        S[1] = t;

        // exchange columns of U and V
        double tmp[2];
        tmp[0] = U[0];
        tmp[1] = U[2];
        U[0] = U[1];
        U[2] = U[3];
        U[1] = tmp[0];
        U[3] = tmp[1];

        tmp[0] = V[0];
        tmp[1] = V[2];
        V[0] = V[1];
        V[2] = V[3];
        V[1] = tmp[0];
        V[3] = tmp[1];
    }
}
