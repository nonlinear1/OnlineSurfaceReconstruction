/*
	This file is part of the implementation for the technical paper

		Field-Aligned Online Surface Reconstruction
		Nico Schertler, Marco Tarini, Wenzel Jakob, Misha Kazhdan, Stefan Gumhold, Daniele Panozzo
		ACM TOG 36, 4, July 2017 (Proceedings of SIGGRAPH 2017)

	Use of this source code is granted via a BSD-style license, which can be found
	in License.txt in the repository root.

	@author Nico Schertler
*/

#pragma once

#include "osr/common.h"

#include <vector>
#include <array>
#include <cstdint>

namespace osr
{
	namespace ExtractionHelper
	{
		struct Vertex;
		struct Edge;
		struct Triangle;
		struct Quad;

		class OSR_EXPORT IEntityContainer
		{
		public:
			virtual void getInterpolatedPositionNormal(const Vertex& v, int localIndex, Vector3f& out_position, Vector3f& out_normal) const = 0;
			virtual void getInterpolatedPositionNormal(const Edge& e, int localIndex, Vector3f& out_position, Vector3f& out_normal) const = 0;
			virtual void getInterpolatedPositionNormal(const Triangle& t, int localIndex, Vector3f& out_position, Vector3f& out_normal) const = 0;
			virtual void getInterpolatedPositionNormal(const Quad& q, int localIndex, Vector3f& out_position, Vector3f& out_normal) const = 0;
		};

		//Base class for geometric primitives from which the final mesh is constructed.
		struct OSR_EXPORT Entity
		{
			//Specifies the index of the entity's first texel within a sequence of all texels of the mesh.
			uint32_t indexInTexelVector;

			//The number of the extraction process in which this entity has been created.
			uint8_t generation;

			Entity() : generation(255) { }

			//Accesses the entity's texel with the given local index.
			virtual const Vector4f& texel(int localIndex) const = 0;

			virtual void getInterpolatedPositionNormal(const IEntityContainer& container, int localIndex, Vector3f& out_position, Vector3f& out_normal) const = 0;
		};

		struct OSR_EXPORT Triangle : public Entity
		{
			static const int FaceDegree = 3;

			std::array<int32_t, 3> edges; //reference to edges; negative if referencing inverse edge direction
			std::vector<Vector4f> colorDisplacement;

			const Vector4f& texel(int localIndex) const { return colorDisplacement[localIndex]; }

			void getInterpolatedPositionNormal(const IEntityContainer& container, int localIndex, Vector3f& out_position, Vector3f& out_normal) const
			{
				container.getInterpolatedPositionNormal(*this, localIndex, out_position, out_normal);
			}
		};
		struct OSR_EXPORT Quad : public Entity
		{
			static const int FaceDegree = 4;

			std::array<int32_t, 4> edges; //reference to edges; negative if referencing inverse edge direction
			std::vector<Vector4f> colorDisplacement;

			const Vector4f& texel(int localIndex) const { return colorDisplacement[localIndex]; }

			void getInterpolatedPositionNormal(const IEntityContainer& container, int localIndex, Vector3f& out_position, Vector3f& out_normal) const
			{
				container.getInterpolatedPositionNormal(*this, localIndex, out_position, out_normal);
			}
		};
		struct OSR_EXPORT Edge : public Entity
		{
			std::array<uint32_t, 2> v; //reference to vertices
			std::vector<Vector4f> colorDisplacement;
			std::vector<uint32_t> incidentTriangles;
			std::vector<uint32_t> incidentQuads;

			const Vector4f& texel(int localIndex) const { return colorDisplacement[localIndex]; }

			void getInterpolatedPositionNormal(const IEntityContainer& container, int localIndex, Vector3f& out_position, Vector3f& out_normal) const
			{
				container.getInterpolatedPositionNormal(*this, localIndex, out_position, out_normal);
			}
		};
		struct OSR_EXPORT Vertex : public Entity
		{
			Vector3f position, normal;
			Vector4f colorDisplacement;
			std::vector<size_t> incidentEdges;

			const Vector4f& texel(int) const { return colorDisplacement; }

			void getInterpolatedPositionNormal(const IEntityContainer& container, int localIndex, Vector3f& out_position, Vector3f& out_normal) const
			{
				container.getInterpolatedPositionNormal(*this, localIndex, out_position, out_normal);
			}
		};

		//Represents one constituent source for the interpolation of a given position on a face.
		struct FaceInterpolationInfo
		{
			//The entity whose texel is used.
			const ExtractionHelper::Entity* entity;

			//The local index of the texel within the entity.
			int localTexelIndex;

			//The weight of this texel for the interpolation.
			float weight;
		};
	}
}