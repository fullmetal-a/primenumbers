#include <iostream>
#include "XMLMgr.h"
#include "PrimeNumbers.h"

int main()
{
	CXMLMgr mgr;
	PrimeNumbersGenerator gen;
	try
	{
		mgr.LoadFrom("intervals.xml");
		std::vector<std::pair<uint32_t, uint32_t>> intervals;
		std::vector<uint32_t> primes;
		std::vector<std::thread> threads;
		auto xml_intervals = mgr.GetRootNode()->GetNode("root", false)->GetNode("intervals", false);
		uint32_t low, high;
		for (size_t i = 0; i <  xml_intervals->GetNodeCount(); i++)
		{
			low = xml_intervals->GetNode("interval", i)->GetNode("low", false)->GetContent<uint32_t>();
			high = xml_intervals->GetNode("interval", i)->GetNode("high", false)->GetContent<uint32_t>();
			gen.InsertInterval(low, high);
		}

		if(gen.GetIntervalCount() == 0)
			throw std::runtime_error("No intervals was found in XML file.");
		
		gen.Generate();

		std::string allPrimeNumbers = gen.GetUniqueGeneratedPrimes();
		auto primesNode = mgr.GetRootNode()->GetNode("root")->GetNode("primes");
		primesNode->SetTagName("primes");
		primesNode->SetContent(allPrimeNumbers);
		mgr.Save();
		std::cout << "Work has been complete. All numbers are saved in 'intervals.xml' file." << std::endl;
	}
	catch (std::runtime_error& err)
	{
		std::cout << err.what() << std::endl;
	}

	system("pause");
	return 0;
}