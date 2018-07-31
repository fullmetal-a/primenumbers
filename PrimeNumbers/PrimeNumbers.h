#pragma once
#include <vector>
#include <string>
#include <set>
#include <mutex>

//Generates prime numbers in specific intervals
//Every interval calculated in separate thread
class PrimeNumbersGenerator
{
private:
	//Container of intervals. Intervals stored by pairs <low, high>
	std::vector<std::pair<uint32_t, uint32_t>> m_intervals; 
	//Container of prime numbers. Set stores only unique values and they are always sorted
	std::set<uint32_t> m_primes; 
	std::mutex m_mutex; //Synq-object


	void _pushPrimeSafely(uint32_t n); //Safely insert prime number to primes container
	void _threadRoutine(uint32_t low, uint32_t high); //Routine of generator's thread

public:
	//Default c-tor
	PrimeNumbersGenerator();
	//Copy c-tor
	PrimeNumbersGenerator(const PrimeNumbersGenerator& copy);
	//~PrimeNumbersGenerator();

	void AddInterval(uint32_t low, uint32_t high); //Adds interval to interval container
	void Generate();

	std::string GetUniqueGeneratedPrimes() const; //Get string of all calculated prime numbers delimited by spaces
	size_t GetIntervalCount() const; //Actualy gets an interval count...
};