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

      puts SUEX_MeshSimplification::hello_world();
    end

    def self.SimplifyMesh
      model = Sketchup.active_model
      
      model.start_operation('Create Face', true)
      entities = model.entities
      points = [
        Geom::Point3d.new(0,   0,   0),
        Geom::Point3d.new(1.m, 0,   0),
        Geom::Point3d.new(1.m, 1.m, 0),
        Geom::Point3d.new(0,   1.m, 0)
      ]
      face = entities.add_face(points)
      model.commit_operation

      selection = Sketchup.active_model.selection

      out_entity = SUEX_MeshSimplification::take_input(selection) #Call C code, get a entity back

      model.start_operation('CreateFaceFromSLAPIOutput', true)
      group = model.active_entities.add_group
      group.name = "Output"
      #Move it so it's not in the same position.
      group.transformation = Geom::Transformation.new([-100,-100,0])
      entities = group.entities
      entities.build { |builder|
          builder.add_face(out_entity)
      }
      model.commit_operation
    end

    unless file_loaded?(__FILE__)
      menu = UI.menu('Plugins')
      menu.add_item('01 Mesh simplification') {
        self.SimplifyMesh
      }
      file_loaded(__FILE__)
    end

end # module Mesh_simplification
