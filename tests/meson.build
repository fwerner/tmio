tmio_test_statistics = executable('tmio_test_statistics', 'tmio_test_statistics.c', dependencies : [tmio_dep])
tmio_test_concatenated_files = executable('tmio_test_concatenated_files', 'tmio_test_concatenated_files.c', dependencies : [tmio_dep])

test('tmio_test_statistics', tmio_test_statistics, is_parallel : true, args : ['tmio_test_statistics.dat'], suite: ['all', 'default'])
test('tmio_test_concatenated_files', tmio_test_concatenated_files, is_parallel : true, args : ['tmio_test_concatenated_files.dat'], suite: ['all', 'default'])

