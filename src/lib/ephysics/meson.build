bullet = dependency('bullet')

ephysics_deps = [evas, bullet]
ephysics_pub_deps = [eina, eo, efl]

ephysics_header_src = [
  'EPhysics.h'
]

ephysics_src = [
  'ephysics_private.h',
  'ephysics_trimesh.h',
  'ephysics_body_materials.h',
  'ephysics_body.cpp',
  'ephysics_camera.cpp',
  'ephysics_constraints.cpp',
  'ephysics_main.cpp',
  'ephysics_quaternion.cpp',
  'ephysics_shape.cpp',
  'ephysics_world.cpp'
]

ephysics_lib = library('ephysics',
    ephysics_src,
    dependencies: ephysics_pub_deps + [m] + ephysics_deps,
    include_directories : config_dir + [include_directories('.')],
    install: true,
    c_args : package_c_args,
    version : meson.project_version()
)

ephysics = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: ephysics_lib,
  sources : pub_eo_file_target + priv_eo_file_target,
  dependencies: ephysics_pub_deps,
)

install_headers(ephysics_header_src,
  install_dir : dir_package_include,
)
