//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
#include <DirectXMath.h>

namespace TestApp
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();

	private:
		void Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		std::wstring RunTests();

		void RunTest_OneTimeScan();
		void RunTest_RealTimeScan_StaticInputData();
		std::wstring RunTest_RealTimeScan_DynamicInputData();
		float GetLength(DirectX::XMFLOAT3 vector);

		const char* CalcInputMeshFilename();
		const char* CalcInputDynMeshTestFilename();
		char fileBuffer[1000];
	};
}
