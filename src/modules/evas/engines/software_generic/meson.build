engine_src = files([
  'evas_engine.c',
  'Evas_Engine_Software_Generic.h',
  'Evas_Engine_Software_Shared.h',
  'evas_native_tbm.c',
  'evas_native_dmabuf.c',
  'evas_ector_software_buffer.c',
  'evas_native_common.h',
  'evas_ector_software.h',
])

gen_src = []

pub_eo_files = [
  'evas_ector_software_buffer.eo'
]

subdir('filters')

foreach eo_file : pub_eo_files
  gen_src += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    depfile : eo_file + '.d',
    command : eolian_gen + [ '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                           '-o', 'd:' + join_paths(meson.current_build_dir(), eo_file + '.d'),
                           '-gchd', '@INPUT@'])
endforeach

engine_src += gen_src

engine_deps = [draw, dl]

if get_option('evas-modules') == 'shared' and not evas_force_static.contains(engine)
  shared_module(mod_full_name, engine_src,
        include_directories : config_dir + [engine_include_dir],
        dependencies : [eina, evas_pre] + engine_deps,
        install : true,
        install_dir : mod_install_dir,
        name_suffix : sys_mod_extension
      )
      module_files += join_paths(mod_install_dir, 'lib'+mod_full_name + '.' + sys_mod_extension)
endif
