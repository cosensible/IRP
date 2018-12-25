//#include "./LKH3Interface/_Cache.h"
//
//using namespace std;
//
//void test_get(const szx::TspCache_BinTreeImpl &tspCache, const vector<int> &nodes)
//{
//	auto cacheTour = tspCache.get(nodes);
//	if (!cacheTour.first.empty() && cacheTour.second > 0) {
//		cout << "get optimal from cache." << endl;
//		for (auto n : cacheTour.first) {
//			cout << n << " ";
//		}
//		cout << endl;
//		cout << "the tour cost is " << cacheTour.second << endl;
//	}
//	else {
//		cout << "no optimal in cache for this `nodes` set" << endl;
//		for (auto n : nodes) {
//			cout << n << " ";
//		}
//		cout << endl;
//	}
//}
//
//void test_get1(const szx::TspCache_BinTreeImpl &tspCache)
//{
//	vector<int> nodes = { 0,1,2,3,4,5 };
//	test_get(tspCache, nodes);
//}
//
//void test_get2(const szx::TspCache_BinTreeImpl &tspCache)
//{
//	vector<int> nodes = { 0,1,2,4,5 };
//	test_get(tspCache, nodes);
//}
//
//void test_get3(const szx::TspCache_BinTreeImpl &tspCache)
//{
//	vector<int> nodes = { 0 };
//	test_get(tspCache, nodes);
//}
//
//void test_get4(const szx::TspCache_BinTreeImpl &tspCache)
//{
//	vector<int> nodes = { 0,2 };
//	test_get(tspCache, nodes);
//}
//
//void test_set1(szx::TspCache_BinTreeImpl &tspCache)
//{
//	vector<int> nodes = { 0,2 };
//	double cost = 89.76;
//	tspCache.set(nodes, cost);
//	if (tspCache.set(nodes, cost)) {
//		cout << "test_set1 passed." << endl;
//	}
//	else {
//		cout << "test_set1 failed." << endl;
//	}
//}
//
//void test_set2(szx::TspCache_BinTreeImpl &tspCache)
//{
//	vector<int> nodes = { 0,2,3 };
//	double cost = 0;
//	if (!tspCache.set(nodes, cost)) {
//		cout << "test_set2 passed." << endl;
//	}
//	else {
//		cout << "test_set2 failed." << endl;
//	}
//}
//
//int main()
//{
//	szx::TspCache_BinTreeImpl tspCache;
//	string tourPath("C:\\Users\\gen\\Downloads\\LKH\\3\\tour");
//	tspCache.readFileToCache(tourPath);
//
//	test_get1(tspCache);	//get optimal
//	test_get2(tspCache);	//no optimal in cache
//	test_get3(tspCache);	//no optimal in cache
//	test_get4(tspCache);	//no optimal in cache
//
//	test_set1(tspCache);	//passed
//	test_set2(tspCache);	//passed
//
//	test_get4(tspCache);	//get optimal after set
//
//	system("pause");
//	return 0;
//}


#include "./LKH3Interface/_Graph.h"
#include "./LKH3Interface/_Cache.h"
#include <iostream>
#include <sstream>
#include <math.h>
#include <string>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <algorithm>
#include <chrono>
#include <random>

using namespace std;
using namespace szx;

//// testing and sample code.
//void testCoordList2D() {
//	Graph::CoordList<> cl({
//		{ 0, 0 },
//		{ 100, 200 },{ 200, 100 },
//		{ 200, 300 },{ 300, 200 },
//		{ 300, 400 },{ 400, 300 },
//		});
//	List<int> nl({ 0,1,3,2,4 });
//	TspCache_BinTreeImpl tsp("test6", cl);
//	tsp.readFileToCache(".\\LKH3Interface\\tour");
//
//	Graph::Tour sln;
//	std::cout << std::endl << tsp.solve(sln, nl, 5) << std::endl;
//	for (auto n : sln) {
//		std::cout << n << " ";
//	}
//	std::cout << std::endl;
//	// if (solve(sln, cl,1)) {
//	// EXTEND[szx][0]: record solution path.
//	//}
//}

