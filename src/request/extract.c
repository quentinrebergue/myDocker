#define _POSIX_C_SOURCE 200809L

#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/utils.h"

/* extract_field:
 * Extrait la valeur d'un champ dans un JSON
 */
char *extract_field(const char *json, const char *key)
{
    char *p = strstr(json, key);
    if (!p)
    {
        return NULL;
    }
    p = strchr(p, ':');
    if (!p)
    {
        return NULL;
    }
    p++;
    while (*p == ' ' || *p == '"')
    {
        p++;
    }

    char *end = strchr(p, '"');

    if (!end)
    {
        return NULL;
    }
    return strndup(p, end - p);
}

int extract_manifest(const char *manifest_json, char **digest,
                     char **media_type)
{
    if (!manifest_json)
        return 1;

    const char *host_arch = get_host_arch();
    if (!host_arch)
        return 1;

    cJSON *root = cJSON_Parse(manifest_json);
    if (!root)
        return 1;

    // Read the list of manifests from the "manifests" field
    cJSON *manifests = cJSON_GetObjectItem(root, "manifests");
    if (!cJSON_IsArray(manifests))
    {
        cJSON_Delete(root);
        return 1;
    }

    cJSON *entry;
    cJSON_ArrayForEach(entry, manifests)
    {
        cJSON *platform = cJSON_GetObjectItem(entry, "platform");
        if (!cJSON_IsObject(platform))
            continue;

        cJSON *os = cJSON_GetObjectItem(platform, "os");
        cJSON *arch = cJSON_GetObjectItem(platform, "architecture");

        if (!cJSON_IsString(os) || !cJSON_IsString(arch))
            continue;

        if (strcmp(os->valuestring, "linux") != 0)
            continue;

        if (strcmp(arch->valuestring, host_arch) != 0)
            continue;

        cJSON *media_type_value = cJSON_GetObjectItem(entry, "mediaType");
        if (!cJSON_IsString(media_type_value))
            continue;

        cJSON *digest_value = cJSON_GetObjectItem(entry, "digest");
        if (!cJSON_IsString(digest_value))
            continue;

        *digest = strdup(digest_value->valuestring);
        *media_type = strdup(media_type_value->valuestring);
        cJSON_Delete(root);
        return 0;
    }

    cJSON_Delete(root);
    fprintf(stderr,
            "extract_manifest: no matching manifest found for arch %s\n",
            host_arch);
    return 1;
}

int extract_config_digest(const char *manifest_json, char **config_digest)
{
    cJSON *root = NULL;
    cJSON *config = NULL;
    cJSON *digest = NULL;

    root = cJSON_Parse(manifest_json);
    if (!root)
        return 1;

    config = cJSON_GetObjectItem(root, "config");
    if (!cJSON_IsObject(config))
    {
        cJSON_Delete(root);
        return 1;
    }

    digest = cJSON_GetObjectItem(config, "digest");
    if (!cJSON_IsString(digest) || !digest->valuestring)
    {
        cJSON_Delete(root);
        return 1;
    }

    *config_digest = strdup(digest->valuestring);
    cJSON_Delete(root);

    return 0;
}

int extract_layer_digest(const char *manifest_json, char ***layer_digest_list,
                         int *layer_count)
{
    cJSON *root = NULL;
    cJSON *layers = NULL;
    cJSON *layer = NULL;
    cJSON *digest = NULL;
    int count = 0;

    root = cJSON_Parse(manifest_json);
    if (!root)
    {
        fprintf(stderr,
                "extract_layer_digest: failed to parse manifest_json\n");
        return 1;
    }

    layers = cJSON_GetObjectItem(root, "layers");
    if (!cJSON_IsArray(layers))
    {
        fprintf(stderr, "extract_layer_digest: layers is not an array\n");
        cJSON_Delete(root);
        return 1;
    }

    count = cJSON_GetArraySize(layers);
    if (count <= 0)
    {
        fprintf(stderr, "extract_layer_digest: no layers found\n");
        cJSON_Delete(root);
        return 1;
    }

    *layer_digest_list = calloc(count, sizeof(char *));
    if (!*layer_digest_list)
    {
        fprintf(stderr, "extract_layer_digest: memory allocation failed\n");
        cJSON_Delete(root);
        return 1;
    }

    for (int i = 0; i < count; i++)
    {
        layer = cJSON_GetArrayItem(layers, i);
        digest = cJSON_GetObjectItem(layer, "digest");
        if (!cJSON_IsString(digest) || !digest->valuestring)
        {
            fprintf(stderr,
                    "extract_layer_digest: invalid digest in layer %d\n", i);
            cJSON_Delete(root);
            free(layer_digest_list);
            return 1;
        }

        (*layer_digest_list)[i] = strdup(digest->valuestring);
    }

    *layer_count = count;
    cJSON_Delete(root);
    return 0;
}
