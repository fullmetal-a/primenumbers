#include "PrimeNumbers.h"
#include <thread>

namespace PrimeNumbers_Utils
{
	bool IsPrimeNumber(uint32_t n)
	{
		if (n == 1)
			return false;
		else if (n == 2 || n == 3 || n == 5 || n == 7)
			return true;

		for (uint32_t i = 3; i <= n / 2; i += 2)
		{
			if ((n % i) == 0)
				return false;
		}
		return true;
	}


}
PrimeNumbersGenerator::PrimeNumbersGenerator()
{
}

PrimeNumbersGenerator::PrimeNumbersGenerator(const PrimeNumbersGenerator& copy)
{
	m_intervals = copy.m_intervals;
	m_primes = copy.m_primes;
}

void PrimeNumbersGenerator::InsertInterval(uint32_t low, uint32_t high)
{
	if (low > high)
	{
		size_t tmp = low;
		low = high;
		high = tmp;
	}

	m_intervals.push_back(std::pair<uint32_t, uint32_t>(low, high));
}

void PrimeNumbersGenerator::_pushPrimeSafely(uint32_t n)
{
	m_mutex.lock();
	m_primes.insert(n);
	m_mutex.unlock();
}

void PrimeNumbersGenerator::_threadRoutine(uint32_t low, uint32_t high)
{
	if (low % 2 == 0 && low - high != 0)
		low++;
	for (auto i = low; i <= high; i+=2)
	{
		if (PrimeNumbers_Utils::IsPrimeNumber(i))
			_pushPrimeSafely(i);
	}
}

void PrimeNumbersGenerator::Generate()
{
	auto size = m_intervals.size();
	if (size == 0)
		return;

	std::vector<std::thread> threads;
	threads.resize(size);
	for (uint32_t i = 0; i < size; i++)
		threads[i] = std::thread(&PrimeNumbersGenerator::_threadRoutine, this, m_intervals[i].first, m_intervals[i].second);

	for (uint32_t i = 0; i < size; i++)
		threads[i].join();


}

std::string PrimeNumbersGenerator::GetUniqueGeneratedPrimes() const
{
	if (m_primes.size() == 0)
		return "";
	std::string retval;
	for (auto it = m_primes.begin(); it != m_primes.end(); it++)
		retval = retval + std::to_string(*it) + " ";
	retval.pop_back();
	return retval;
}

size_t PrimeNumbersGenerator::GetIntervalCount() const
{
	return m_intervals.size();
}