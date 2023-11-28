install_headers('tmio.h')

tmio_sources = files('tmio.c')

tmio_lib = library('tmio',
  tmio_sources,
  include_directories : tmio_inc,
  dependencies : bufio_dep,
  install : true
)