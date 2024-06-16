
#include "RubyUtils/RubyUtils.h"


VALUE hello_world() {
  return GetRubyInterface("Hello World!");
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
}
