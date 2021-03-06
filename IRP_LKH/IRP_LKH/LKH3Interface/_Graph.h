////////////////////////////////
/// usage : 1.	topological or geometrical graph representation.
/// 
/// note  : 1.	
////////////////////////////////

#ifndef SMART_SZX_GOAL_LKH__GRAPH_H
#define SMART_SZX_GOAL_LKH__GRAPH_H


#include <initializer_list>

#include "_Arr.h"


namespace szx {

	struct Graph {
		using ID = int;
		using DefaultWeightType = int;
		using DefaultCapacityType = int;
		using DefaultCoordType = int;


		// graph elements.
		struct AdjNode { // adjacent node.
			AdjNode() {}
			AdjNode(ID dstNodeId) : dst(dstNodeId) {}

			ID dst;
		};

		template<typename Weight = DefaultWeightType>
		struct WeightedAdjNode : public virtual AdjNode {
			WeightedAdjNode() {}
			WeightedAdjNode(ID dstNodeId, Weight edgeWeight) : AdjNode(dstNodeId), weight(edgeWeight) {}
			WeightedAdjNode(const AdjNode &adjNode, Weight edgeWeight) : AdjNode(adjNode), weight(edgeWeight) {}

			Weight weight;
		};

		template<typename Capacity = DefaultCapacityType>
		struct CapacitatedAdjNode : public virtual AdjNode {
			CapacitatedAdjNode() {}
			CapacitatedAdjNode(ID dstNodeId, Capacity edgeCapacity) : AdjNode(dstNodeId), capacity(edgeCapacity) {}
			CapacitatedAdjNode(const AdjNode &adjNode, Capacity edgeCapacity) : AdjNode(adjNode), capacity(edgeCapacity) {}

			Capacity capacity;
		};

		template<typename Weight = DefaultWeightType, typename Capacity = DefaultCapacityType>
		struct WeightedCapacitatedAdjNode : public virtual WeightedAdjNode<Weight>, public virtual CapacitatedAdjNode<Capacity> {
			WeightedCapacitatedAdjNode() {}
			//WeightedCapacitatedAdjNode(ID dstNodeId, Weight edgeWeight, Capacity edgeCapacity)
			  //  : AdjNode(dstNodeId), weight(edgeWeight), capacity(edgeCapacity) {}
			//WeightedCapacitatedAdjNode(const AdjNode &adjNode, Weight edgeWeight, Capacity edgeCapacity)
			  //  : AdjNode(adjNode), weight(edgeWeight), capacity(edgeCapacity) {}
		};


		struct Edge {
			Edge() {}
			Edge(ID srcNodeId, ID dstNodeId) : src(srcNodeId), dst(dstNodeId) {}

			ID src;
			ID dst;
		};

		template<typename Weight = DefaultWeightType>
		struct WeightedEdge : public virtual Edge {
			WeightedEdge() {}
			WeightedEdge(ID srcNodeId, ID dstNodeId, Weight edgeWeight) : Edge(srcNodeId, dstNodeId), weight(edgeWeight) {}
			WeightedEdge(const Edge &edge, Weight edgeWeight) : Edge(edge), weight(edgeWeight) {}

			Weight weight;
		};

		template<typename Capacity = DefaultCapacityType>
		struct CapacitatedEdge : public virtual Edge {
			CapacitatedEdge() {}
			CapacitatedEdge(ID srcNodeId, ID dstNodeId, Capacity edgeCapacity) : Edge(srcNodeId, dstNodeId), capacity(edgeCapacity) {}
			CapacitatedEdge(const Edge &edge, Capacity edgeCapacity) : Edge(edge), capacity(edgeCapacity) {}

			Capacity capacity;
		};

		template<typename Weight = DefaultWeightType, typename Capacity = DefaultCapacityType>
		struct WeightedCapacitatedEdge : public virtual WeightedEdge<Weight>, public virtual CapacitatedEdge<Capacity> {
			WeightedCapacitatedEdge() {}
			//WeightedCapacitatedEdge(ID srcNodeId, ID dstNodeId, Weight edgeWeight, Capacity edgeCapacity)
			  //  : Edge(srcNodeId, dstNodeId), weight(edgeWeight), capacity(edgeCapacity) {}
			//WeightedCapacitatedEdge(const Edge &edge, Weight edgeWeight, Capacity edgeCapacity)
			  //  : Edge(edge), weight(edgeWeight), capacity(edgeCapacity) {}
		};


		template<typename Coord = DefaultCoordType>
		struct Coord2D {
			Coord2D() {}
			Coord2D(Coord xCoord, Coord yCoord) : x(xCoord), y(yCoord) {}

			Coord x;
			Coord y;
		};

		template<typename Coord = DefaultCoordType>
		struct Coord3D : public Coord2D<Coord> {
			Coord3D() {}
			Coord3D(Coord xCoord, Coord yCoord, Coord zCoord) : Coord2D<Coord>(xCoord, yCoord), z(zCoord) {}
			Coord3D(const Coord2D<Coord> &coord2D, Coord zCoord) : Coord2D<Coord>(coord2D), z(zCoord) {}

			Coord z;
		};


		// graph representations.
		template<typename T = DefaultWeightType> // `T` can be weight, capacity, both of them or any other adjacency info.
		using AdjMat = Arr2D<T>; // adjacency matrix. `adjMat[n][m]` is the `T` on the edge from node `n` to node `m`.

		template<typename AdjNodeType = WeightedAdjNode<>>
		using AdjList = Arr<List<AdjNodeType>>; // adjacency list. `adjList[n][i]` is the `i`_th adjacent node to node `n`.

		template<typename EdgeType = WeightedEdge<>>
		using EdgeList = List<EdgeType>;

		template<typename CoordType = Coord2D<>>
		using CoordList = List<CoordType>;

		using Path = List<ID>; // `path[i]` is the `i`_th node visited in the path.
		using Tour = List<ID>; // `tour[i]` is the `i`_th node visited in the tour. the first and last item should be the same node.
	};

}


#endif // SMART_SZX_GOAL_LKH__GRAPH_H
