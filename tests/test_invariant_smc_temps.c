#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

// Test the actual production function by executing it in a separate process
static void run_smc_temps_with_input(const char *input) {
    int pipefd[2];
    pid_t pid;
    
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    
    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    
    if (pid == 0) { // Child process
        close(pipefd[0]); // Close read end
        
        // Write input to pipe (simulating function input)
        write(pipefd[1], input, strlen(input));
        close(pipefd[1]);
        
        // Execute the actual production code
        execl("./test_smc_temps_wrapper", "test_smc_temps_wrapper", (char *)NULL);
        perror("execl");
        exit(EXIT_FAILURE);
    } else { // Parent process
        close(pipefd[1]); // Close write end
        int status;
        waitpid(pid, &status, 0);
        
        // Check if child crashed (buffer overflow would cause crash)
        ck_assert_msg(WIFEXITED(status) && WEXITSTATUS(status) == 0,
                     "Buffer overflow detected with input: %s", input);
        close(pipefd[0]);
    }
}

START_TEST(test_buffer_reads_never_exceed_declared_length)
{
    // Invariant: Buffer reads never exceed the declared length
    const char *payloads[] = {
        "valid_temp",                    // Valid input
        "A",                             // Boundary: single char
        "very_long_temperature_name_that_exceeds_typical_buffer_size_by_more_than_double_what_is_expected_in_normal_operation_conditions", // 2x+ overflow
        "X" "very_long_string_" "X" "very_long_string_" "X" "very_long_string_", // 10x+ overflow
        NULL                             // NULL pointer case
    };
    
    for (int i = 0; i < 4; i++) { // Test first 4 payloads
        run_smc_temps_with_input(payloads[i]);
    }
    
    // Test NULL case separately
    pid_t pid = fork();
    if (pid == 0) {
        execl("./test_smc_temps_wrapper", "test_smc_temps_wrapper", (char *)NULL);
        exit(EXIT_FAILURE);
    } else {
        int status;
        waitpid(pid, &status, 0);
        ck_assert_msg(WIFEXITED(status) && WEXITSTATUS(status) == 0,
                     "NULL input caused crash");
    }
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_buffer_reads_never_exceed_declared_length);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}