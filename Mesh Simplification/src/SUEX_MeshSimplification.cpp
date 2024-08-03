
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
	SUEntitiesGetNumGroups(entities, &num_faces);
	rb_warn("Number of groups %d", (int)num_faces);

	std::vector<SUFaceRef> faces(1);
	size_t count = 0;
	// From the collection of entities, let's get the faces...
	SUEntitiesGetFaces(entities, 1, &faces[0], &count);
	std::vector<SUVertexRef> vertices(3);
	// ... and get the first 3 vertices to send back to ruby so it can make a
	// new face.
	SUFaceGetVertices(faces[0], 3, &vertices[0], &count);

	VALUE ruby_vertices = rb_ary_new_capa(static_cast<long>(vertices.size()));
	// Iterate through the vertices, adding them to the ruby_vertices container,
	// so we can send them back.
	for (auto v : vertices) {
		SUEntityRef entity = SUVertexToEntity(v);
		VALUE ruby_vertex = Qnil;
		SUEntityToRuby(entity, &ruby_vertex);
		rb_ary_push(ruby_vertices, ruby_vertex);
	}

	// Send out the vertices after doing geometry processing.
	return ruby_vertices;
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
