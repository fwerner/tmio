tmio_test_statistics = executable('tmio_test_statistics', 'tmio_test_statistics.c', dependencies : [tmio_dep])

test('tmio_test_statistics', tmio_test_statistics, is_parallel : true, args : ['tmio_test_statistics.dat'], suite: ['all', 'default'])
