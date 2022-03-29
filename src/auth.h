#ifndef PAM_H
#define PAM_H

char **get_credentials_by_decoded_basic_auth(char decoded_credentials[]);

int is_authorized(char* encoded_credentials);

#endif
