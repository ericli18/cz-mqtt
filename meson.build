project('tutorial', 'c')
executable('server', 'chatServer/pollserver.c')
executable('client', 'chatServer/pollclient.c')

msgpack_dep = dependency('msgpack-c')

e = executable('prog', 'tests/test_main.c')
test('test test', e, should_fail: true)

test_src = ['tests/msgpack.c']
test_exe = executable('test_msgpack', test_src,
                      dependencies : [msgpack_dep])

test('msgpack minunit test', test_exe)