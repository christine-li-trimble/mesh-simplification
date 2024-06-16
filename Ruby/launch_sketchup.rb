# This file should be required by a file that has defined `RELEASE` to be the
# name of the build folder where the binaries are located.
module Examples

configuration, platform = ARGV[0].split(':')

if SKETCHUP_CONSOLE.respond_to?(:show)
  SKETCHUP_CONSOLE.show
else
  Sketchup.send_action("showRubyPanel:")
end

pro_status = Sketchup.is_pro? ? "Pro" : ""
puts "SketchUp #{pro_status} #{Sketchup.version}"
puts "Loading '#{configuration}' (#{platform}) build..."

ruby_path = File.dirname(__FILE__)
project_path = File.expand_path( File.join(ruby_path, '..') )
mesh_ruby_path = File.expand_path(File.join(ruby_path, 'mesh_simplification'))
if platform == 'x64'
  binary_path = File.join(project_path, configuration, platform)
else
  binary_path = File.join(project_path, configuration)
end

puts ruby_path
puts project_path
puts binary_path

pattern = File.join(binary_path, "*.{so,bundle}")
Dir.glob(pattern).each { |library|
  puts "Requiring #{library}"
  require library
}





  # # Finds and returns the filename for each of the root .rb files in the
  # # examples folder.
  # #
  # # @yield [String] filename
  # #
  # # @return [Array<String>] files
  # def self.rb_files(include_subfolders = false)
  #   examples_path = File.join(__dir__, 'examples')
  #   folders = include_subfolders ? '**' : '*'
  #   examples_pattern = File.join(examples_path, folders, '*.rb')
  #   Dir.glob(examples_pattern).each { |filename|
  #     yield filename
  #   }
  # end

 
  # # This runs when this file is loaded and adds the location of each of the
  # # tutorials folders to the load path such that the tutorials can be loaded
  # # into SketchUp directly from the repository.
  # self.rb_files { |filename|
  #   begin
  #     path = File.dirname(filename)
  #     $LOAD_PATH << path
  #     require filename
  #   rescue LoadError => error
  #     warn "Failed to load: #{filename}"
  #     warn error.inspect
  #     warn error.description
  #   end
  # }




  mesh_rubyfiles = File.join(mesh_ruby_path, '**', "*.rb")
  Dir.glob(mesh_rubyfiles).each { |rubyfile|
    puts "Requiring #{rubyfile}"
    require rubyfile
  }


end # module Mesh_simplification