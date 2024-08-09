
#include <algorithm>
#include <iostream>
#include <vector>

#include "RubyUtils/RubyUtils.h"
#include <SketchUpAPI/sketchup.h>
#include <SketchUpAPI/common.h>
#include <SketchUpAPI/model/defs.h>
#include <SketchupAPI/application/ruby_api.h>

VALUE hello_world() {
	return GetRubyInterface("Hello World!");
}


class Halfedge
{
public:
	int startVertex;
	int finalVertex;
	int face;
	bool isBoundary;

	int next;
	int twin;

	Halfedge(int startV, int finalV, int f, bool bound, int n, int t = -1): 
		startVertex{startVert}, finalVertex{finalV}, face{f}, isBoundary{bound}, next{n}, twin{t}
	{}

	//void setNext(int n) { next = n; }
	void setTwin(int t) { twin = t; }
};

VALUE take_input(void* input) {
	SUModelRef model = SU_INVALID;
	// Get a reference to the model in the currently running SketchUp.
	SUApplicationGetActiveModel(&model);

	if (SUIsInvalid(model)) {
		return Qnil;
	}

	SUEntitiesRef entities = SU_INVALID;
	// Get a collection of all the entities at the top level of the model.
	SUModelGetEntities(model, &entities);

	size_t num_faces = 0;
	// Get the number of groups in the collection of entities.
	// Not needed for this example use-case, but just showing some more apis.
	SUEntitiesGetNumFaces(entities, &num_faces);
	rb_warn("Number of faces %d", (int)num_faces);

	if (num_faces == 0) {
		rb_warn("No faces found at the top level of the model %d");
		return Qnil;
	}

	std::vector<SUFaceRef> faces(num_faces);
	size_t count = 0;
	// From the collection of entities, let's get the faces...
	SUEntitiesGetFaces(entities, num_faces, &faces[0], &count);

	// Create an array of faces. Each entry will be an array of vertices
	VALUE ruby_new_faces = rb_ary_new_capa(static_cast<long>(faces.size()));

	std::vector<Halfedge> halfedges;

	int faceIndex = 0;

	for (auto face : faces) {
		std::vector<SUVertexRef> vertices(3);
		size_t count_verts = 0;
		size_t count_verts_used = 0;
		SUFaceGetNumVertices(face, &count_verts);
		// ... and get the first 3 vertices to send back to ruby so it can make a
		// new face.
		SUFaceGetVertices(face, 3, &vertices[0], &count_verts_used);
		rb_warn("Number of verts in face %d (truncated to %d)", (int)count_verts, count_verts_used);
		VALUE ruby_vertices = rb_ary_new_capa(static_cast<long>(vertices.size()));

		// Iterate through the vertices, adding them to the ruby_vertices container, which in turn
		// is put into the ruby_new_faces container to be sent back to Ruby.
		for (auto v : vertices) {
			SUEntityRef entity = SUVertexToEntity(v);
			VALUE ruby_vertex = Qnil;
			SUEntityToRuby(entity, &ruby_vertex);
			rb_ary_push(ruby_vertices, ruby_vertex);
		}
		
		// How to get v0. v1 and v2? Dict from SUVertexRef to global index?
		int localOffset = halfedges.size();
		Halfedge he(v0, v1, faceIndex, false, localOffset + 1);
		Halfedge he(v1, v2, faceIndex, false, localOffset + 2);
		Halfedge he(v2, v0, faceIndex, false, localOffset + 0);
		halfedges.push_back(he);

		++faceIndex;
		// this shouldn't be here...
		rb_ary_push(ruby_new_faces, ruby_vertices);
	}

	// TODO: create twins and borders...

	// Send out the array of arrays of vertices after doing geometry processing.
	return ruby_new_faces;
}

VALUE ruby_platform() {
	return GetRubyInterface(RUBY_PLATFORM);
}

// Load this module from Ruby using:
//   require 'SUEX_MeshSimplification'
extern "C"
void Init_SUEX_MeshSimplification()
{
	VALUE mSUEX_MeshSimplification = rb_define_module("SUEX_MeshSimplification");
	rb_define_const(mSUEX_MeshSimplification, "CEXT_VERSION", GetRubyInterface("1.0.0"));
	rb_define_module_function(mSUEX_MeshSimplification, "hello_world", VALUEFUNC(hello_world), 0);
	rb_define_module_function(mSUEX_MeshSimplification, "ruby_platform", VALUEFUNC(ruby_platform), 0);
	rb_define_module_function(mSUEX_MeshSimplification, "take_input", VALUEFUNC(take_input), 1);
}
