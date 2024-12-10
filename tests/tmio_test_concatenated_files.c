#include "tmio.h"

#include "test.h"

#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <errno.h>
#include <string.h>

const int protocol_timeout = 3000;  // ms
const int connect_timeout = -1;  // indefinite
const int wait_timeout = 0;  // immediate
const int verbosity = 3;  // 0...3 (silent...very verbose)
const int buffersize = 0;  // 0: default size, >0: kByte

#define TAG 1
#define LONG_MSG_SIZE (2)
#define SHORT_MSG_SIZE (LONG_MSG_SIZE/2)

const size_t frame_header_size = sizeof(int);
char buffer[LONG_MSG_SIZE] = {0};

unsigned long filesize(const char* filename) {
  struct stat st;
  stat(filename, &st);
  return st.st_size;
}

void main_writer(const char* name, const char* peer)
{
  unsigned long exp_written_bytes = 0;
  int exp_written_tags = 0;
  int exp_written_data = 0;

  tmio_stream *stream = tmio_init(name, protocol_timeout, buffersize, verbosity);
  assert(tmio_create(stream, peer, connect_timeout) == TMIO_FILE);

  // 0. check init protocol size
  assert(stream->byteswritten == (exp_written_bytes += frame_header_size + TMIO_PROTOCOL_SIZE));

  // 1. check tag size
  tmio_write_tag(stream, TAG);
  assert(stream->byteswritten == (exp_written_bytes += frame_header_size));
  assert(stream->tagwrites == (exp_written_tags += 1));

  // 3. check datashort : reading less than requested
  tmio_write_data(stream, buffer, LONG_MSG_SIZE);
  assert(stream->byteswritten == (exp_written_bytes += frame_header_size + LONG_MSG_SIZE));
  assert(stream->datawrites == (exp_written_data += 1));

  // 3. check reading beyond SHORT_MSG_SIZE
  tmio_write_data(stream, buffer, SHORT_MSG_SIZE);
  assert(stream->byteswritten == (exp_written_bytes += frame_header_size + SHORT_MSG_SIZE));
  assert(stream->datawrites == (exp_written_data += 1));

  // 4. check datatrunc : reading more than expected, skipping non-requested
  tmio_write_data(stream, buffer, LONG_MSG_SIZE);
  assert(stream->byteswritten == (exp_written_bytes += frame_header_size + LONG_MSG_SIZE));
  assert(stream->datawrites == (exp_written_data += 1));

  // 5. check dataskip : looking for a tag, but there is still data left
  tmio_write_data(stream, buffer, SHORT_MSG_SIZE);
  assert(stream->byteswritten == (exp_written_bytes += frame_header_size + SHORT_MSG_SIZE));
  assert(stream->datawrites == (exp_written_data += 1));

  tmio_write_tag(stream, TAG);
  assert(stream->byteswritten == (exp_written_bytes += frame_header_size));
  assert(stream->tagwrites == (exp_written_tags += 1));

  // 6. check datamissing : reading non-existant data until following tag
  tmio_write_tag(stream, TAG);
  assert(stream->byteswritten == (exp_written_bytes += frame_header_size));
  assert(stream->tagwrites == (exp_written_tags += 1));

  // -1. check on-disk size
  tmio_flush(stream);
  assert(stream->byteswritten == filesize(peer));
  assert(stream->flushes == 1);

  tmio_delete(stream);
}

