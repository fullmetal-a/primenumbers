#include <iostream>
#include "XMLMgr.h"
#include "PrimeNumbers.h"

int main()
{
	CXMLMgr mgr;	//Creating XML Manager object
	PrimeNumbersGenerator gen;	//Creating prime numbers generator
	try
	{
		mgr.LoadFrom("intervals.xml");	//Loading and parsing XML file

		//Getting content of <intervals> tag (node) 
		//Do not create tags if they are not exist. In this case it will cause en exeption.
		auto xml_intervals = mgr.GetRootNode()->GetNode("root", false)->GetNode("intervals", false);	
		
		//Extracting intervals from parsed XML and adding it
		uint32_t low, high;
		for (size_t i = 0; i <  xml_intervals->GetNodeCount(); i++)
		{
			low = xml_intervals->GetNode("interval", i)->GetNode("low", false)->GetContent<uint32_t>();	//Reading low interval from
			high = xml_intervals->GetNode("interval", i)->GetNode("high", false)->GetContent<uint32_t>();
			gen.AddInterval(low, high);	//Adding intervals to generator instance
		}

		if(gen.GetIntervalCount() == 0)	//If we have no intervals - throw an exeption, we cannot continue
			throw std::runtime_error("No intervals was found in XML file.");
		
		gen.Generate();	//Generating prime numbers

		std::string allPrimeNumbers = gen.GetUniqueGeneratedPrimes();	//Get string row of all numbers (every number is unique)

																		//Get node of <primes> tag. If it exists - returns existing node. If it's not - it will be created.
		auto primesNode = mgr.GetRootNode()->GetNode("root")->GetNode("primes");	
		primesNode->SetContent(allPrimeNumbers);	//Write all prime numbers to <primes> tag
		mgr.Save();	//Save XML file
		std::cout << "Work has been complete. All numbers are saved in 'intervals.xml' file." << std::endl;
	}
	catch (std::runtime_error& err)	//Handles all exeptions of XML parser and main application
	{
		std::cout << err.what() << std::endl;	//Prints error message
	}

	system("pause");
	return 0;
}