#define _POSIX_SOURCE
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "request/extract.h"
#include "request/request.h"

int fetch_and_extract_layers(const char *repo, char **layer_digest_list,
                             int layer_count, const char *token,
                             const char *rootfs)
{
    for (int i = 0; i < layer_count; i++)
    {
        if (layer_wget((char *)repo, layer_digest_list[i], (char *)token) < 0)
        {
            fprintf(stderr, "layer_wget failed for layer %s\n",
                    layer_digest_list[i]);
            return -1;
        }

        pid_t pid = fork();
        if (pid < 0)
        {
            perror("fork");
            return -1;
        }

        if (pid == 0)
        {
            execlp("tar", "tar", "-xzf", "layer.tar.gz", "-C", rootfs, NULL);
            perror("execlp");
            _exit(1);
        }

        int status;
        if (waitpid(pid, &status, 0) < 0)
        {
            perror("waitpid");
            return -1;
        }
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
        {
            fprintf(stderr, "tar failed for layer %s\n", layer_digest_list[i]);
            return -1;
        }
    }
    return 0;
}

int build_env(char *repo, char *tag, char **rootfs)
{
    if (!repo || !tag)
    {
        return -1;
    }
    // Auth token
    char *token = NULL;
    // List of all manifest for the image
    char *manifest_json = NULL;
    // Digest of the manifest corresponding to the host architecture
    char *digest = NULL;
    // Media type of the manifest
    char *media_type = NULL;
    // Manifest of the image
    char *manifest = NULL;
    // Digest of the config image
    char *config_digest = NULL;
    // Config image JSON
    char *config_image_json = NULL;
    // List of layer digests
    char **layer_digest_list = NULL;
    // Number of layers
    int layer_count = 0;

    int return_value = 0;

    *rootfs = calloc(50, sizeof(char));

    // Create a temp dir
    if (!*rootfs)
    {
        return_value = -1;
        perror("calloc");
        goto cleanup;
    }
    if (!strncpy(*rootfs, "/tmp/mymoulette-XXXXXX", 23))
    {
        return_value = -1;
        perror("strncpy");
        goto cleanup;
    }
    if (!mkdtemp(*rootfs))
    {
        return_value = -1;
        perror("mkdtemp");
        goto cleanup;
    }

    // Auth request to retrieve token
    if (auth_request(repo, &token))
    {
        return_value = -1;
        fprintf(stderr, "auth_request failed\n");
        goto cleanup;
    }

    // Get list of all manifests for the image
    if (list_manifest_request(repo, tag, token, &manifest_json))
    {
        return_value = -1;
        fprintf(stderr, "list_manifest_request failed\n");
        goto cleanup;
    }

    // Get the manifest digest (hash) and media type from the list corresponding
    // to the host architecture
    if (extract_manifest(manifest_json, &digest, &media_type))
    {
        return_value = -1;
        fprintf(stderr, "extract_manifest failed\n");
        goto cleanup;
    }

    // From the digest, and the media type, get the manifest of the image
    if (!(manifest = manifest_request(repo, token, media_type, digest)))
    {
        return_value = -1;
        fprintf(stderr, "fetch_image_manifest failed\n");
        goto cleanup;
    }

    // From the manifest, get the config image digest (hash)
    if (extract_config_digest(manifest, &config_digest))
    {
        return_value = -1;
        fprintf(stderr, "extract_config_digest failed\n");
        goto cleanup;
    }

    // From the config image digest, get the config image
    if (config_image_request(repo, token, config_digest, &config_image_json))
    {
        return_value = -1;
        fprintf(stderr, "extract_config_digest failed\n");
        goto cleanup;
    }

    // From the config image, get the list of layer digests (hash) and the
    // number of layers
    if (extract_layer_digest(manifest, &layer_digest_list, &layer_count))
    {
        return_value = -1;
        fprintf(stderr, "extract_layer_digest failed\n");
        goto cleanup;
    }

    // For each layer digest, fetch and extract the layer into the rootfs
    if (fetch_and_extract_layers(repo, layer_digest_list, layer_count, token,
                                 *rootfs))
    {
        return_value = -1;
        goto cleanup;
    }

cleanup:
    for (int j = 0; j < layer_count; j++)
    {
        free(layer_digest_list[j]);
    }
    free(layer_digest_list);
    free(config_digest);
    free(media_type);
    free(digest);
    free(manifest_json);
    free(token);
    free(manifest);
    free(config_image_json);
    return return_value;
}