#include "PrimeNumbers.h"
#include <thread>

namespace PrimeNumbers_Utils
{
	//Checks if number is prime
	bool IsPrimeNumber(uint32_t n) 
	{
		//If number is <= 7 it's better to check it's directly
		if (n == 0 || n == 1)
			return false;
		else if (n == 2 || n == 3 || n == 5 || n == 7)
			return true;

		for (uint32_t i = 3; i <= n / 2; i += 2) //Iterate starting from 3 till half of N, iterating only odd numbers
		{
			if ((n % i) == 0) //If number is divided completely
				return false; //Than number is not prime
		}
		return true; //If no number to divide completly - number is prime
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

void PrimeNumbersGenerator::AddInterval(uint32_t low, uint32_t high)
{
	if (low > high) //If low interval is higher than high - swap it
	{
		size_t tmp = low;
		low = high;
		high = tmp;
	}

	m_intervals.push_back(std::pair<uint32_t, uint32_t>(low, high)); //Push interval to interval container
}

void PrimeNumbersGenerator::_pushPrimeSafely(uint32_t n)
{
	m_mutex.lock();
		m_primes.insert(n); //Lock mutex and write number to set
	m_mutex.unlock();
}

void PrimeNumbersGenerator::_threadRoutine(uint32_t low, uint32_t high)
{
	if (low % 2 == 0 && low != high && low > 3) //If lower value is even and low value not equals high and bigger than 3
		low++; //Make lower value odd
	uint32_t  inc = 1; //Initialy iterates by +1
	for (auto i = low; i <= high; i+=inc) //Iterate only odd values
	{
		if (i >= 3 && i != 2)	//Iterate by +2 when i is bigger or equals 3
			inc = 2;
		if (PrimeNumbers_Utils::IsPrimeNumber(i))	//Check if i is prime number
			_pushPrimeSafely(i); //Write i to shared container
	}
}

void PrimeNumbersGenerator::Generate()
{
	auto size = m_intervals.size(); 
	if (size == 0) //If no intervals - return
		return;

	std::vector<std::thread> threads; //Threads container
	threads.resize(size); //Thread count == interval count
	for (uint32_t i = 0; i < size; i++) //Create threads for each interval
		threads[i] = std::thread(&PrimeNumbersGenerator::_threadRoutine, this, m_intervals[i].first, m_intervals[i].second);

	//Wait for all threads
	for (uint32_t i = 0; i < size; i++) 
		threads[i].join();
}

std::string PrimeNumbersGenerator::GetUniqueGeneratedPrimes() const
{
	if (m_primes.size() == 0) //If no primes - return an empty string
		return "";
	std::string retval;
	for (auto it = m_primes.begin(); it != m_primes.end(); it++) //Make string of all prime numbers delimited by spaces
		retval = retval + std::to_string(*it) + " ";
	retval.pop_back(); //Remove last space from sttring
	return retval;
}

size_t PrimeNumbersGenerator::GetIntervalCount() const
{
	return m_intervals.size();
}