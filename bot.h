#include <stdint.h>

struct bot{
    int socketfd;
};

extern struct bot context;

int join_server(struct bot *, char *, char *, char *);
int send_str(struct bot *, char *);
int send_raw(struct bot *, void *, int);
int varint64(char *, int64_t *);
int uvarint64(char *, uint64_t *);