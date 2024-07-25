project('tutorial', 'c')
executable('server', 'chatServer/pollserver.c')
executable('client', 'chatServer/pollclient.c')

# msgpack_dep = dependency('msgpack-c')
# executable('mytest', 'src/main.c', dependencies : [msgpack_dep])

mqtt_test = executable('mqtt_test', 'tests/mqtt.c')
test('mqtt_Test', mqtt_test)
