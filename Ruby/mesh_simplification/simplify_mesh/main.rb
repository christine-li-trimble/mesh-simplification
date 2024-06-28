# Copyright 2016 Trimble Inc
# Licensed under the MIT license

require 'sketchup.rb'

module Examples

    def self.readentity(entity)
      #puts entity.typename
      if entity.is_a? Sketchup::Group 
        puts entity.typename
        entity.entities.each { |subentity|
          self.readentity(subentity)
        }
      elsif entity.is_a? Sketchup::ComponentInstance
        puts entity.typename
        definition = entity.definition
        definition.entities.each { |subentity|
           readentity(subentity)
        }
      elsif entity.is_a? Sketchup::Face
        $face_count += 1
      elsif entity.is_a? Sketchup::Edge
        $edge_count += 1
      end
    end

    def self.readmesh
      model = Sketchup.active_model
      
      model.start_operation('Create Cube', true)
      group = model.active_entities.add_group
      entities = group.entities
      points = [
        Geom::Point3d.new(0,   0,   0),
        Geom::Point3d.new(1.m, 0,   0),
        Geom::Point3d.new(1.m, 1.m, 0),
        Geom::Point3d.new(0,   1.m, 0)
      ]
      face = entities.add_face(points)
      face.pushpull(-1.m)
      model.commit_operation

      selection = Sketchup.active_model.selection
      $face_count = 0
      $edge_count = 0
      
      # Look at all of the entities in the selection.
      selection.each { |entity|
        # puts entity.typename
        self.readentity(entity)   
      }
      
      UI.messagebox("There are " + $face_count.to_s + " faces selected." +
                    "There are " + $edge_count.to_s + " edges selected.")
      
      # mesh = face.mesh
      # puts mesh.count_polygons
      # puts mesh.count_points

      puts SUEX_MeshSimplification::hello_world();
    end

    unless file_loaded?(__FILE__)
      menu = UI.menu('Plugins')
      menu.add_item('01 Mesh simplification') {
        self.readmesh
      }
      file_loaded(__FILE__)
    end

end # module Mesh_simplification
