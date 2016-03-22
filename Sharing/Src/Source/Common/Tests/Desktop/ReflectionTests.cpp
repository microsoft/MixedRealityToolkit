#include "stdafx.h"
#include <Tests\Utilities.h>

using namespace XTools;
using namespace XTools::Reflection;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

// Disable warning about LINE_INFO() "nonstandard extension used : class rvalue used as lvalue"
#pragma warning( disable : 4238 )

namespace CommonDesktopTests
{
	// Inheritance structure ///////////////////////////////////
	//   A
	//     Aa
	//     Ab
	//       Ab1 --o Y
	//     Ac --o Z
	//       Ac1 --o Y
	//   B

	class Y XTABSTRACT
	{
		XTOOLS_REFLECTION_DECLARE(Y);
	};

	class Z XTABSTRACT
	{
		XTOOLS_REFLECTION_DECLARE(Z);
	};

	class A : public XTObject
	{
		XTOOLS_REFLECTION_DECLARE(A);
	};

	class B : public XTObject
	{
		XTOOLS_REFLECTION_DECLARE(B);
	};

	class Aa : public A
	{
		XTOOLS_REFLECTION_DECLARE(Aa);
	};

	class Ab : public A
	{
		XTOOLS_REFLECTION_DECLARE(Ab);
	};

	class Ab1 : public Ab, public Y
	{
		XTOOLS_REFLECTION_DECLARE(Ab1);
	};

	class Ac : public A, public Z
	{
		XTOOLS_REFLECTION_DECLARE(Ac);
	};

	class Ac1 : public Ac, public Y
	{
		XTOOLS_REFLECTION_DECLARE(Ac1);
	};

	XTOOLS_REFLECTION_DEFINE(Y);
	XTOOLS_REFLECTION_DEFINE(Z);
	XTOOLS_REFLECTION_DEFINE(A).BaseClass<XTObject>();
	XTOOLS_REFLECTION_DEFINE(B).BaseClass<XTObject>();
	XTOOLS_REFLECTION_DEFINE(Aa).BaseClass<A>();
	XTOOLS_REFLECTION_DEFINE(Ab).BaseClass<A>();
	XTOOLS_REFLECTION_DEFINE(Ab1).BaseClass<Ab>().BaseClass<Y>();
	XTOOLS_REFLECTION_DEFINE(Ac).BaseClass<A>().BaseClass<Z>();
	XTOOLS_REFLECTION_DEFINE(Ac1).BaseClass<Ac>().BaseClass<Y>();