void testTspCache() {
	int nodeNum = 200;
	int tourNum = 100000;
	int minNodeNumInTour = 3;
	int writeCount = 4 * tourNum;
	int readCount = 12 * tourNum;

	mt19937 rgen;
	chrono::steady_clock::time_point begin, end;

	cerr << "init test data: ";
	begin = chrono::steady_clock::now();
	TspCache::NodeList nodes(nodeNum);
	for (int i = 0; i < nodeNum; ++i) { nodes[i] = i; }
	vector<TspCache::TourAndCost> tours(tourNum);
	for (int i = 0; i < tourNum; ++i) {
		if (rgen() % 1000 != 0) { shuffle(nodes.begin(), nodes.end(), rgen); } // small number of duplicated paths with different distances.
		int tourLen = (rgen() % (nodeNum - minNodeNumInTour)) + minNodeNumInTour;
		tours[i].first.resize(tourLen);
		for (int n = 0; n < tourLen; ++n) { tours[i].first[n] = nodes[n]; }
		tours[i].second = rgen() % 10000 - 100; // allow negative distance.
	}
	TspCache::NodeSet containNode;
	end = chrono::steady_clock::now();
	cerr << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << "ms" << endl;

	cerr << endl;

	cerr << "zero overhead write test (x" << writeCount << "): ";
	rgen.seed();
	begin = chrono::steady_clock::now();
	int overwriteCount = 0;
	for (int i = 0; i < writeCount; ++i) {
		TspCache::TourAndCost &tour(tours[rgen() % tours.size()]); // pick a random tour.
		for (const auto &n : tour.first) { containNode.set(n); }
		++overwriteCount;
	}
	end = chrono::steady_clock::now();
	cerr << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << "ms" << endl;

	cerr << "              write test (x" << writeCount << "): ";
	rgen.seed();
	begin = chrono::steady_clock::now();
	overwriteCount = 0;
	TspCache tspCache;
	for (int i = 0; i < writeCount; ++i) {
		TspCache::TourAndCost &tour(tours[rgen() % tours.size()]); // pick a random tour.
		containNode.reset();
		for (const auto &n : tour.first) { containNode.set(n); }
		overwriteCount += tspCache.set(tour, containNode); // add it to cache.
	}
	end = chrono::steady_clock::now();
	cerr << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << "ms" << endl;
	cerr << "overwrite=" << overwriteCount << endl;

	cerr << endl;

	cerr << "zero overhead read test (x" << readCount << "): ";
	rgen.seed();
	begin = chrono::steady_clock::now();
	int missCount = 0;
	for (int i = 0; i < readCount; ++i) {
		TspCache::TourAndCost &tour(tours[rgen() % tours.size()]); // pick a random tour.
		containNode.reset();
		for (const auto &n : tour.first) { containNode.set(n); }
		for (int perturb = rgen() % 8 - 1; perturb > 0; --perturb) {
			containNode[rgen() % nodeNum] = (rgen() & 1); // modify the node set randomly.
		}
		missCount += tour.first.empty();
	}
	end = chrono::steady_clock::now();
	cerr << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << "ms" << endl;

	cerr << "              read test (x" << readCount << "): ";
	rgen.seed();
	begin = chrono::steady_clock::now();
	missCount = 0;
	for (int i = 0; i < readCount; ++i) {
		TspCache::TourAndCost &tour(tours[rgen() % tours.size()]); // pick a random tour.
		containNode.reset();
		for (const auto &n : tour.first) { containNode.set(n); }
		for (int perturb = rgen() % 8 - 1; perturb > 0; --perturb) {
			containNode[rgen() % nodeNum] = (rgen() & 1); // modify the node set randomly.
		}
		const TspCache::TourAndCost &sln(tspCache.get(containNode));
		missCount += sln.first.empty();
	}
	end = chrono::steady_clock::now();
	cerr << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << "ms" << endl;
	cerr << "miss=" << missCount << endl;
}



int main(int argc, char *argv[]) 
{
	testTspCache();
	system("pause");
}
