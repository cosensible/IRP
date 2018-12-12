////////////////////////////////
/// usage : 1.	cache for precomputed TSP optima.
/// 
/// note  : 1.	
////////////////////////////////

#ifndef SMART_SZX_GOAL_LKH__CACHE_H
#define SMART_SZX_GOAL_LKH__CACHE_H


#include <functional>
#include <bitset>
#include <map>
#include <set>
#include <string>
#include <io.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>

#include "_Graph.h"


// get the names of all files under `path`
void GetAllFiles(std::string path, std::vector<std::string>& files)
{
	long   hFile = 0;
	struct _finddata_t fileinfo;	// file info
	std::string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			if ((fileinfo.attrib &  _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
				{
					files.push_back(p.assign(path).append("\\").append(fileinfo.name));
					GetAllFiles(p.assign(path).append("\\").append(fileinfo.name), files);
				}
			}
			else
			{
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}

		} while (_findnext(hFile, &fileinfo) == 0);

		_findclose(hFile);
	}
}

// get the names of all files with extension `format` under `path`
void GetAllFormatFiles(std::string path, std::vector<std::string>& files, std::string format)
{
	long   hFile = 0;
	struct _finddata_t fileinfo;
	std::string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*" + format).c_str(), &fileinfo)) != -1)
	{
		do
		{
			if ((fileinfo.attrib &  _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
				{
					//files.push_back(p.assign(path).append("\\").append(fileinfo.name) );
					GetAllFormatFiles(p.assign(path).append("\\").append(fileinfo.name), files, format);
				}
			}
			else
			{
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);

		_findclose(hFile);
	}
}

namespace szx {

struct TspCacheBase {
    using ID = Graph::ID;
    using NodeList = List<ID>; // the `nodes` should be ordered in increasing order during the traverse.
    using NodeSet = List<bool>; // the size of the set should be the same as the node number. // OPTIMIZE[szx][5]: use bitset?
    using TraverseEvent = std::function<bool(const Graph::Tour&)>;
	using TourAndCost = std::pair<NodeList, double>;

	TspCacheBase() = default;
	TspCacheBase(const std::string &name) :insName(name) {}
	TspCacheBase(const std::string &name, const Graph::CoordList<> &coordList) :insName(name), coordList(coordList) {
		std::cout << "Initialize success. Coord list is : " << std::endl;
		for (auto cd : coordList) {
			std::cout << cd.x << "  " << cd.y << std::endl;
		}
		std::cout << "-----------------------" << std::endl;
	}
    //TspCacheBase(ID nodeNumber) : nodeNum(nodeNumber) {}


    // return a non-empty tour if there is cached solution for such node set, otherwise cache miss happens.
    // if the returned tour is `tour`, call `tour.empty()` to check the status.
    virtual const Graph::Tour& get(const NodeSet &containNode) const = 0;
	
	// `nodes` is the set of nodes in the tour and it is pre-sorted 
    virtual TourAndCost get(const NodeList &nodes) const = 0;

    // return true is overwriting happens, otherwise a new entry is added.
	virtual bool set(const Graph::Tour &sln, int tourCost) = 0;
    // the node set in `sln` and `nodes` should be the same.
    // return true is overwriting happens, otherwise a new entry is added.
    virtual bool set(const Graph::Tour &sln, const NodeList &nodes) = 0;
    virtual bool set(const Graph::Tour &sln, const NodeSet &containNode) = 0;

    // `onTour` return true if the traverse should be breaked, otherwise the loop will continue.
    // return true if no break happens.
    virtual bool forEach(TraverseEvent onTour) const = 0;

	// read all TSP results from disk
	virtual void readFileToCache(const std::string &cachePath) = 0;

	virtual void printCache() const = 0;
protected:
    //ID nodeNum;

	List<TourAndCost> toursCache;
	std::vector<std::string> tourNames;	// file names of all tour with precomputed optimal
	Graph::CoordList<> coordList;	// coordinates of all nodes in current instance
	std::string insName;	// name of current instance
};


struct TspCache_BinTreeImpl : public TspCacheBase {
    using TspCacheBase::TspCacheBase;


    virtual const Graph::Tour& get(const NodeSet &containNode) const override {
        return Graph::Tour();
    }

    virtual TourAndCost get(const NodeList &nodes) const override {
        //NodeSet containNode(nodeNum, false);
        //for (auto n = nodes.begin(); n != nodes.end(); ++n) { containNode[*n] = true; }
        //return get(containNode);
		if (nodes.size() > 1) {
			for (auto cache : toursCache) {
				NodeList tour(cache.first);
				sort(tour.begin(), tour.end());
				if (tour == nodes) { return cache; }
			}
		}
		return TourAndCost();
    }

	virtual bool set(const Graph::Tour &sln, int cost) override {
		if (sln.size() > 1 && cost > 0) {
			toursCache.push_back({ sln,cost });
			return true;
		}
		return false;
	}
    virtual bool set(const Graph::Tour &sln, const NodeList &nodes) override {
        return false;
    }
    virtual bool set(const Graph::Tour &sln, const NodeSet &containNode) override {
        return false;
    }

    virtual bool forEach(std::function<bool(const Graph::Tour&)> onCacheEntry) const override {
        //for each cache entry {
        //    if (onCacheEntry(entry.tour)) { return false; }
        //}
		return false;
    }

	virtual void readFileToCache(const std::string &cachePath) override {
		toursCache.clear();
		GetAllFormatFiles(cachePath, tourNames, ".tour");
		for (auto t : tourNames) {
			if (t.find(insName) == std::string::npos) { continue; }
			TourAndCost tourAndCost;
			NodeList nl;
			unsigned len = t.find("tour\\") + 5, len1 = t.find("_p");
			std::string map = t.substr(0, len) + t.substr(len, len1 - len) + ".map";
			int round = stoi(t.substr(len1 + 2));
			//std::cout << "round : " << round << std::endl;
			//std::cout << "node map : " << map << std::endl;
			std::ifstream nodemap(map);
			int nodeNumber = -1;
			while (std::getline(nodemap, map)) {
				if (map == "ROUND : " + std::to_string(round)) {
					while (nodemap >> nodeNumber) {
						nl.push_back(nodeNumber);
					}	
					break;
				}
			}
			std::cout << "read nodes : " << std::endl;
			for (auto n : nl) {
				std::cout << n << " ";
			}
			std::cout << std::endl;

			std::ifstream ifTour(t);
			std::string line;
			for (int i = 0; std::getline(ifTour, line); ++i) {
				if (1 == i) { 
					tourAndCost.second = std::stod(line.substr(19)); 
					//std::cout << "tour cost = " << tourAndCost.second << std::endl;
				}
				if (i > 5) {
					if ("-1" == line) { break; }
					tourAndCost.first.push_back(nl[std::stoi(line) - 1]);
				}
			}
			toursCache.push_back(tourAndCost);
			std::cout << "read tour : " << t << " success" << std::endl;
		}
		std::cout << "Initialize cache success." << std::endl;
		std::cout << "------------------------------" << std::endl << std::endl;
	}

	void writeFile(const NodeList &nodeList, int round) {//round代表当前在第几周期
		std::ostringstream vname1;
		std::ostringstream vname;
		std::ostringstream vname2;
		std::ostringstream st;
		vname << ".\\LKH3Interface\\instance\\" << insName << "_p" << round << ".tsp";
		vname1 << ".\\LKH3Interface\\instance\\" << insName << "_p" << round << ".par";
		vname2 << ".\\LKH3Interface\\tour\\" << insName << ".map";

		std::fstream f(vname.str(), std::ios::out);
		std::fstream f1(vname1.str(), std::ios::out);
		std::fstream f2(vname2.str(), std::ios::app);
		if (f.bad())
		{
			std::cout << "打开文件出错" << std::endl;
		}
		int point = 1;
		for (auto n : nodeList) {
			st << point++ << ' ' << coordList[n].x << ' ' << coordList[n].y << std::endl;
		}
		f << "NAME : " << insName << std::endl << "TYPE : TSP\nDIMENSION : " << (point - 1) << "\nEDGE_WEIGHT_TYPE : EUC_2D\nNODE_COORD_SECTION\n" << st.str() << "EOF";
		f.close();
		f1 << "PROBLEM_FILE = " << vname.str() << "\nMOVE_TYPE = 5\nPATCHING_C = 3\nPATCHING_A = 2\nPOPULATION_SIZE=2\nRUNS = 10\nPOPULATION_SIZE=2\nOUTPUT_TOUR_FILE = .\\LKH3Interface\\tour\\" << insName << "_p" << round << ".tour";
		f1.close();
		f2 << "ROUND : " << round << std::endl;
		for (auto n : nodeList) {
			f2 << n << ' ';
		}
		f2 << std::endl;
		std::cout << "write instance success." << std::endl;
	}

	// interface.
	int solve(Graph::Tour &sln, NodeList &nodeList, int round, const Graph::Tour &hintSln = Graph::Tour()) {
		if (!hintSln.empty()) {
			// EXTEND[szx][0]: utilize initial solution.
		}

		sln.clear();
		std::sort(nodeList.begin(), nodeList.end());
		TourAndCost tac = get(nodeList);
		if (!tac.first.empty()) {
			sln = tac.first;
			std::cout << "get optimal success. cost = " << tac.second << std::endl;
			return tac.second;
		}

		std::cout << "get optimal failed." << std::endl;
		// no optimal in cache
		writeFile(nodeList, round);

		std::ostringstream vname;
		vname << ".\\lkh3.exe .\\LKH3Interface\\instance\\" << insName << "_p" << round << ".par";
		system(vname.str().data());
		vname.str("");
		vname << ".\\LKH3Interface\\tour\\" << insName << "_p" << round << ".tour";
		//std::cout<<vname.str();

		int cost;
		std::string line;
		std::ifstream ifTour(vname.str());
		for (int i = 0; std::getline(ifTour, line); ++i) {
			if (1 == i) {
				cost = std::stod(line.substr(19));
				//std::cout << "tour cost = " << tourAndCost.second << std::endl;
			}
			if (i > 5) {
				if ("-1" == line) { break; }
				sln.push_back(nodeList[std::stoi(line) - 1]);
			}
		}
		toursCache.push_back({ sln,cost });
		std::cout << "insert optimal into cache success! " << "cache : " << std::endl;
		printCache();
		// TODO[szx][0]: retrieve solution path.
		return cost;
	}

	virtual void printCache() const {
		for (auto cache : toursCache) {
			for (auto node : cache.first) {
				std::cout << node << " ";
			}
			std::cout << std::endl;
			std::cout << cache.second << std::endl;
			std::cout << std::endl;
		}
	}
};


struct TspCache_TrieImpl : public TspCacheBase {
    
};


struct TspCache_HashImpl : public TspCacheBase {

};


using TspCache = TspCache_BinTreeImpl;

}



#endif // SMART_SZX_GOAL_LKH__CACHE_H
