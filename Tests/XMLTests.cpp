#include "stdafx.h"
#include "CppUnitTest.h"
#include "../PrimeNumbers/XMLMgr.h"
#include <fstream>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tests
{		
	TEST_CLASS(XMLTests)
	{
	public:
		
		TEST_METHOD(ParseTest)
		{
			try
			{
				CXMLMgr mgr("../Tests/test_in.xml");
				auto node = mgr.GetRootNode();

				const int cvalue1 = 35;
				int value1 = node->GetNodeSkipMatches("someTag", 0)->GetNode("value", false)->GetContent<int>();
				Assert::AreEqual(cvalue1, value1);

				const int cvalue2 = 8725;
				int value2 = node->GetNodeSkipMatches("someTag", 1)->GetNode("data", false)->GetNode("value", false)->GetContent<int>();
				Assert::AreEqual(cvalue2, value2);

				const char cvalue3[] = "Hello world!";
				std::string value3 = node->GetNodeSkipMatches("someTag", 2)->GetNode("string", false)->GetContent();
				Assert::AreEqual(cvalue3, value3.c_str());

				const int cvalue4 = -30;
				int value4 = node->GetNode("anotherTag", false)->GetNode("ice", false)->GetNode("temp", false)->GetContent<int>();
				Assert::AreEqual(cvalue4, value4);
			}
			catch (std::runtime_error& err)
			{
				//Assert::Fail accepts only wchar_t* so we need to convert error message to wide-char string
				size_t len = strlen(err.what());
				wchar_t* errmsg = new wchar_t[len + 1];
				size_t unused = 0;
				mbstowcs_s(&unused, errmsg, len * 2, err.what(), len + 1);
				Assert::Fail(errmsg);
				delete[] errmsg;
			}
		}

		TEST_METHOD(ParseAndSaveTest)
		{
			try
			{
				const char inFile[] = "../Tests/test_in.xml";
				const char outFile[] = "../Tests/test_out.xml";
				CXMLMgr mgr(inFile);
				mgr.SaveTo(outFile); 

				std::ifstream f(inFile);
				std::string originalBuffer, savedBuffer; 
				f.seekg(0, std::ios::end);
				originalBuffer.reserve(size_t(f.tellg()));
				f.seekg(0, std::ios::beg);
				originalBuffer.assign((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
				f.close(); 

				f.open(outFile); 
				f.seekg(0, std::ios::end);
				savedBuffer.reserve(size_t(f.tellg()));
				f.seekg(0, std::ios::beg);
				savedBuffer.assign((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
				f.close();

				Assert::AreEqual(originalBuffer.c_str(), savedBuffer.c_str());
			}
			catch (std::runtime_error& err)
			{
				//Assert::Fail accepts only wchar_t* so we need to convert error message to wide-char string
				size_t len = strlen(err.what());
				wchar_t* errmsg = new wchar_t[len + 1];
				size_t unused = 0;
				mbstowcs_s(&unused, errmsg, len * 2, err.what(), len + 1);
				Assert::Fail(errmsg);
				delete[] errmsg;
			}
		}

		TEST_METHOD(ParseTest2)
		{
			try
			{
				const char xml[] = 
				{
					"<first>\n"
					"<second>\n"
					"<value>123456</value>\n"
					"</second>\n"
					"</first>"
				};

				CXMLMgr mgr;
				mgr.LoadFromBuffer(xml);
				auto node = mgr.GetRootNode();

				const int cvalue = 123456;
				int value = node->GetNode("first", false)->GetNode("second", false)->GetNode("value", false)->GetContent<int>();

				Assert::AreEqual(cvalue, value);
			}
			catch (std::runtime_error& err)
			{
				//Assert::Fail accepts only wchar_t* so we need to convert error message to wide-char string
				size_t len = strlen(err.what());
				wchar_t* errmsg = new wchar_t[len + 1];
				size_t unused = 0;
				mbstowcs_s(&unused, errmsg, len * 2, err.what(), len + 1);
				Assert::Fail(errmsg);
				delete[] errmsg;
			}
		}
	};
}