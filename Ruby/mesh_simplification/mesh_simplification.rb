# Copyright 2016 Trimble Inc
# Licensed under the MIT license

require 'sketchup.rb'
require 'extensions.rb'

module Examples

    unless file_loaded?(__FILE__)
      puts File.dirname(__FILE__);
      ex = SketchupExtension.new('simplify_mesh', 'simplify_mesh/main')
      ex.description = 'SketchUp Ruby API Simplify Mesh.'
      ex.version     = '1.0.0'
      ex.copyright   = 'Trimble Navigations © 2024'
      ex.creator     = 'SketchUp'
      Sketchup.register_extension(ex, true)
      file_loaded(__FILE__)
    end

end # module Mesh_simplification
