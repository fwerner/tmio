cc = meson.get_compiler('c')
m_dep = cc.find_library('m')

tmio_benchmark_simple = executable('tmio_benchmark_simple', ['tmio_benchmark_simple.c', 'timer.c'], include_directories : tmio_inc, link_with : tmio_lib, dependencies : [ bufio_dep, m_dep] )
tmio_benchmark = executable('tmio_benchmark', ['tmio_benchmark.c', 'timer.c'], include_directories : tmio_inc, link_with : tmio_lib, dependencies : [ bufio_dep, m_dep] )
tmio_example_reader = executable('tmio_example_reader', ['tmio_example_reader.c', 'timer.c'], include_directories : tmio_inc, link_with : tmio_lib, dependencies : [ bufio_dep, m_dep] )
tmio_example_writer = executable('tmio_example_writer', ['tmio_example_writer.c', 'timer.c'], include_directories : tmio_inc, link_with : tmio_lib, dependencies : [ bufio_dep, m_dep] )
tmio_sink = executable('tmio_sink', ['tmio_sink.c', 'timer.c'], include_directories : tmio_inc, link_with : tmio_lib, dependencies : [ bufio_dep, m_dep] )