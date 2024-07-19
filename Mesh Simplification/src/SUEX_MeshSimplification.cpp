
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
	SUApplicationGetActiveModel(&model);

	if (SUIsInvalid(model)) {
		return Qnil;
	}

	SUEntitiesRef entities = SU_INVALID;
	SUModelGetEntities(model, &entities);

	size_t num_faces = 0;
	SUEntitiesGetNumGroups(entities, &num_faces);
	rb_warn("Number of groups %d", (int)num_faces);

	std::vector<SUFaceRef> faces(1);
	size_t count = 0;
	SUEntitiesGetFaces(entities, 1, &faces[0], &count);
	std::vector<SUVertexRef> vertices(3);
	SUFaceGetVertices(faces[0], 3, &vertices[0], &count);

	VALUE ruby_vertices = rb_ary_new_capa(static_cast<long>(vertices.size()));
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
