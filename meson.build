
project('tmio', 'c', version:'1.0.0',
  license:'MPL-2.0',
  license_files: 'LICENSE',
  default_options : [
    'warning_level=3',
    'c_std=c99',
    'buildtype=release',
    'default_library=both',
    'prefer_static=true',
  ]
)

bufio_dep = dependency('bufio', fallback : ['bufio', 'bufio_dep'])

tmio_inc = include_directories('src')

subdir('src')

tmio_dep = declare_dependency(include_directories : tmio_inc, link_with : tmio_lib, dependencies : [bufio_dep])

subdir('examples')

pkg_mod = import('pkgconfig')
pkg_mod.generate(tmio_lib)

