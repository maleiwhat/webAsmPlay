/***************************************************************************
 *   Copyright (C) 2006 by Matthew Tang                                    *
 *   trailcode@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef __BSP_NODE_H_
#define __BSP_NODE_H_

#include <vector>

#include <tceGeom/AABB3D.h>

namespace tce
{
	namespace geom
	{

		struct BSPNode
		{

			int planeIndex;
			int frontIndex;
			int backIndex;
			int parentIndex;

			AABB3Df boundingBox;

			BSPNode() : planeIndex(-1), frontIndex(-1), backIndex(-1), parentIndex(-1), boundingBox() { ;}

			BSPNode(int _parentIndex) : planeIndex(-1), frontIndex(-1), backIndex(-1), parentIndex(_parentIndex), boundingBox() { ;}

		};

		/**
		 * Template node container wrapper, provides a constant and normal iterator 
		 * to the container.
		 */
		template <typename Container>
		class BSPNodeContainer
		{
			public:

				BSPNodeContainer() : container() { ;}

				virtual ~BSPNodeContainer() { ;}

				/**
				*	Provides access to the iterator of the container
				*/
				typedef typename Container::iterator iterator;
	
				/**
				* Provides access to the const_iterator of the container
				*/
				typedef typename Container::const_iterator const_iterator;

				typedef typename Container::reverse_iterator reverse_iterator;	

				typedef typename Container::const_reverse_iterator const_reverse_iterator;

				typedef typename Container::size_type size_type;

				typedef typename Container::value_type value_type;

				/**
				* stl container member functions
				*/

				inline iterator begin() { return container.begin() ;}

				inline const_iterator begin() const { return container.begin() ;}

				inline iterator end() { return container.end() ;}

				inline const_iterator end() const { return container.end() ;}

				inline reverse_iterator rbegin() { return container.rbegin() ;}

				inline const_reverse_iterator rbegin() const { return container.rbegin() ;}

				inline reverse_iterator rend() { return container.rend() ;}

				inline const_reverse_iterator rend() const { return container.rend() ;}

				inline bool empty() const { return container.empty() ;}

				inline size_type size() const { return container.size() ;}

				inline const value_type & operator[] (const unsigned int i) const { return container[i] ;}

				inline value_type & operator[] (const unsigned int i) { return container[i] ;}

				inline void push_back(const value_type poly) { container.push_back(poly) ;}

			private:

				Container container;
		};

		typedef BSPNodeContainer < std::vector < BSPNode * > > BSPNodeVector;

	}
}
#endif