tmio_stream *main_reader(const char* name, const char* peer, int n_concatenated_files)
{
  unsigned long exp_read_bytes = 0;
  unsigned long exp_skipped_bytes = 0;
  int exp_read_tags = 0;
  int exp_read_data = 0;

  tmio_stream *stream = tmio_init(name, protocol_timeout, buffersize, verbosity);
  assert(strcmp(tmio_protocol(stream), name) == 0);
  assert(strlen(tmio_stream_protocol(stream)) == 0);
  assert(tmio_open(stream, peer, connect_timeout) == TMIO_FILE);

  // 0. check init protocol match and size
  assert(strncmp(tmio_stream_protocol(stream), name, strlen(name)) == 0);
  assert(stream->bytesread == (exp_read_bytes += frame_header_size + TMIO_PROTOCOL_SIZE));

  int iterations = 0;
  while (iterations < n_concatenated_files) {
    // 1. check tag size
    assert(tmio_read_tag(stream) == TAG);
    assert(stream->bytesread == (exp_read_bytes += frame_header_size + (iterations?1:0) * (frame_header_size + TMIO_PROTOCOL_SIZE)));
    assert(stream->tagreads == (exp_read_tags += 1));

    // 2. check data size
    assert(tmio_read_data(stream, buffer, LONG_MSG_SIZE) == LONG_MSG_SIZE);
    assert(stream->bytesread == (exp_read_bytes += frame_header_size + LONG_MSG_SIZE));
    assert(stream->datareads == (exp_read_data += 1));

    // 3. check datashort : reading less than requested
    assert(stream->datashorts == iterations);
    assert(tmio_read_data(stream, buffer, LONG_MSG_SIZE) == SHORT_MSG_SIZE);
    assert(stream->bytesread == (exp_read_bytes += frame_header_size + SHORT_MSG_SIZE));
    assert(stream->datareads == (exp_read_data += 1));
    assert(stream->datashorts == iterations+1);

    // 4. check datatrunc : reading more than expected, skipping non-requested
    assert(stream->datatruncs == iterations);
    assert(tmio_read_data(stream, buffer, SHORT_MSG_SIZE) == LONG_MSG_SIZE);
    assert(stream->bytesread == (exp_read_bytes += frame_header_size + SHORT_MSG_SIZE));
    assert(stream->bytesskipped == (exp_skipped_bytes += SHORT_MSG_SIZE));
    assert(stream->datareads == (exp_read_data += 1));
    assert(stream->datatruncs == iterations+1);

    // 5. check dataskip : looking for a tag, but there is still data left
    assert(stream->dataskipped == iterations);
    assert(tmio_read_tag(stream) == TAG);
    assert(stream->bytesread == (exp_read_bytes += frame_header_size));
    assert(stream->bytesskipped == (exp_skipped_bytes += frame_header_size + SHORT_MSG_SIZE));
    assert(stream->tagreads == (exp_read_tags += 1));
    assert(stream->dataskipped == iterations+1);

    // 6. check datamissing : reading non-existant data until following tag
    assert(stream->datamissing == iterations);
    assert(tmio_read_data(stream, buffer, SHORT_MSG_SIZE) == -2);
    assert(stream->datamissing == iterations+1);
    assert(tmio_read_tag(stream) == TAG);
    assert(stream->bytesread == (exp_read_bytes += frame_header_size));
    assert(stream->tagreads == (exp_read_tags += 1));

    iterations++;
  }

  // -1. check on-disk size
  assert(stream->bytesread + stream->bytesskipped == filesize(peer));

  return stream;
}

int append_file(const char* destination_filename, const char* source_filename, int count) {
  FILE* destination = fopen(destination_filename, "a");
  if (!destination) {
    fprintf(stderr, "Couldn't open destination %s %d/%s\n", destination_filename, errno, strerror(errno));
    return -1;
  }

  while (count--) {

    FILE* source = fopen(source_filename, "r");
    if (!source) {
      fprintf(stderr, "Couldn't open source %s %d/%s\n", source_filename, errno, strerror(errno));
      return -1;
    }

    size_t fsize = filesize(source_filename);

    int rc;
    while ((rc = fgetc(source)) != EOF)
      fputc(rc, destination);
    if (rc < 0 && rc != EOF) {
      fprintf(stderr, "Not all data was sent %d/%zu: %d/%s\n", count, fsize, errno, strerror(errno));
      return -1;
    }
    fclose(source);
  }
  fclose(destination);


  return 0;
}


int main(int argc, const char* argv[])
{
  if (argc < 2)
    return 1;

  const char* tempfile0 = "tmio_test_file_0.dat";
  const char* tempfile1 = "tmio_test_file_1.dat";

  remove(argv[1]);
  remove(tempfile0);
  remove(tempfile1);

  main_writer("TMIOTestv1.0", tempfile0);
  main_writer("TMIOTestv1.1", tempfile1);

  assert(append_file(argv[1], tempfile0, 1) == 0);
  assert(append_file(argv[1], tempfile1, 1) == 0);
  tmio_stream *stream = main_reader("TMIOTestv1.", argv[1], 2);
  assert(strcmp(tmio_stream_protocol(stream), "TMIOTestv1.1") == 0);
  tmio_delete(stream);

  remove(argv[1]);
  remove(tempfile0);
  remove(tempfile1);

  return 0;
}
