// SuperPermutation.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <stack>


#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))


/// best for 7 = 5906
const int N = 7;


class List;

// global variables
int generation = 0;
List** permutations;
int populationSize;
double mutateOneLocationChance = 0;



static unsigned long rx = 123456789, ry = 362436069, rz = 521288629;
unsigned long ul_rnd()
{
	unsigned long t;
	rx ^= rx << 16;
	rx ^= rx >> 5;
	rx ^= rx << 1;

	t = rx;
	rx = ry;
	ry = rz;
	rz = t ^ rx ^ ry;

	return rz;
}

double rnd()
{
	double v = ul_rnd();
	return v / ((double)ULONG_MAX + 1.0);
}




class List
{
public:
	int length;
	int* data;
public:
	List() { length = 0; data = nullptr; }
	List(int _length) { length = _length; data = new int[length]; }
	List(int _length, int* _data) { length = _length; data = new int[length]; cloneData(_data); }
	List(List* _list) { length = _list->length; data = new int[length]; cloneData(_list->data); }
	~List() { if (data != nullptr) delete[] data; data = nullptr; length = 0; }

	List* clone()
	{
		List* list = new List(length, data);
		return list;
	}

	void cloneData(int* _data)
	{
		for (int i = 0; i < length; i++)
			data[i] = _data[i];
	}

	int* cloneData()
	{
		int* ret = new int[length];
		for (int i = 0; i < length; i++)
			ret[i] = data[i];
		return ret;
	}

	int* slice(int _first, int _length)
	{
		int last = _first + _length;
		_ASSERT(last <= length);
		int* ret = new int[_length];
		for (int j = 0, i = _first; i < last; j++, i++)
		{
			ret[j] = data[i];
		}
		return ret;
	}

	void splice(int _first, int _length)
	{
		int newLength = length - _length;
		int last = _first + _length;
		_ASSERT(last <= length);

		int* ret = new int[newLength];

		int i, j;
		for (i = 0; i < _first; i++)
			ret[i] = data[i];
		for (j = last; j < length; j++, i++)
			ret[i] = data[j];

		delete[] data;
		data = ret;
		length = newLength;
	}

	List* concat(List* _list)
	{
		int i, l = length + _list->length;
		List* ret = new List(l);

		for (i = 0; i < length; i++)
			ret->data[i] = data[i];

		for (int j = 0; i < l; i++, j++)
			ret->data[i] = _list->data[j];

		return ret;
	}

	void swapData(int _i, int _j)
	{
		int t = data[_i];
		data[_i] = data[_j];
		data[_j] = t;
	}

	void cout()
	{
		for (int i = 0; i < length; i++)
			std::cout << data[i] << ",";
		std::cout << std::endl;
	}
};



// create one of every permutation of symbols in _n into global 'permutations'
int heapPermutation(List* _list, int _size, int _n, int _p)
{
	if (_size == 1)
	{
		int* s = _list->slice(0, _n);
		permutations[_p] = new List(_n, s);
		delete [] s;
		return _p + 1;
	}

	for (int i = 0; i < _size; i++)
	{
		_p = heapPermutation(_list, _size - 1, _n, _p);
		if (_size & 1)
			_list->swapData(0, _size - 1);
		else
			_list->swapData(i, _size - 1);
	}
	return _p;
}

int factorial(int _n)
{
	int t = 1;
	for (int i = 1; i <= _n; i++)
		t *= i;
	return t;
}

int lowestBound(int _n)
{
	int t = 0;
	if (_n >= 7)
	{
		t = factorial(_n);
		t += factorial(_n - 1);
		t += factorial(_n - 2);
		t += factorial(_n - 3);
		t += _n - 3;
	}
	else
	{
		for (int i = 1; i <= _n; i++)
			t += factorial(i);
	}
	return t;
}

// look for _value in _list searching from _start to _last (not end-inclusive)
int findValue(int _value, int* _list, int _start, int _last)
{
	for (int i = _start; i < _last; i++)
		if (_list[i] == _value)
			return i;
	return -1;
}

// squash them by removing overlapping sequences (1,2,3,2,3,4 => 1,2,3,4) up to length N:
// for each value in the list with i
//  for each matching value in the list up to N positions away with j
//   advance i & j by one
//   if we have advanced i == original j then we have an overlapping sequence
//   else compare values, if they are not equal, break inner loop
void squash(List* _list)
{
	for (int i = 0, l = _list->length - N; i < l; i++)
	{
		int ii = i;

		bool overlap = false;
		int j = i;
		while ((j = findValue(_list->data[i], _list->data, j + 1, i + N)) != -1)
		{
			int jj = j;

			do {
				ii++;
				jj++;
				if (_list->data[ii] != _list->data[jj])
				{
					// no overlap here
					break;
				}
			} while (ii < j);
			if (ii >= j)
			{
				overlap = true;
				// break the inner loop, we're removing i..j so search no more inside that span
				break;
			}
		}

		if (overlap)
		{
			// we found an overlap from i to j
			_list->splice(i, j - i);
			l = _list->length - N;
		}
	}
}

