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
using namespace szx;

// testing and sample code.
void testCoordList2D() {
	Graph::CoordList<> cl({
		{ 0, 0 },
		{ 100, 200 },{ 200, 100 },
		{ 200, 300 },{ 300, 200 },
		});
	List<int> nl({ 0,1,2 });
	TspCache_BinTreeImpl tsp("test5", cl);
	tsp.readFileToCache(".\\LKH3Interface\\tour");

	Graph::Tour sln;
	std::cout << std::endl << tsp.solve(sln, nl, 1) << std::endl;
	for (auto n : sln) {
		std::cout << n << " ";
	}
	std::cout << std::endl;
	// if (solve(sln, cl,1)) {
	// EXTEND[szx][0]: record solution path.
	//}
}


int main(int argc, char *argv[]) 
{
	testCoordList2D();
	//testCoordList3D();
	//testAdjMat();
	//testWeightedAdjList();
	//testWeightedEdgeList();
	system("pause");
}
