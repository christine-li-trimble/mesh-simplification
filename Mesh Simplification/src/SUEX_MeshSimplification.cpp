
#include <algorithm>
#include <iostream>
#include <vector>

#include "RubyUtils/RubyUtils.h"
#include <SketchUpAPI/sketchup.h>
#include <SketchUpAPI/common.h>
#include <SketchUpAPI/model/defs.h>
#include <SketchupAPI/application/ruby_api.h>
#include "decimate_qem.h"

VALUE hello_world() {
	return GetRubyInterface("Hello World!");
}

VALUE take_input() {
	SUModelRef model = SU_INVALID;
	// Get a reference to the model in the currently running SketchUp.
	SUApplicationGetActiveModel(&model);

	if (SUIsInvalid(model)) {
		return Qnil;
	}

	SUEntitiesRef entities = SU_INVALID;
	// Get a collection of all the entities at the top level of the model.
	// SUModelGetEntities(model, &entities);
	SUModelGetActiveEntities(model, &entities);

	size_t num_components = 0;
	size_t count = 0;
	size_t num_faces = 0;
	// Get the number of groups in the collection of entities.
	// Not needed for this example use-case, but just showing some more apis.
	size_t num_groups = 0;
	SUEntitiesGetNumGroups(entities, &num_groups);
	rb_warn("Number of groups %d", (int)num_groups);

	std::vector<SUGroupRef> groups(num_groups);
	SUEntitiesGetGroups(entities, num_groups, &groups[0], &count);


	SUEntitiesRef group_entities = SU_INVALID;
	SUGroupGetEntities(groups[0], &group_entities);

	SUEntitiesGetNumFaces(group_entities, &num_faces);
	rb_warn("Number of faces %d", (int)num_faces);
	if (num_faces == 0) {
		rb_warn("No faces found at the top level of the model %d");
		return Qnil;
	}

	std::vector<SUFaceRef> faces(num_faces);
	count = 0;

	// From the collection of entities, let's get the faces...
	SUEntitiesGetFaces(group_entities, num_faces, &faces[0], &count);


	std::vector<long> faces_indices;
	std::vector<vertex> vertices;
	for (auto face : faces) {
		
		size_t count_verts = 0;
		size_t count_verts_used = 0;
		size_t count_triangles = 0;
		SUFaceGetNumVertices(face, &count_verts);
		std::vector<SUVertexRef> temp_vertices(count_verts);
		SUFaceGetVertices(face, count_verts, &temp_vertices[0], &count_verts_used);
		for (int i = 0; i < 3; i++)
		{
			SUPoint3D position;
			SUVertexGetPosition(temp_vertices[i], &position);
			vertex temp = { position.x, position.y, position.z };
			std::vector<vertex>::iterator it = std::find(vertices.begin(), vertices.end(), temp);
			if (it == vertices.end())
			{
				vertices.push_back(temp);
				faces_indices.push_back(static_cast<long>(vertices.size()-1));
			}
			else
			{
				faces_indices.push_back(static_cast<long>(it-vertices.begin()));
			}
		}
	}

	// Decimation QEM Function
	// TODO : Test the implemenation of the decimation function
	long num_target_vertices = static_cast < long>(vertices.size()-1);
	decimate_qem(faces_indices, vertices, num_target_vertices,
	             500, 1.0, 1.e-6, true);

	// Create an array of faces. Each entry will be an array of vertices
	// TODO : Test the implemenation of the adding the vertices to the faces
	long num_faces_indices = static_cast<long>(faces_indices.size());
	long num_decimated_faces = static_cast<long>(faces_indices.size() / 3);
	VALUE ruby_new_faces = rb_ary_new_capa(num_decimated_faces);
	for (long i = 0; i < num_decimated_faces; i++) {

		VALUE ruby_vertices = rb_ary_new_capa(3);
		
		for (int j = 0; j < 3; j++)
		{
			SUPoint3D position{ vertices[faces_indices[i * 3 + j]].x,
							   vertices[faces_indices[i * 3 + j]].y,
							   vertices[faces_indices[i * 3 + j]].z };
			VALUE ruby_position = rb_ary_new_capa(3);
			rb_ary_push(ruby_position, DBL2NUM(position.x));
			rb_ary_push(ruby_position, DBL2NUM(position.y));
			rb_ary_push(ruby_position, DBL2NUM(position.z));
			rb_ary_push(ruby_vertices, ruby_position);
		}
		rb_ary_push(ruby_new_faces, ruby_vertices);
	}



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
	rb_define_module_function(mSUEX_MeshSimplification, "take_input", VALUEFUNC(take_input), 0);
}
