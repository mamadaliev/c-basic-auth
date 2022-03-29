#ifndef BASE46_H
#define BASE46_H

#include <stdlib.h>
#include <memory.h>

char *base64_encode(char *plain_text);

char *base64_decode(char *encoded_text);

#endif
