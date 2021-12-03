#include <rena_cgi.h>
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

#if 0
LER DO TECLADO POST!
QUERY_STRING=a=2&b=2#112332
HTTP_USER_AGENT=Wget/1.20.3 (linux-gnu)
HTTP_ACCEPT=* div *
CONTENT_LENGTH=0
HTTP_HOST=asumi.devitest:8080
REQUEST_URI=/a/b
REQUEST_SCHEME=http
HTTP_ACCEPT_ENCODING=identity
REQUEST_METHOD=GET
#endif


int main(int argc, char **argv, char **envs)
{
    struct rena *all_modules = NULL;
    int ret_code = rena_cgi_setup(argc, argv, &all_modules);
    if (ret_code < 0)
    {
        return ret_code;
    }

    printf("Content-Type: text/plain\n\n");

    printf("<pre>");
    for (int e=0; envs[e] != NULL; e++)
    {
        printf("%s\n", envs[e]);
    }
    printf("--\n");

    const char *scheme = getenv("REQUEST_SCHEME");
    const char *host = getenv("HTTP_HOST");
    const char *path = getenv("REQUEST_URI");
    if (path==NULL) path="";

    printf("%s://%s%s\n\n\n", scheme, host, path);
    printf("</pre>");

    printf("<pre>");
  CURL *curl;
  CURLcode res;
 
  curl_global_init(CURL_GLOBAL_DEFAULT);
 
  curl = curl_easy_init();
  if(curl) {
      curl_easy_setopt(curl, CURLOPT_URL, "https://example.com/");
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

      res = curl_easy_perform(curl);
      /* Check for errors */
      if(res != CURLE_OK)
      {
          fprintf(stdout, "curl_easy_perform() failed: %s\n",
                  curl_easy_strerror(res));
      }

      /* always cleanup */
      curl_easy_cleanup(curl);
  }
    printf("</pre>");


    rena_cgi_close(&all_modules);
    return 0;
}

#if 0
REMOTE_ADDR=10.0.2.15
QUERY_STRING=
HTTP_USER_AGENT=Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:94.0) Gecko/20100101 Firefox/94.0
HTTP_ACCEPT=text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,* div *;q=0.8
CONTENT_LENGTH=0
HTTP_HOST=medicinanet.asumi.devitest:8080
REQUEST_URI=/xxx/3333/asd?x=a
REQUEST_SCHEME=http
HTTP_CONNECTION=keep-alive
HTTP_COOKIE=marma=maro
HTTP_ACCEPT_LANGUAGE=en-US,en;q=0.5
SERVER_PROTOCOL=HTTP/1.1
HTTP_ACCEPT_ENCODING=gzip, deflate
REQUEST_METHOD=GET
SERVER_NAME=medicinanet.asumi.devitest



curl_easy_getinfo
CURLINFO_CONTENT_TYPE char**, do NOT free it!
CURLINFO_COOKIELIST struct curl_slist **, free it with curl_slist_free_all
CURLINFO_REDIRECT_URL char **, do NOT free it!
CURLINFO_RESPONSE_CODE long *, do NOT free it!
CURLINFO_REFERER char **, do NOT free it!


    curl_easy_setopt(curl, CURLOPT_URL, "https://example.com");
    curl_easy_setopt(curl, CURLOPT_REFERER, "https://example.org/referrer");


















struct curl_slist *chunk = NULL;
/* Remove a header curl would otherwise add by itself */
chunk = curl_slist_append(chunk, "Accept:");
/* Add a custom header */
chunk = curl_slist_append(chunk, "Another: yes");
/* Modify a header curl otherwise adds differently */
chunk = curl_slist_append(chunk, "Host: example.com");
/* Add a header with "blank" contents to the right of the colon. Note that
   we are then using a semicolon in the string we pass to curl! */
chunk = curl_slist_append(chunk, "X-silly-header;");
/* set our custom set of headers */
curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
curl_easy_setopt(curl, CURLOPT_URL, "localhost");
curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
res = curl_easy_perform(curl);
/* Check for errors */
if(res != CURLE_OK)
  fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
/* always cleanup */
curl_easy_cleanup(curl);
/* free the custom headers */
curl_slist_free_all(chunk);
#endif
