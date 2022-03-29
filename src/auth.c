#include <stdio.h>
#include <security/pam_appl.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "auth.h"
#include "base64.h"

char **get_credentials_by_decoded_basic_auth(char decoded_credentials[]) {
    char **credentials = malloc(2 * sizeof(char));
    int i = 0;
    for (char *t = strtok(decoded_credentials, ":"); t != NULL; t = strtok(NULL, ":")) {
        credentials[i] = t;
        i++;
    }
    return credentials;
}

static int pam_conversation(int num_msg, const struct pam_message **msg,
                            struct pam_response **resp, void *appdata_ptr) {
    char *pass = malloc(strlen(appdata_ptr) + 1);
    strcpy(pass, appdata_ptr);

    int i;

    *resp = calloc(num_msg, sizeof(struct pam_response));

    for (i = 0; i < num_msg; ++i) {
        /* Ignore all PAM messages except prompting for hidden input */
        if (msg[i]->msg_style != PAM_PROMPT_ECHO_OFF)
            continue;

        /* Assume PAM is only prompting for the password as hidden input */
        resp[i]->resp = pass;
    }

    return PAM_SUCCESS;
}

bool is_access_allowed_by_username_and_password(const char *username, const char *password) {
    /* use own PAM conversation function just responding with the
       password passed here */
    struct pam_conv conv = {&pam_conversation, (void *) password};

    pam_handle_t *handle;
    int authResult;

    pam_start("shutterd", username, &conv, &handle);
    authResult = pam_authenticate(handle,
                                  PAM_SILENT | PAM_DISALLOW_NULL_AUTHTOK);
    pam_end(handle, authResult);

    return (authResult == PAM_SUCCESS);
}

int is_authorized(char *encoded_credentials) {
    printf("EC: %s\n", encoded_credentials);
    char *decoded_credentials = base64_decode(encoded_credentials);
    printf("DC: %s\n", decoded_credentials);
    char **data = get_credentials_by_decoded_basic_auth(decoded_credentials);
    return is_access_allowed_by_username_and_password(data[0], data[1]);
}
