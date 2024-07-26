#include "minunit.h"
#include "../src/mqtt_packet_utils.h"
#include <string.h>
#include <stdlib.h>

static uint8_t test_buffer[256];
static uint8_t *buffer_ptr;

void test_setup(void) {
    memset(test_buffer, 0, sizeof(test_buffer));
    buffer_ptr = test_buffer;
}

void test_teardown(void) {
    // Nothing to tear down
}

MU_TEST(test_pack_unpack_u8) {
    uint8_t original = 0xA5;
    mqtt_pack_u8(&buffer_ptr, original);
    buffer_ptr = test_buffer;
    uint8_t unpacked = mqtt_unpack_u8(&buffer_ptr);
    mu_assert_int_eq(original, unpacked);
}

MU_TEST(test_pack_unpack_u16) {
    uint16_t original = 0xA5B6;
    mqtt_pack_u16(&buffer_ptr, original);
    buffer_ptr = test_buffer;
    uint16_t unpacked = mqtt_unpack_u16(&buffer_ptr);
    mu_assert_int_eq(original, unpacked);
}

MU_TEST(test_pack_unpack_u32) {
    uint32_t original = 0xA5B6C7D8;
    mqtt_pack_u32(&buffer_ptr, original);
    buffer_ptr = test_buffer;
    uint32_t unpacked = mqtt_unpack_u32(&buffer_ptr);
    mu_assert_int_eq(original, unpacked);
}

MU_TEST(test_pack_unpack_variable_int) {
    uint32_t original = 128;
    mqtt_pack_variable_int(&buffer_ptr, original);
    buffer_ptr = test_buffer;
    int bytes_read;
    uint32_t unpacked = mqtt_unpack_variable_int(&buffer_ptr, &bytes_read);
    mu_assert_int_eq(original, unpacked);
    mu_assert_int_eq(2, bytes_read);
}

MU_TEST(test_pack_unpack_string) {
    const char* original = "MQTT";
    uint16_t length = strlen(original);
    mqtt_pack_string(&buffer_ptr, original, length);
    buffer_ptr = test_buffer;
    uint16_t unpacked_length;
    char* unpacked = mqtt_unpack_string(&buffer_ptr, &unpacked_length);
    mu_assert_string_eq(original, unpacked);
    mu_assert_int_eq(length, unpacked_length);
    free(unpacked);
}

MU_TEST(test_validate_utf8_valid) {
    const char* valid_utf8 = "Hello, 世界!";
    mu_check(mqtt_validate_utf8(valid_utf8, strlen(valid_utf8)));
}

MU_TEST(test_validate_utf8_invalid) {
    const char invalid_utf8[] = {0xFF, 0xFE, 0xFD};
    mu_check(!mqtt_validate_utf8(invalid_utf8, sizeof(invalid_utf8)));
}

MU_TEST_SUITE(test_suite) {
    MU_SUITE_CONFIGURE(&test_setup, &test_teardown);

    MU_RUN_TEST(test_pack_unpack_u8);
    MU_RUN_TEST(test_pack_unpack_u16);
    MU_RUN_TEST(test_pack_unpack_u32);
    MU_RUN_TEST(test_pack_unpack_variable_int);
    MU_RUN_TEST(test_pack_unpack_string);
    MU_RUN_TEST(test_validate_utf8_valid);
    MU_RUN_TEST(test_validate_utf8_invalid);
}

int main(int argc, char *argv[]) {
    MU_RUN_SUITE(test_suite);
    MU_REPORT();
    return MU_EXIT_CODE;
}
