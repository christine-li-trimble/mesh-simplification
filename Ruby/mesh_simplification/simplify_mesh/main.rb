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

    def self.hello_world
      puts SUEX_MeshSimplification::hello_world();
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

      
    end

    # Function to tessellate a face and print its triangles
    def self.tessellate_face(face)
      return unless face.is_a?(Sketchup::Face)
      
      # Convert the face into a mesh
      mesh = face.mesh
      model = Sketchup.active_model
      entities = model.active_entities
      # printf 'entities size %d', entities.count
      entities.add_faces_from_mesh(mesh)
      # face.erase!
    end

    def self.tessellate_all_faces
      puts "Tessellating faces..."
      model = Sketchup.active_model
      entities = model.entities
      
      # Iterate over entities and process faces
      entities.each do |entity|
        if entity.is_a?(Sketchup::Face)
          tessellate_face(entity)
        end
      end
      model.close_active
    end

    def self.SimplifyMesh
      model = Sketchup.active_model
      
      # model.start_operation('Create Face', true)
      t_group = model.active_entities.add_group
      t_group.name = "Tessellated"
      path = Sketchup::InstancePath.new([t_group])
      model.active_path = path
      tessellate_all_faces
      model.close_active


      # Call the C code.
      out_entity = SUEX_MeshSimplification::take_input() 

      model.start_operation('CreateFacesFromSLAPIOutput', true)
      # In SketchUp, we tend to put geometry in groups/components, otherwise
      # colocated geometry sticks.
      group = model.active_entities.add_group
      group.name = "Output"
      # Move it so it's not in the same position.
      group.transformation = Geom::Transformation.new([-100,-100,0])
      entities = group.entities
      sz = out_entity.length()
    
      puts sz
      entities.build { |builder|
        sz.times { |x|
          # puts x
          # puts out_entity[x]
          # Sometimes we run into an error with add face, if the vertices are not coplanar - not sure why this happens!
          # If we do run into an error, just continue the loop and ignore this face.
          begin
            builder.add_face(out_entity[x])
          rescue
            puts "Error detected - Skipping face"
            next
          end
        }
      }
      model.commit_operation
    end

    unless file_loaded?(__FILE__)
      menu = UI.menu('Plugins')
      menu.add_item('01 Mesh simplification') {
        #self.hello_world
        self.SimplifyMesh
      }
      file_loaded(__FILE__)
    end

end # module Mesh_simplification
