#include <iostream>
#include "XMLMgr.h"

int main()
{
	try
	{
		CXMLMgr mgr("D:/ftp/test.xml");
	}
	catch (std::runtime_error& err)
	{
		std::cout << err.what() << std::endl;
	}
	system("pause");
	return 0;
}