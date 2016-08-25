// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <pch.h>
#include <Math_Z.h>

Float	SignedCalcArea2D( Vec2f *Shape, S32 NbPoints)
{
	Vec2f	*CurPt = Shape;
	Vec2f	*EndPt = CurPt + NbPoints - 2;
	float	Aire;

	// Note sur le fonctionnement.
	//
	// Le dernier points de la Shape n'a pas obligatoirement le m�me que le premier (ne change rien au calcul)
	// mais elle est consid�r� comme FERMEE : On lie le premier au dernier point (si confondu, pas de soucis).
	//
	// Part de la formule http://fr.wikipedia.org/wiki/Aire_et_centre_de_gravit%C3%A9_d'un_polygone
	// Optimisation x 2 : On traite les points 2 par 2 pour �viter les multiplications redondantes.
	// A l'initialisation, on traite le rebouclage du polygone (A cause de l'optim, diff�rent si pair ou impair).

   if (NbPoints & 0x1)
       //Aire = ((Shape->x + EndPt[1].x) * (Shape->y - EndPt[1].y) - (Shape->y + EndPt[1].y) * (Shape->x - EndPt[1].x)) * 0.5f;
	   Aire =  Shape->y*EndPt[1].x - Shape->x*EndPt[1].y;	// Equivalent � ce qu'il y a ci-dessus.
   else
	   Aire = (EndPt[1].x * (Shape->y - EndPt->y) - EndPt[1].y * (Shape->x - EndPt->x));

   // Points au centre.
   while (CurPt < EndPt)
   {
	   register Vec2f	*pPrev = CurPt++;
	   register Vec2f	*pMiddle = CurPt++;
	   Aire += (pMiddle->x * (CurPt->y - pPrev->y) - pMiddle->y * (CurPt->x - pPrev->x));
   }
	return Aire * 0.5f;
}

Float	CalcArea2D(  Vec2f *Shape, S32 NbPoints)
{
	Float Area = SignedCalcArea2D(Shape,NbPoints);
	if (Area < 0.f)  return -Area;
   return Area;
}

void	RotateVector2D(Vec2f &vec, Float Angle)
{ 
    Vec2f tp;

    Vec2f   SC;
    SinCos(SC,Angle);

    tp.x = SC.y*vec.x - SC.x*vec.y;
    tp.y = SC.x*vec.x + SC.y*vec.y;

    vec.x=tp.x;
    vec.y=tp.y;
}
