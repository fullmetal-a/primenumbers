#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include "XMLMgr.h"

std::mutex g_Mutex;

bool IsPrimeNumber(uint32_t n)
{
	for (uint32_t i = 3; i <= n / 2; i += 2)
	{
		if ((n % i) == 0)
			return false;
	}
	return true;
}

void WriteNumber(uint32_t n, std::vector<uint32_t>* container)
{
	static std::mutex m;
	static size_t i = 0;
	m.lock();
	container->at(i) = n;
	i++;
	m.unlock();
}

void GeneratePrimeNumbers(uint32_t from, uint32_t to, std::vector<uint32_t>* container)
{
	if (from > to)
	{
		auto tmp = to;
		to = from;
		from = tmp;
	}

	for (auto i = from; i <= to; i++)
	{
		if (IsPrimeNumber(i))
			WriteNumber(i, container);
	}
}

int main()
{
	CXMLMgr mgr;
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
			intervals.push_back(std::pair<uint32_t, uint32_t>(low, high));
		}

		if(intervals.size() == 0)
			throw std::runtime_error("No intervals was found in XML file.");
		
		size_t threadCount = intervals.size();

		threads.resize(threadCount);
		primes.resize(2000);
		for (size_t i = 0; i < threadCount; i++)
			threads[i] = std::thread(GeneratePrimeNumbers, intervals[i].first, intervals[i].second, &primes);

		std::cout << "Work initialized, " << threadCount << "threads are working. Waiting for all threads..." << std::endl;
		for (size_t i = 0; i < threadCount; i++)
			threads[i].join();

		//Sort, remove duplicates

		std::string allPrimeNumbers;
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