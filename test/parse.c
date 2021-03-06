#include "test.h"
#include <bmff.h>
#include "mp4.h"
#include <memory.h>

char *test_data_pos = test_data_fmp4_mp4;

void test_parse(void);

int main(int argc, char** argv)
{
    test_parse();
    return 0;
}

size_t read_mp4(uint8_t *dest, int n, int s)
{
    char *end = &test_data_fmp4_mp4[sizeof(test_data_fmp4_mp4) - 1];
    int size = n * s;
    if(test_data_pos + size > end) {
        size = end - test_data_pos;
    }
    memcpy(dest, test_data_pos, size);
    test_data_pos += size;
    return size > 0 ? size : 0;
}

void callback_func(BMFFContext *ctx,
    BMFFEventId id,
    const uint8_t *four_cc,
    void *data,
    void *user_data)
{
    const char *breadcrumb = bmff_get_breadcrumb(ctx);

    if(id == BMFFEventParseStart)
    {
        if(strncmp(four_cc, "moov", 4) == 0) {
            test_assert_equal(strlen(breadcrumb), 0, "breadcrumb should not be set");
        }
        else if(strncmp(four_cc, "trak", 4) == 0) {
            test_assert_equal(strcmp("moov", breadcrumb), 0, "breadcrumb should be moov");
        }
        else if(strncmp(four_cc, "tkhd", 4) == 0) {
            test_assert_equal(strcmp("moov.trak", breadcrumb), 0, "breadcrumb should be moov.trak");
        }
        else if(strncmp(four_cc, "minf", 4) == 0) {
            test_assert_equal(strcmp("moov.trak.mdia", breadcrumb), 0, "breadcrumb should be moov.trak.mdia");
        }
    }
    else if(id == BMFFEventParseComplete)
    {
        if(strncmp(four_cc, "moov", 4) == 0) {
            test_assert_equal(strlen(breadcrumb), 0, "breadcrumb should not be set");
        }
        else if(strncmp(four_cc, "trak", 4) == 0) {
            test_assert_equal(strcmp("moov", breadcrumb), 0, "breadcrumb should be moov");
        }
        else if(strncmp(four_cc, "tkhd", 4) == 0) {
            test_assert_equal(strcmp("moov.trak", breadcrumb), 0, "breadcrumb should be moov.trak");
        }
        else if(strncmp(four_cc, "minf", 4) == 0) {
            test_assert_equal(strcmp("moov.trak.mdia", breadcrumb), 0, "breadcrumb should be moov.trak.mdia");
        }
    }
}

void test_parse(void)
{
    test_start("test_parse");

    // buffer for storing data into
    size_t buffer_size = 1024*1024;
    uint8_t *buffer = (uint8_t*)malloc(buffer_size);
    uint8_t *write_pos = buffer;
    uint8_t *read_pos = buffer;

    // create a context for parsing
    BMFFContext ctx;
    bmff_context_init(&ctx);

    // set the parsing callback
    bmff_set_event_callback(&ctx, callback_func, NULL);

    // read 1024 bytes from the file until we reach the end of the file
    int read_size = 1024;
    size_t read = read_mp4(write_pos, 1, read_size);
    while(read > 0)
    {
        // move the ptr forward
        write_pos += read;
        // parse the data
        BMFFCode res;
        size_t parsed = bmff_parse(&ctx, read_pos, write_pos-read_pos, &res);
        read_pos += parsed;

        // reset the buffer if everything is parsed
        if(read_pos == write_pos) {
            read_pos = write_pos = buffer;
        }

        // print an error if we don't have enough data in the buffer
        if(write_pos + read_size > buffer + buffer_size) {
            test_assert(0, "ran out of room in the buffer");
            break;
        }

        // read 1024 bytes from the file until we reach the end of the file
        read = read_mp4(write_pos, 1, read_size);
    }

    // end parsing
    bmff_parse_end(&ctx);
    // destory the context
    bmff_context_destroy(&ctx);
    // end the test
    test_end();
}