	TEST_CLASS(ReflectionTests)
	{
		TEST_METHOD(TestReflectionCast)
		{
			XTObject* x = new XTObject();
			XTObject* a = new A();
			XTObject* b = new B();
			XTObject* aa = new Aa();
			XTObject* ab = new Ab();
			XTObject* ac = new Ac();
			XTObject* ab1 = new Ab1();
			XTObject* ac1 = new Ac1();


			const XTObject* const_x = new XTObject();
			const XTObject* const_a = new A();
			const XTObject* const_b = new B();
			const XTObject* const_aa = new Aa();
			const XTObject* const_ab = new Ab();
			const XTObject* const_ac = new Ac();
			const XTObject* const_ab1 = new Ab1();
			const XTObject* const_ac1 = new Ac1();

			// Test that self-type casting works correctly
			Assert::IsNotNull(reflection_cast<XTObject>(x));
			Assert::IsNotNull(reflection_cast<A>(a));
			Assert::IsNotNull(reflection_cast<B>(b));
			Assert::IsNotNull(reflection_cast<Aa>(aa));
			Assert::IsNotNull(reflection_cast<Ab>(ab));
			Assert::IsNotNull(reflection_cast<Ab1>(ab1));
			Assert::IsNotNull(reflection_cast<Ac>(ac));
			Assert::IsNotNull(reflection_cast<Ac1>(ac1));

			// Casts to XTObject succeed
			Assert::IsNotNull(reflection_cast<XTObject>(a));
			Assert::IsNotNull(reflection_cast<XTObject>(b));
			Assert::IsNotNull(reflection_cast<XTObject>(aa));
			Assert::IsNotNull(reflection_cast<XTObject>(ab));
			Assert::IsNotNull(reflection_cast<XTObject>(ab1));
			Assert::IsNotNull(reflection_cast<XTObject>(ac));
			Assert::IsNotNull(reflection_cast<XTObject>(ac1));

			// Casts between unrelated types fail
			Assert::IsNull(reflection_cast<A>(b));
			Assert::IsNull(reflection_cast<B>(a));
			Assert::IsNull(reflection_cast<B>(aa));
			Assert::IsNull(reflection_cast<B>(ab));
			Assert::IsNull(reflection_cast<Aa>(ab));
			Assert::IsNull(reflection_cast<Ab>(aa));
			Assert::IsNull(reflection_cast<Ab1>(ac1));
			Assert::IsNull(reflection_cast<Ac1>(ab1));

			// Casts from a derived type to a base type succeed
			Assert::IsNotNull(reflection_cast<A>(aa));
			Assert::IsNotNull(reflection_cast<A>(ab));
			Assert::IsNotNull(reflection_cast<A>(ac));
			Assert::IsNotNull(reflection_cast<A>(ab1));
			Assert::IsNotNull(reflection_cast<A>(ac1));

			// Casts from a type to an interface (multiple inheritance) succeed
			Assert::IsNotNull(reflection_cast<Z>(ac));
			Assert::IsNotNull(reflection_cast<Z>(ac1));
			Assert::IsNotNull(reflection_cast<Y>(ab1));
			Assert::IsNotNull(reflection_cast<Y>(ac1));

			// Casts from a base type to a derived type fail
			Assert::IsNull(reflection_cast<Aa>(a));
			Assert::IsNull(reflection_cast<Ab>(a));

			// Casting works for const pointers
			Assert::IsNotNull(reflection_cast<const XTObject>(const_a));
			Assert::IsNotNull(reflection_cast<const XTObject>(const_b));
			Assert::IsNotNull(reflection_cast<const XTObject>(const_aa));
			Assert::IsNotNull(reflection_cast<const XTObject>(const_ab));
			Assert::IsNotNull(reflection_cast<const XTObject>(const_ab1));
			Assert::IsNotNull(reflection_cast<const XTObject>(const_ac));
			Assert::IsNotNull(reflection_cast<const XTObject>(const_ac1));

			delete x;
			delete a;
			delete b;
			delete aa;
			delete ab;
			delete ac;
			delete ab1;
			delete ac1;

			delete const_x;
			delete const_a;
			delete const_b;
			delete const_aa;
			delete const_ab;
			delete const_ac;
			delete const_ab1;
			delete const_ac1;
		}

		TEST_METHOD(TestReflectionIsA)
		{
			XTObject* x = new XTObject();
			XTObject* a = new A();
			XTObject* b = new B();
			XTObject* aa = new Aa();
			XTObject* ab = new Ab();
			XTObject* ac = new Ac();
			XTObject* ab1 = new Ab1();
			XTObject* ac1 = new Ac1();

			// Objects should only be IsA to themselves
			Assert::IsTrue(IsA<XTObject>(x));
			Assert::IsTrue(IsA<A>(a));
			Assert::IsTrue(IsA<B>(b));
			Assert::IsTrue(IsA<Aa>(aa));
			Assert::IsTrue(IsA<Ab>(ab));
			Assert::IsTrue(IsA<Ac>(ac));
			Assert::IsTrue(IsA<Ab1>(ab1));
			Assert::IsTrue(IsA<Ac1>(ac1));

			// Objects should not be IsA to other objects (related or unrelated)
			Assert::IsFalse(IsA<XTObject>(ab));
			Assert::IsFalse(IsA<A>(ab));
			Assert::IsFalse(IsA<B>(ab));
			Assert::IsFalse(IsA<Aa>(ab));
			Assert::IsFalse(IsA<Ab1>(ab));
			Assert::IsFalse(IsA<Ac>(ab));
			Assert::IsFalse(IsA<Ac1>(ab));
			Assert::IsFalse(IsA<Y>(ab));
			Assert::IsFalse(IsA<Z>(ab));

			delete x;
			delete a;
			delete b;
			delete aa;
			delete ab;
			delete ac;
			delete ab1;
			delete ac1;
		}

		TEST_METHOD(TestReflectionIsFrom)
		{
			XTObject* x = new XTObject();
			XTObject* a = new A();
			XTObject* b = new B();
			XTObject* aa = new Aa();
			XTObject* ab = new Ab();
			XTObject* ac = new Ac();
			XTObject* ab1 = new Ab1();
			XTObject* ac1 = new Ac1();

			// Objects should be IsFrom to themselves
			Assert::IsTrue(IsFrom<XTObject>(x));
			Assert::IsTrue(IsFrom<A>(a));
			Assert::IsTrue(IsFrom<B>(b));
			Assert::IsTrue(IsFrom<Aa>(aa));
			Assert::IsTrue(IsFrom<Ab>(ab));
			Assert::IsTrue(IsFrom<Ac>(ac));
			Assert::IsTrue(IsFrom<Ab1>(ab1));
			Assert::IsTrue(IsFrom<Ac1>(ac1));

			// Objects should be IsFrom to base types and not IsFrom derived types
			Assert::IsTrue(IsFrom<XTObject>(ac));
			Assert::IsTrue(IsFrom<A>(ac));
			Assert::IsTrue(IsFrom<Z>(ac));
			Assert::IsFalse(IsFrom<B>(ac));
			Assert::IsFalse(IsFrom<Aa>(ac));
			Assert::IsFalse(IsFrom<Ab>(ac));
			Assert::IsFalse(IsFrom<Ab1>(ac));
			Assert::IsFalse(IsFrom<Ac1>(ac));
			Assert::IsFalse(IsFrom<Y>(ac));

			delete x;
			delete a;
			delete b;
			delete aa;
			delete ab;
			delete ac;
			delete ab1;
			delete ac1;
		}

		TEST_METHOD(TestReflectionTypeName)
		{
			XTObject* ab1 = new Ab1();

			Assert::IsTrue(ab1->GetTypeInfo().GetName() == "Ab1");
		}
	};
}