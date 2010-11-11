/*
 *  ExpandNodeParallelFunctor.h
 *  crag
 *
 *  Created by John McFarlane on 11/8/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "core/intOps.h"

#include "smp/ForEach.h"


namespace form
{
	template <typename BIT_TYPE, size_t MAX_NUM_NODES> 
	class SimpleBitMap
	{
	public:
		static unsigned const BYTES_PER_ELEMENT = sizeof(BIT_TYPE);
		static unsigned const BITS_PER_ELEMENT = BYTES_PER_ELEMENT * CHAR_BIT;
		static unsigned const INDEX_SHIFT = Number<BITS_PER_ELEMENT>::FIELD_SIZE;
		static unsigned const NUM_ELEMENTS = (MAX_NUM_NODES + BITS_PER_ELEMENT - 1) / BITS_PER_ELEMENT;
		BIT_TYPE bits [NUM_ELEMENTS];
	};


	template <size_t MAX_NUM_NODES>
	class ExpandNodeParallelFunctor : public smp::Functor
	{
		OBJECT_NO_COPY (ExpandNodeParallelFunctor);

	public:
		ExpandNodeParallelFunctor(NodeBuffer & _node_buffer)
		: node_buffer(_node_buffer)
		, num_expanded(0)
		{
			RecalculateMinScore();
		}
		
		void RecalculateMinScore()
		{
			min_score = node_buffer.GetWorseReplacableQuaternaScore();
		}

		int GetNumExpanded() const
		{
			return num_expanded;
		}
		
		void ResetNumExpanded() 
		{
			num_expanded = 0;
		}
		
		// The first pass, smp::ForEach functor.
		// Gathers a map of the nodes which might be expandable.
		// TODO: I still think this would go better if it was orderer by quaterna.
		// TODO: An early out test based on the initial worst quaterna would really help.
		void operator () (int first_node_index, int last_node_index)
		{
			size_t sbm_index = first_node_index >> BitMap::INDEX_SHIFT;
			BIT_TYPE * sub_bit_map = expandable_nodes_maps.bits + sbm_index;

			Node const * first_node = node_buffer.nodes + first_node_index;
			Node const * last_node = node_buffer.nodes + last_node_index;
			
			for (Node const * node = first_node; node != last_node; ++ sub_bit_map)
			{
				BIT_TYPE bit_cache = 0;
				BIT_TYPE bit = 1;
				do
				{
					Assert(node < last_node);	// Looks like last-first isn't a high enough power of 2.
					
					if (node->IsExpandable())
					{
						bit_cache |= bit;
						++ num_expanded;
					}
					
					++ node;

					bit <<= 1;
					if (bit == 0 || node == last_node)
					{
						break;
					}
				}	while (bit != 0 && node != last_node);
				
				(* sub_bit_map) = bit_cache;
			}
		}
		
		// The second pass, core::for_each_chunk functor.
		void operator() (Node * first_node, Node * last_node)
		{
#if ! defined (NDEBUG)
			int results [2][2] = { { 0, 0 }, { 0, 0 } };
#endif
			
			int first_node_index = first_node - node_buffer.nodes;
			
			size_t sbm_index = first_node_index >> BitMap::INDEX_SHIFT;
			BIT_TYPE * sub_bit_map = expandable_nodes_maps.bits + sbm_index;
			
			for (Node * node = first_node; node < last_node; ++ sub_bit_map)
			{
				BIT_TYPE bit_cache = * sub_bit_map;
				
				if (bit_cache != 0)
				{
					BIT_TYPE bit = 1;
					do
					{
						Assert(node < last_node);	// Looks like last-first isn't a high enough power of 2.

						if (bit_cache & bit)
						{
							if (node->IsExpandable())
							{
								if (node->score > min_score)
								{
									if (node_buffer.ExpandNode(* node)) 
									{
										RecalculateMinScore();
										++ num_expanded;
									}
								}
								else
								{
									Assert (! node_buffer.ExpandNode(* node));
								}
#if ! defined (NDEBUG)
								++ results [true] [true];
							}
							else
							{
								++ results [true] [false];
							}
						}
						else
						{
							if (node->IsExpandable())
							{
								++ results [false] [true];
							}
							else
							{
								++ results [false] [false];
#endif
							}
						}
					
						++ node;
						
						bit <<= 1;
						if (bit == 0 || node == last_node)
						{
							break;
						}
					}	while (bit != 0 && node != last_node);
				}
				else
				{
					node += BitMap::BITS_PER_ELEMENT;
				}
			}
			
			return;
		}
		
		// The second pass, core::for_each_chunk functor.
		/*void operator() (Node & node)
		{
			if (node.IsExpandable()) 
			{
				if (node_buffer.ExpandNode(node)) 
				{
					++ num_expanded;
				}
			}
		}*/
		
	private:
		
		typedef unsigned long BIT_TYPE;
		typedef SimpleBitMap<BIT_TYPE, MAX_NUM_NODES> BitMap;
		BitMap expandable_nodes_maps;	// TODO: consider alignment for this array
		
		NodeBuffer & node_buffer;
		int num_expanded;
		float min_score;
	};
}
