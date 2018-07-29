#include <iostream>
#include "XMLMgr.h"

int main()
{
	CXMLMgr mgr;
	try
	{
		mgr.LoadFrom("D:/ftp/test.xml");
	}
	catch (std::runtime_error& err)
	{
		std::cout << err.what() << std::endl;
	}
	mgr.GetRootNode()->GetNode("tagKek")->GetNode("ololo", 0)->GetNode(1)->Destroy();
	mgr.Save();
	system("pause");
	return 0;
}