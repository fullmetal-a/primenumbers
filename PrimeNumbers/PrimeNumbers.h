#pragma once
#include <vector>
#include <string>
#include <set>
#include <mutex>

class PrimeNumbersGenerator
{
private:
	std::vector<std::pair<uint32_t, uint32_t>> m_intervals;
	std::set<uint32_t> m_primes;
	std::mutex m_mutex;

	void _pushPrimeSafely(uint32_t n);
	void _threadRoutine(uint32_t low, uint32_t high);

public:
	PrimeNumbersGenerator();
	PrimeNumbersGenerator(const PrimeNumbersGenerator& copy);

	void InsertInterval(uint32_t low, uint32_t high);
	void Generate();

	std::string GetUniqueGeneratedPrimes() const;
	size_t GetIntervalCount() const;
};