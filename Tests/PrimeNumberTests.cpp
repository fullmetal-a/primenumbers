#include "stdafx.h"
#include "CppUnitTest.h"
#include "../PrimeNumbers/PrimeNumbers.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tests
{
	TEST_CLASS(PrimeNumberTests)
	{
	public:


		//Compares generated prime numbers from 0 to 200 with numbers taken from internet
		TEST_METHOD(NumbersCorrect1)
		{
			PrimeNumbersGenerator gen; 
			//Summary range - from 0 to 200
			gen.AddInterval(0, 15);
			gen.AddInterval(11, 27);
			gen.AddInterval(50, 25);
			gen.AddInterval(51, 51);
			gen.AddInterval(52, 100);
			gen.AddInterval(101, 200);
			gen.Generate();
			auto primes = gen.GetUniqueGeneratedPrimes();
			Assert::AreEqual("2 3 5 7 11 13 17 19 23 29 31 37 41 43 47 53 59 61 67 71 73 79 83 89 97 101 103 107 109 " 
							 "113 127 131 137 139 149 151 157 163 167 173 179 181 191 193 197 199", primes.c_str());
			
		}

		TEST_METHOD(NumbersCorrect2)
		{
			PrimeNumbersGenerator gen;
			//Summary range - from 416 to 800
			gen.AddInterval(416, 421);
			gen.AddInterval(420, 540);
			gen.AddInterval(700, 421);
			gen.AddInterval(750, 701);
			gen.AddInterval(751, 800);
			gen.Generate();
			auto primes = gen.GetUniqueGeneratedPrimes();
			Assert::AreEqual("419 421 431 433 439 443 449 457 461 463 467 479 487 491 499 503 509 521 523 541 547 "
				 "557 563 569 571 577 587 593 599 601 607 613 617 619 631 641 643 647 653 659 661 673 677 683 691 "
				 "701 709 719 727 733 739 743 751 757 761 769 773 787 797", primes.c_str());
		}

	};
}