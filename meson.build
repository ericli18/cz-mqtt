project('mqtt_project', 'c')

# Create a library from the MQTT utility functions
mqtt_lib = static_library('mqtt_utils', 
                          sources: ['src/mqtt_packet_utils.c'],
                          include_directories: include_directories('src'))

# Build the chat server and client
executable('server', 'chatServer/pollserver.c')
executable('client', 'chatServer/pollclient.c')

# Build and run the MQTT tests
mqtt_test = executable('mqtt_test', 
                       'tests/mqtt.c',
                       link_with: mqtt_lib,
                       include_directories: include_directories('src'))
test('mqtt_Test', mqtt_test)

mqtt_utils_test = executable('mqtt_utils_test', 
                             'tests/packet_utils.c',
                             link_with: mqtt_lib,
                             include_directories: include_directories('src'))
test('packet_utils', mqtt_utils_test)

# msgpack_dep = dependency('msgpack-c')
# executable('mytest', 'src/main.c', dependencies : [msgpack_dep])