#ifndef REQUEST_H
#define REQUEST_H

#include <stdlib.h>

struct buff
{
    char *data;
    size_t size;
};

int list_manifest_request(char *repo, char *tag, const char *token,
                          char **manifest_json);
int auth_request(char *repo, char **token);
char *manifest_request(const char *repo, const char *token,
                       const char *mediatype, const char *manifest_digest);
int config_image_request(const char *repo, const char *token,
                         const char *config_digest, char **config_image_json);
int layer_wget(const char *repo, const char *layer_digest, const char *token);

#endif // REQUEST_H