#include "../src/mqtt.c"
#include "minunit.h"
#include "src/mqtt.h"
#include <stdio.h>

size_t len = 18;

void test_setup(void) { /* Nothing */ }

void test_teardown(void) { /* Nothing */ }


MU_TEST(test_check) {
	mu_check(len == 18);
}

MU_TEST(test_same) {
    unsigned char buf[4];
    mqtt_encode_length(buf, len);
    size_t ans = 0;
    const unsigned char *ptr = buf;
    mqtt_decode_length(&ptr, &ans);
    mu_check(ans == len);
}



MU_TEST_SUITE(test_suite) {
	MU_SUITE_CONFIGURE(&test_setup, &test_teardown);

	MU_RUN_TEST(test_check);
	MU_RUN_TEST(test_same);

}

int main(int argc, char *argv[]) {
	MU_RUN_SUITE(test_suite);
	MU_REPORT();
	return MU_EXIT_CODE;
}