// count how many entire consecutive permutations exist in _list
int countPermutations(List* _list, const int _numPermutations)
{
	int* pointers = new int[_numPermutations] {};
	int c = 0;

	// for every value in this _list
	for (int i = 0, l = _list->length; i < l; i++)
	{
		int v = _list->data[i];

		// for every permutation
		for (int j = 0; j < _numPermutations; j++)
		{
			// if we haven't matched this permutation yet
			if (pointers[j] < N)
			{
				// if the value matches the next permutation value
				if (v == permutations[j]->data[pointers[j]])
				{
					// advance the pointer to the next permutation value
					if (++pointers[j] >= N)
					{
						c++;		// complete match, count it
					}
				}
				else
				{
					// reset to the start
					pointers[j] = 0;
					// if the value matches the start permutation value
					if (v == permutations[j]->data[pointers[j]])
					{
						// advance the pointer to the next permutation value
						pointers[j]++;
					}
				}
			}
		}

		// exit if we've found all of the permutations
		if (c >= _numPermutations)
			break;
	}

	delete[] pointers;
	return c;
}

int stackDepth = 0;
List* bestFit(int _numPermutations, bool* _used, List* _base )
{
	List** shortestList = new List*[_numPermutations];
	int* shortestOrigin = new int[_numPermutations];
	int shortCount = 0;
	int shortest = INT32_MAX;

	int available = 0;

	// try to fit all unused permutations onto the base string
	// record the shortest squashed resulting strings (including ties)
	for (int i = 0; i < _numPermutations; i++)
	{
		if (!_used[i])
		{
			available++;

			List* c = _base->concat(permutations[i]);
			squash(c);

			if (c->length < shortest)
			{
				// new shortest, throw away old list
				shortest = c->length;
				if (shortestList)
				{
					for (int s = 0; s < shortCount; s++)
						delete shortestList[s];
				}
				shortCount = 0;
			}

			if (c->length == shortest)
			{
				// tie for shortest, add it to the list
				shortestOrigin[shortCount] = i;
				shortestList[shortCount] = c->clone();
				shortCount++;
			}

			delete c;
		}
	}

	// there are no permutations available
	if (available == 0)
	{
		for (int s = 0; s < shortCount; s++)
			delete shortestList[s];
		delete[] shortestList;
		delete[] shortestOrigin;
		return _base->clone();
	}

	// recurse for all tied shortest strings
	List* best = nullptr;
	//if (shortCount > 1) std::cout << "results tied x " << shortCount << std::endl;
	for (int i = 0; i < shortCount; i++)
	{
		_used[shortestOrigin[i]] = true;

		stackDepth++;
		List* copy = shortestList[i]->clone();
		List* r = bestFit(_numPermutations, _used, copy);
		stackDepth--;

		if (r)
		{
			if (best)
			{
				if (r->length < best->length)
				{
					delete best;
					best = r;
				}
				else
				{
					delete r;
				}
			}
			else
			{
				best = r;
			}
		}
		_used[shortestOrigin[i]] = false;
		delete copy;
	}

	for (int s = 0; s < shortCount; s++)
		delete shortestList[s];
	delete[] shortestList;
	delete[] shortestOrigin;

	if (best)
	{
		return best;
	}

	return _base->clone();
}




int main()
{
	const int numPermutations = factorial(N);

	/// NOTE: good results = n! + (n-1)! + (n-2)! + (n-3!) + n - 3  (Egan et al)
	const double goodResult = lowestBound(N);

	///the maximum incentive for a genome to grow longer
	const double maxIncentive = (goodResult * goodResult);


	permutations = new List*[ numPermutations ];

	// recursive, but maximum stack depth = N
	int symbols[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };
	List* symbolList = new List(20, symbols);
	_ASSERT( N <= 20 );

	heapPermutation(symbolList, N, N, 0);

	std::cout << "permutations of " << N << " = " << numPermutations << std::endl;
	for (int i = 0; i < numPermutations; i++)
		permutations[i]->cout();
	std::cout << "typical minimum bound = " << goodResult << std::endl;

	bool* permutationUsed = new bool[numPermutations]{};
	List* shortestList = bestFit(numPermutations, permutationUsed, permutations[0]);
	for (int i = 0; i < numPermutations; i++)
	{
		List* base = new List(permutations[i]->length, permutations[i]->cloneData());
		permutationUsed[i] = true;
		List* result = bestFit(numPermutations, permutationUsed, base);
		if (countPermutations(result, numPermutations) == numPermutations)
		{
			if (!shortestList || result->length <= shortestList->length)
			{
				if (shortestList) delete shortestList;
				shortestList = result;
				std::cout << "solution " << i << ": " << shortestList->length << std::endl;
				shortestList->cout();
			}
			else
			{
				delete result;
			}
		}
		permutationUsed[i] = false;
		delete base;
	}
	delete[] permutationUsed;

	if (shortestList)
	{
		int p = countPermutations(shortestList, numPermutations);
		std::cout << "RESULTS:\npermutations = " << p << "\nlength = " << shortestList->length << std::endl;
		shortestList->cout();
		std::cout << "\n" << std::endl;
	}
	else
	{
		std::cout << "No results found" << std::endl;
	}
	return 0;
}
