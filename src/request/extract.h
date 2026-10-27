#ifndef EXTRACT_H
#define EXTRACT_H

char *extract_field(const char *json, const char *key);
int extract_manifest(const char *manifest_json, char **digest,
                     char **media_type);
int extract_config_digest(const char *manifest_json, char **config_digest);
int extract_layer_digest(const char *manifest_json, char ***layer_digest_list,
                         int *layer_count);

#endif // EXTRACT_H