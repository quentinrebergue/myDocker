#define _POSIX_C_SOURCE 200809L

#include "request/request.h"

#include <cjson/cJSON.h>
#include <curl/curl.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "request/extract.h"
#include "utils/utils.h"

#define DOCKER_AUTH_REG_URL                                                    \
    "https://auth.docker.io/"                                                  \
    "token?service=registry.docker.io&scope=repository:"
#define DOCKER_MANIFEST_URL "https://registry-1.docker.io/v2/"

/* write_func:
 * Fonction de callback pour ecrire les donnees recues par curl
 */
static size_t write_func(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t n = size * nmemb;
    struct buff *b = userdata;
    char *tmp = realloc(b->data, b->size + n + 1);
    if (!tmp)
        return 0;
    b->data = tmp;
    memcpy(b->data + b->size, ptr, n);
    b->size += n;
    b->data[b->size] = '\0';
    return n;
}

static size_t write_blob(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t n = size * nmemb;
    struct buff *b = userdata;
    unsigned char *tmp = realloc(b->data, b->size + n);
    if (!tmp)
        return 0;
    b->data = (char *)tmp;
    memcpy(b->data + b->size, ptr, n);
    b->size += n;
    return n;
}

/* auth_request:
 * curl -s "https://auth.docker.io/token?service=registry.docker.io&\
 * scope=repository:${REPO}:pull"
 */
int auth_request(char *repo, char **token)
{
    // printf("auth_request: Repo: %s\n", repo);
    if (!repo)
    {
        return 1;
    }
    // Create url for auth request
    size_t url_size =
        strlen(DOCKER_AUTH_REG_URL) + strlen(repo) + strlen(":pull") + 5;
    char *url = malloc(url_size);
    if (!url)
    {
        return 1;
    }
    snprintf(url, url_size, "%s%s:pull", DOCKER_AUTH_REG_URL, repo);

    // Perform http request
    CURL *curl = curl_easy_init();
    struct buff buf = { NULL, 0 };
    if (!curl)
    {
        free(url);
        fprintf(stderr, "http_request: curl_easy_init failed\n");
        return 1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_func);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);

    if (curl_easy_perform(curl) != CURLE_OK)
    {
        fprintf(stderr, "http_request: curl_easy_perform failed\n");
        curl_easy_cleanup(curl);
        free(buf.data);
        free(url);
        return -1;
    }
    free(url);
    curl_easy_cleanup(curl);

    *token = extract_field(buf.data, "token");
    if (!*token)
    {
        free(buf.data);
        return 1;
    }
    free(buf.data);
    return 0;
}

/* list_manifest_request:
 * curl -s \
 * -H "Authorization: Bearer ${TOKEN}" \
 * -H "Accept: application/vnd.docker.distribution.manifest.list.v2+json" \
 * "https://registry-1.docker.io/v2/library/${REPO}/manifests/${TAG}"
 */
int list_manifest_request(char *repo, char *tag, const char *token,
                          char **manifest_json)
{
    // printf("get_list_manifests: Repo: %s, Tag: %s\n", repo, tag);
    if (!token || !repo || !tag)
        return 1;

    *manifest_json = NULL;

    size_t url_size = strlen(DOCKER_MANIFEST_URL) + strlen(repo)
        + strlen("/manifests/") + strlen(tag) + 5;
    char *url = malloc(url_size);
    if (!url)
    {
        perror("malloc");
        return 1;
    }
    snprintf(url, url_size, "%s%s/manifests/%s", DOCKER_MANIFEST_URL, repo,
             tag);

    CURL *curl = curl_easy_init();
    struct buff buf = { NULL, 0 };
    struct curl_slist *headers = NULL;

    if (!curl)
    {
        perror("curl_easy_init");
        free(url);
        return 1;
    }

    // Authorization header
    size_t auth_size = strlen("Authorization: Bearer ") + strlen(token) + 1;
    char *auth_header = malloc(auth_size);
    if (!auth_header)
    {
        perror("malloc");
        free(url);
        curl_easy_cleanup(curl);
        return 1;
    }
    snprintf(auth_header, auth_size, "Authorization: Bearer %s", token);

    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(
        headers,
        "Accept: application/vnd.docker.distribution.manifest.list.v2+json");

    free(auth_header);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_func);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);

    if (curl_easy_perform(curl) != CURLE_OK)
    {
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        free(buf.data);
        free(url);
        return 1;
    }

    *manifest_json = buf.data;

    free(url);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return 0;
}

/* manifest_request:
 * curl -s \
 *   -H "Authorization: Bearer ${TOKEN}" \
 *   -H "Accept: ${MEDIATYPE}" \
 * "https://registry-1.docker.io/v2/library/{REPO}/manifests/${MANIFEST_DIGEST}"
 */
