// SuperPermutation.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <stack>


#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))


/// best for 7 = 5906
const int N = 6;


class List;

// global variables
int numPermutations;
List** permutations;
double goodResult;


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

int findValue(int _value, int* _list, int i, int _last)
{
	int* p = _list + i;
	while (i++ < _last)
		if (*p++ == _value)
			return i - 1;
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
int countPermutations(List* _list)
{
	int* pointers = new int[numPermutations] {};
	int c = 0;

	// for every value in this _list
	for (int i = 0, l = _list->length; i < l; i++)
	{
		int v = _list->data[i];

		// for every permutation
		for (int j = 0; j < numPermutations; j++)
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
		if (c >= numPermutations)
			break;
	}

	delete[] pointers;
	return c;
}

int stackDepth = 0;
int progressCount = 0;
List* bestFit(bool* _used, List* _base, bool _alternatives = false )
{
	int startLength = _base->length;
	List** shortestList = new List*[numPermutations];
	int* shortestOrigin = new int[numPermutations];
	int shortCount = 0;
	int shortest = INT32_MAX;

	int available = 0;

	//_base->cout();

	// try to fit all unused permutations onto the base string
	// record the shortest squashed resulting strings (including ties)
	for (int i = 0; i < numPermutations; i++)
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
	if (available == 0 || shortCount == 0)
	{
		for (int s = 0; s < shortCount; s++)
			delete shortestList[s];
		delete[] shortestList;
		delete[] shortestOrigin;
		return _base->clone();
	}

	// recurse for tied shortest strings
	List* best = nullptr;
	//std::cout << "progress " << shortestList[0]->length << std::endl;

	// terminate if this will grow the string significantly and there are still alternatives to explore
	int growth = shortestList[0]->length - startLength;
	if (growth > 2)
	{
		if (_alternatives)
		{
			std::cout << growth << "@" << startLength << std::endl;
			return _base;
		}
	}

	// explore deeper recursively near the end of the search where it's relatively cheap
	int deep = 1;
	if (stackDepth > goodResult / 2) deep = min(shortCount, 2);
	if (stackDepth > goodResult * 3 / 4) deep = shortCount;
	for (int i = 0; i < deep; i++)
	{
		_used[shortestOrigin[i]] = true;

		stackDepth++;
		List* copy = shortestList[i]->clone();
		List* r = bestFit(_used, copy, i < deep - 1);
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
	numPermutations = factorial(N);

	/// NOTE: good results = n! + (n-1)! + (n-2)! + (n-3!) + n - 3  (Egan et al)
	goodResult = lowestBound(N);

	///the maximum incentive for a genome to grow longer
	const double maxIncentive = (goodResult * goodResult);


	permutations = new List*[ numPermutations ];

	// recursive, but maximum stack depth = N
	int symbols[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };
	List* symbolList = new List(20, symbols);
	_ASSERT( N <= 20 );

	heapPermutation(symbolList, N, N, 0);

	std::cout << "permutations of " << N << " = " << numPermutations << std::endl;
	//for (int i = 0; i < numPermutations; i++)
	//	permutations[i]->cout();
	std::cout << "low length result = " << goodResult << std::endl;

	bool* permutationUsed = new bool[numPermutations]{};
	permutationUsed[0] = true;
	List* shortestList = bestFit(permutationUsed, permutations[0]);
	delete[] permutationUsed;

	if (shortestList)
	{
		int p = countPermutations(shortestList);
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
