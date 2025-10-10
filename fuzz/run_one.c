/* Simple runner to invoke LLVMFuzzerTestOneInput on a single file.
 * This lets us run the harness under Valgrind (no libFuzzer main required).
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* Declare the fuzzer entry implemented in fuzz_yaep.c */
extern int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s <input-file>\n", argv[0]);
        return 2;
    }

    const char *path = argv[1];
    FILE *f = fopen(path, "rb");
    if (!f) {
        perror("fopen");
        return 2;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        perror("fseek");
        fclose(f);
        return 2;
    }
    long len = ftell(f);
    if (len < 0) {
        perror("ftell");
        fclose(f);
        return 2;
    }
    rewind(f);

    uint8_t *buf = malloc(len > 0 ? (size_t)len : 1);
    if (!buf) {
        perror("malloc");
        fclose(f);
        return 2;
    }

    size_t readn = fread(buf, 1, (size_t)len, f);
    if (readn != (size_t)len) {
        fprintf(stderr, "short read: %zu != %ld\n", readn, len);
        /* continue anyway */
    }
    fclose(f);

    /* Call the fuzzer entry */
    LLVMFuzzerTestOneInput(buf, readn);

    free(buf);
    return 0;
}