char *manifest_request(const char *repo, const char *token,
                       const char *mediatype, const char *manifest_digest)
{
    // printf("get_manifest: Repo: %s, Manifest Digest: %s\n", repo,
    //        manifest_digest);
    if (!repo || !token || !mediatype || !manifest_digest)
        return NULL;

    char *manifest_json = NULL;

    // Build URL
    size_t url_size = strlen(DOCKER_MANIFEST_URL) + strlen(repo)
        + strlen("/manifests/") + strlen(manifest_digest) + 5;

    char *url = malloc(url_size);
    if (!url)
        return NULL;

    snprintf(url, url_size, "%s%s/manifests/%s", DOCKER_MANIFEST_URL, repo,
             manifest_digest);

    CURL *curl = curl_easy_init();
    struct buff buf = { NULL, 0 };
    struct curl_slist *headers = NULL;

    if (!curl)
    {
        free(url);
        return NULL;
    }

    size_t auth_size = strlen("Authorization: Bearer ") + strlen(token) + 1;

    char *auth_header = malloc(auth_size);
    if (!auth_header)
    {
        free(url);
        curl_easy_cleanup(curl);
        return NULL;
    }

    snprintf(auth_header, auth_size, "Authorization: Bearer %s", token);
    headers = curl_slist_append(headers, auth_header);
    free(auth_header);

    // Header Accept
    size_t accept_size = strlen("Accept: ") + strlen(mediatype) + 1;
    char *accept_header = malloc(accept_size);
    if (!accept_header)
    {
        curl_slist_free_all(headers);
        free(url);
        curl_easy_cleanup(curl);
        return NULL;
    }

    snprintf(accept_header, accept_size, "Accept: %s", mediatype);
    headers = curl_slist_append(headers, accept_header);
    free(accept_header);

    // curl options
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_func);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);

    if (curl_easy_perform(curl) != CURLE_OK)
    {
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        free(buf.data);
        free(url);
        return NULL;
    }

    manifest_json = buf.data;

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    free(url);
    return manifest_json;
}

/* config_image_request:
 * curl -s --location \
 *   -H "Authorization: Bearer ${TOKEN}" \
 * "https://registry-1.docker.io/v2/library/{REPO}/blobs/${CNF_DIGEST}"
 */
int config_image_request(const char *repo, const char *token,
                         const char *config_digest, char **config_image_json)
{
    // printf("get_config_image: Repo: %s, Config Digest: %s\n", repo,
    //        config_digest);
    if (!repo || !token || !config_digest)
        return 1;

    // Build URL
    size_t url_size = strlen(DOCKER_MANIFEST_URL) + strlen(repo)
        + strlen("/blobs/") + strlen(config_digest) + 5;

    char *url = malloc(url_size);
    if (!url)
        return 1;

    snprintf(url, url_size, "%s%s/blobs/%s", DOCKER_MANIFEST_URL, repo,
             config_digest);

    CURL *curl = curl_easy_init();
    struct buff buf = { NULL, 0 };
    struct curl_slist *headers = NULL;

    if (!curl)
    {
        free(url);
        return 1;
    }

    size_t auth_size = strlen("Authorization: Bearer ") + strlen(token) + 1;

    char *auth_header = malloc(auth_size);
    if (!auth_header)
    {
        free(url);
        curl_easy_cleanup(curl);
        return 1;
    }

    snprintf(auth_header, auth_size, "Authorization: Bearer %s", token);
    headers = curl_slist_append(headers, auth_header);
    free(auth_header);

    // Set the URL to request
    curl_easy_setopt(curl, CURLOPT_URL, url);
    // Set HTTP headers (Authorization)
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    // Function to write received data
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_blob);
    // Buffer to store the response
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
    // Follow HTTP redirects (3xx)
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    if (curl_easy_perform(curl) != CURLE_OK)
    {
        fprintf(stderr, "config_image_request: curl_easy_perform failed\n");
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        free(buf.data);
        free(url);
        return 1;
    }

    *config_image_json = buf.data;

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    free(url);
    return 0;
}

/* fetch_layer:
 * wget --header "Authorization: Bearer ${TOKEN}" \
 * "https://registry-1.docker.io/v2/library/hello-world/blobs/${LAYER_DGST}"
 */
int layer_wget(const char *repo, const char *layer_digest, const char *token)
{
    // printf("fetch_layer: Repo: %s, Layer Digest: %s\n", repo, layer_digest);
    int fd;

    // Build URL
    size_t url_size = strlen(DOCKER_MANIFEST_URL) + strlen(repo)
        + strlen("/blobs/") + strlen(layer_digest) + 5;

    char *url = malloc(url_size);
    if (!url)
    {
        perror("malloc");
        return 1;
    }

    snprintf(url, url_size, "%s%s/blobs/%s", DOCKER_MANIFEST_URL, repo,
             layer_digest);

    CURL *curl = curl_easy_init();
    struct buff layer = { NULL, 0 };
    struct curl_slist *headers = NULL;

    if (!curl)
    {
        fprintf(stderr, "fetch_layer: curl_easy_init failed\n");
        free(url);
        return 1;
    }

    size_t auth_size = strlen("Authorization: Bearer ") + strlen(token) + 1;

    char *auth_header = malloc(auth_size);
    if (!auth_header)
    {
        perror("malloc");
        free(url);
        curl_easy_cleanup(curl);
        return 1;
    }

    snprintf(auth_header, auth_size, "Authorization: Bearer %s", token);
    headers = curl_slist_append(headers, auth_header);
    free(auth_header);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_blob);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &layer);

    if (curl_easy_perform(curl) != CURLE_OK)
    {
        free(url);
        fprintf(stderr, "fetch_layer: curl_easy_perform failed\n");
        return 1;
    }

    fd = open("layer.tar.gz", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0)
    {
        perror("open");
        free(url);
        return 1;
    }

    if (write(fd, layer.data, layer.size) < 0)
    {
        perror("write");
        free(url);
        close(fd);
        return 1;
    }

    close(fd);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    free(layer.data);
    free(url);

    return 0;
}
