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
const int debug = 0;
const int buffersize = 0;  // 0: default size, >0: kByte
const char* peer = "tmio_test_concatenated_files.dat";

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

void main_reader(const char* name, const char* peer, int count)
{
  unsigned long exp_read_bytes = 0;
  unsigned long exp_skipped_bytes = 0;
  int exp_read_tags = 0;
  int exp_read_data = 0;

  tmio_stream *stream = tmio_init(name, protocol_timeout, buffersize, verbosity);
  assert(tmio_open(stream, peer, connect_timeout) == TMIO_FILE);

  // 0. check init protocol size
  assert(stream->bytesread == (exp_read_bytes += frame_header_size + TMIO_PROTOCOL_SIZE));

  int iters = 0;
  while (iters < count) {
    // 1. check tag size
    assert(tmio_read_tag(stream) == TAG);
    assert(stream->bytesread == (exp_read_bytes += frame_header_size + (iters?1:0) * (frame_header_size + TMIO_PROTOCOL_SIZE)));
    // assert(stream->bytesskipped == (exp_skipped_bytes += (iters?1:0) * (frame_header_size + TMIO_PROTOCOL_SIZE)));
    assert(stream->tagreads == (exp_read_tags += 1));

    // 2. check data size
    assert(tmio_read_data(stream, buffer, LONG_MSG_SIZE) == LONG_MSG_SIZE);
    assert(stream->bytesread == (exp_read_bytes += frame_header_size + LONG_MSG_SIZE));
    assert(stream->datareads == (exp_read_data += 1));

    // 3. check datashort : reading less than requested
    assert(stream->datashorts == iters);
    assert(tmio_read_data(stream, buffer, LONG_MSG_SIZE) == SHORT_MSG_SIZE);
    assert(stream->bytesread == (exp_read_bytes += frame_header_size + SHORT_MSG_SIZE));
    assert(stream->datareads == (exp_read_data += 1));
    assert(stream->datashorts == iters+1);

    // 4. check datatrunc : reading more than expected, skipping non-requested
    assert(stream->datatruncs == iters);
    assert(tmio_read_data(stream, buffer, SHORT_MSG_SIZE) == LONG_MSG_SIZE);
    assert(stream->bytesread == (exp_read_bytes += frame_header_size + SHORT_MSG_SIZE));
    assert(stream->bytesskipped == (exp_skipped_bytes += SHORT_MSG_SIZE));
    assert(stream->datareads == (exp_read_data += 1));
    assert(stream->datatruncs == iters+1);

    // 5. check dataskip : looking for a tag, but there is still data left
    assert(stream->dataskipped == iters);
    assert(tmio_read_tag(stream) == TAG);
    assert(stream->bytesread == (exp_read_bytes += frame_header_size));
    assert(stream->bytesskipped == (exp_skipped_bytes += frame_header_size + SHORT_MSG_SIZE));
    assert(stream->tagreads == (exp_read_tags += 1));
    assert(stream->dataskipped == iters+1);

    // 6. check datamissing : reading non-existant data until following tag
    assert(stream->datamissing == iters);
    assert(tmio_read_data(stream, buffer, SHORT_MSG_SIZE) == -2);
    assert(stream->datamissing == iters+1);
    assert(tmio_read_tag(stream) == TAG);
    assert(stream->bytesread == (exp_read_bytes += frame_header_size));
    assert(stream->tagreads == (exp_read_tags += 1));

    iters++;
  }

  // -1. check on-disk size
  assert(stream->bytesread + stream->bytesskipped == filesize(peer));

  tmio_delete(stream);
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

  const char* tempfile = "tmio_test_file.dat";
  remove(argv[1]);
  remove(tempfile);
  main_writer(argv[0], tempfile);

  assert(append_file(argv[1], tempfile, 2) == 0);
  main_reader(argv[0], argv[1], 2);

  remove(argv[1]);
  remove(tempfile);

  return 0;
}
