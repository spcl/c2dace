#include "/workspaces/c2dace/.dacecache/_array_access/include/_array_access.h"

int main() {
    int M = 1, N = 20, K = 3;
    double *A = malloc(100 * N * sizeof(double));
    double *B = malloc(100 * M * sizeof(double));
    int i;

    // Initialize the SDFG (note that only the symbols are passed)
    my_sdfg_t handle = __dace_init_my_sdfg(K, M, N);

    // ...

    // Call the SDFG with arguments and symbols
    for (i = 0; i < 10; ++i)
        __program_my_sdfg(handle, A, B, K, M, N);

    // ...

    // Finalize the SDFG, freeing its resources
    __dace_exit_my_sdfg(handle);

    free(A);
    free(B);
    return 0;
}
