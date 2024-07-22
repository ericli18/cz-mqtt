#include "minunit.h"
#include <msgpack.h>
#include <string.h>

static msgpack_sbuffer sbuf;
static msgpack_packer pk;
static msgpack_unpacked result;
static msgpack_unpack_return ret;

void test_setup(void) {
    msgpack_sbuffer_init(&sbuf);
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

    msgpack_pack_array(&pk, 3);
    msgpack_pack_int(&pk, 42);
    msgpack_pack_true(&pk);
    msgpack_pack_str(&pk, 5);
    msgpack_pack_str_body(&pk, "Hello", 5);

    msgpack_unpacked_init(&result);
    ret = msgpack_unpack_next(&result, sbuf.data, sbuf.size, NULL);
}

void test_teardown(void) {
    msgpack_sbuffer_destroy(&sbuf);
    msgpack_unpacked_destroy(&result);
}

MU_TEST(test_unpack_success) {
    mu_check(ret == MSGPACK_UNPACK_SUCCESS);
}

MU_TEST(test_array_type) {
    mu_check(result.data.type == MSGPACK_OBJECT_ARRAY);
}

MU_TEST(test_array_size) {
    mu_assert_int_eq(3, result.data.via.array.size);
}

MU_TEST(test_integer_value) {
    mu_check(result.data.via.array.ptr[0].type == MSGPACK_OBJECT_POSITIVE_INTEGER);
    mu_assert_int_eq(42, result.data.via.array.ptr[0].via.i64);
}

MU_TEST(test_boolean_value) {
    mu_check(result.data.via.array.ptr[1].type == MSGPACK_OBJECT_BOOLEAN);
    mu_check(result.data.via.array.ptr[1].via.boolean == true);
}

MU_TEST(test_string_value) {
    mu_check(result.data.via.array.ptr[2].type == MSGPACK_OBJECT_STR);
    mu_assert_int_eq(5, result.data.via.array.ptr[2].via.str.size);
    mu_assert_string_eq("Hello", result.data.via.array.ptr[2].via.str.ptr);
}

MU_TEST_SUITE(test_suite) {
    MU_SUITE_CONFIGURE(&test_setup, &test_teardown);

    MU_RUN_TEST(test_unpack_success);
    MU_RUN_TEST(test_array_type);
    MU_RUN_TEST(test_array_size);
    MU_RUN_TEST(test_integer_value);
    MU_RUN_TEST(test_boolean_value);
    MU_RUN_TEST(test_string_value);
}

int main(int argc, char *argv[]) {
    MU_RUN_SUITE(test_suite);
    MU_REPORT();
    return MU_EXIT_CODE;
}