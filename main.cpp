/*
 * C++ example of using pinned SSL/TLS certificates with libtls
 * without using libcurl.
 *
 * Based initially off of
 * https://gist.github.com/kinichiro/9ac1f6768d490bb3d9828e9ffac7d098
 */

#include <err.h>
#include <stdio.h>
#include <string.h>

#include <tls.h>

#include "http.h"

const char *USER_AGENT = "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_6) "
                         "AppleWebKit/537.36 (KHTML, like Gecko) "
                         "Chrome/65.0.3325.181 Safari/537.36";
bool flag_printResponseHeaders = false;
bool flag_printRequest = false;

#ifndef NDEBUG
#define PERR(A) warn A;
#else
#define PERR(A)
#endif

const char pinnedCertsPEM[] =
    // DigiCert Global Root CA (badssl.com)
    "-----BEGIN CERTIFICATE-----\n\
MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n\
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n\
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n\
QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n\
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n\
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n\
9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n\
CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n\
nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n\
43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n\
T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n\
gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n\
BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n\
TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n\
DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n\
hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n\
06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n\
PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n\
YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n\
CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n\
-----END CERTIFICATE-----\n"
    // GeoTrust Global CA (google.com)
    "-----BEGIN CERTIFICATE-----\n\
MIIDVDCCAjygAwIBAgIDAjRWMA0GCSqGSIb3DQEBBQUAMEIxCzAJBgNVBAYTAlVT\n\
MRYwFAYDVQQKEw1HZW9UcnVzdCBJbmMuMRswGQYDVQQDExJHZW9UcnVzdCBHbG9i\n\
YWwgQ0EwHhcNMDIwNTIxMDQwMDAwWhcNMjIwNTIxMDQwMDAwWjBCMQswCQYDVQQG\n\
EwJVUzEWMBQGA1UEChMNR2VvVHJ1c3QgSW5jLjEbMBkGA1UEAxMSR2VvVHJ1c3Qg\n\
R2xvYmFsIENBMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA2swYYzD9\n\
9BcjGlZ+W988bDjkcbd4kdS8odhM+KhDtgPpTSEHCIjaWC9mOSm9BXiLnTjoBbdq\n\
fnGk5sRgprDvgOSJKA+eJdbtg/OtppHHmMlCGDUUna2YRpIuT8rxh0PBFpVXLVDv\n\
iS2Aelet8u5fa9IAjbkU+BQVNdnARqN7csiRv8lVK83Qlz6cJmTM386DGXHKTubU\n\
1XupGc1V3sjs0l44U+VcT4wt/lAjNvxm5suOpDkZALeVAjmRCw7+OC7RHQWa9k0+\n\
bw8HHa8sHo9gOeL6NlMTOdReJivbPagUvTLrGAMoUgRx5aszPeE4uwc2hGKceeoW\n\
MPRfwCvocWvk+QIDAQABo1MwUTAPBgNVHRMBAf8EBTADAQH/MB0GA1UdDgQWBBTA\n\
ephojYn7qwVkDBF9qn1luMrMTjAfBgNVHSMEGDAWgBTAephojYn7qwVkDBF9qn1l\n\
uMrMTjANBgkqhkiG9w0BAQUFAAOCAQEANeMpauUvXVSOKVCUn5kaFOSPeCpilKIn\n\
Z57QzxpeR+nBsqTP3UEaBU6bS+5Kb1VSsyShNwrrZHYqLizz/Tt1kL/6cdjHPTfS\n\
tQWVYrmm3ok9Nns4d0iXrKYgjy6myQzCsplFAMfOEVEiIuCl6rYVSAlk6l5PdPcF\n\
PseKUgzbFbS9bZvlxrFUaKnjaZC2mqUPuLk/IH2uSrW4nOQdtqvmlKXBx4Ot2/Un\n\
hw4EbNX/3aBd7YdStysVAq45pmp06drE57xNNB6pXE0zX5IJL4hmXXeXxx12E6nV\n\
5fEWCRE11azbJHFwLJhWC9kXtNHjUStedejV0NxPNO3CBWaAocvmMw==\n\
-----END CERTIFICATE-----\n"
    /* You can comment out individual entries like this...
// GlobalSign Root CA (wikipedia.org)
"-----BEGIN CERTIFICATE-----\n\
MIIDXzCCAkegAwIBAgILBAAAAAABIVhTCKIwDQYJKoZIhvcNAQELBQAwTDEgMB4G\n\
A1UECxMXR2xvYmFsU2lnbiBSb290IENBIC0gUjMxEzARBgNVBAoTCkdsb2JhbFNp\n\
Z24xEzARBgNVBAMTCkdsb2JhbFNpZ24wHhcNMDkwMzE4MTAwMDAwWhcNMjkwMzE4\n\
MTAwMDAwWjBMMSAwHgYDVQQLExdHbG9iYWxTaWduIFJvb3QgQ0EgLSBSMzETMBEG\n\
A1UEChMKR2xvYmFsU2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjCCASIwDQYJKoZI\n\
hvcNAQEBBQADggEPADCCAQoCggEBAMwldpB5BngiFvXAg7aEyiie/QV2EcWtiHL8\n\
RgJDx7KKnQRfJMsuS+FggkbhUqsMgUdwbN1k0ev1LKMPgj0MK66X17YUhhB5uzsT\n\
gHeMCOFJ0mpiLx9e+pZo34knlTifBtc+ycsmWQ1z3rDI6SYOgxXG71uL0gRgykmm\n\
KPZpO/bLyCiR5Z2KYVc3rHQU3HTgOu5yLy6c+9C7v/U9AOEGM+iCK65TpjoWc4zd\n\
QQ4gOsC0p6Hpsk+QLjJg6VfLuQSSaGjlOCZgdbKfd/+RFO+uIEn8rUAVSNECMWEZ\n\
XriX7613t2Saer9fwRPvm2L7DWzgVGkWqQPabumDk3F2xmmFghcCAwEAAaNCMEAw\n\
DgYDVR0PAQH/BAQDAgEGMA8GA1UdEwEB/wQFMAMBAf8wHQYDVR0OBBYEFI/wS3+o\n\
LkUkrk1Q+mOai97i3Ru8MA0GCSqGSIb3DQEBCwUAA4IBAQBLQNvAUKr+yAzv95ZU\n\
RUm7lgAJQayzE4aGKAczymvmdLm6AC2upArT9fHxD4q/c2dKg8dEe3jgr25sbwMp\n\
jjM5RcOO5LlXbKr8EpbsU8Yt5CRsuZRj+9xTaGdWPoO4zzUhw8lo/s7awlOqzJCK\n\
6fBdRoyV3XpYKBovHd7NADdBj+1EbddTKJd+82cEHhXXipa0095MJ6RMG3NzdvQX\n\
mcIfeg7jLQitChws/zyrVQ4PkX4268NXSb7hLi18YIvDQVETI53O9zJrlAGomecs\n\
Mx86OyXShkDOOyyGeMlhLxS67ttVb9+E7gUJTb0o2HLO02JQZR7rkpeDMdmztcpH\n\
WD9f\n\
-----END CERTIFICATE-----\n"
*/;

/**
 *
 */
int httpSend(HttpRequest &request, HttpResponse &response,
             const char *pemBundleString, unsigned int pemBundleLength) {
  struct tls_config *cfg = NULL;
  struct tls *ctx = NULL;
  ssize_t writelen;
  HttpReader *httpReader = HttpReaderNew(response);

  /*
  ** initialize libtls
  */

  if (tls_init() != 0) {
    PERR(("tls_init:"));
    return 1;
  }

  /*
  ** configure libtls
  */

  if ((cfg = tls_config_new()) == NULL) {
    PERR(("tls_config_new:"));
    return 2;
  }

  /* set root certificate (CA) */
  if (tls_config_set_ca_mem(cfg, (const uint8_t *)pemBundleString,
                            pemBundleLength) != 0) {
    PERR(("tls_config_set_ca_mem:"));
    return 3;
  }

  /*
  ** initiate client context
  */

  if ((ctx = tls_client()) == NULL) {
    PERR(("tls_client:"));
    return 4;
  }

  /*
  ** apply config to context
  */

  if (tls_configure(ctx, cfg) != 0) {
    PERR(("tls_configure: %s", tls_error(ctx)));
    return 5;
  }

  /*
  ** connect to server
  */

  if (tls_connect(
          ctx, request.host.c_str(),
          (request.portstr.length() == 0 ? "443" : request.portstr.c_str())) !=
      0) {
    PERR(("tls_connect: %s", tls_error(ctx)));
    return 6;
  }

  /*
  ** send message to server
  */

  std::string requestStr = request.generate();

  if (flag_printRequest)
    printf("sending\n%s\n", requestStr.c_str());

  if ((writelen = tls_write(ctx, requestStr.c_str(), requestStr.length())) <
      0) {
    PERR(("tls_write: %s", tls_error(ctx)));
    return 7;
  }

  /*
  ** read response - headers and body
  */

  char readbuf[8192];
  size_t readlen;
  while ((readlen = tls_read(ctx, readbuf, sizeof(readbuf) - 1)) > 0) {
    readbuf[readlen] = 0;

    httpReader->onBuffer(readbuf, readlen, sizeof(readbuf));
    if (httpReader->isFinished())
      break;
  }
  delete httpReader;

  // clean up

  if (tls_close(ctx) != 0)
    err(1, "tls_close: %s", tls_error(ctx));
  tls_free(ctx);
  tls_config_free(cfg);

  return (0);
}

/**
 * @brief Simple function to read a small text file into 'dest' param.
 * @returns 0 on success, -1 otherwise.
 */
int read_text_file(const std::string path, std::string &dest) {
  char achLine[2048];
  FILE *pf = fopen(path.c_str(), "rt");
  if (NULL == pf)
    return -1;

  while (!feof(pf)) {
    char *pstr = fgets(achLine, sizeof(achLine), pf);
    if (pstr != NULL)
      dest.append(pstr);
  }

  fclose(pf);
  return 0;
}


/**
 * main
 */
int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("usage: %s url <optional/path/to/cabundle.pem>\n\n", argv[0]);
    return 1;
  }

  const char *url = argv[1];
  HttpRequest request = HttpRequest(url);
  if (request.host.length() == 0) {
    printf("Failed to parse URL:%s\n", url);
    return 2;
  }

  // by default, use pinned certs

  const char *pemBundleString = pinnedCertsPEM;
  unsigned int pemBundleLength = sizeof(pinnedCertsPEM);

  std::string pemBundleFileContents = std::string();
  if (argc > 2) {

    // try to load PEM bundle from file

    const char *pem_filename = argv[2];
    int status = read_text_file(pem_filename, pemBundleFileContents);
    if (status != 0) {
      printf("Error reading PEM bundle file at '%s'\n", pem_filename);
      return 2;
    }
    pemBundleString = pemBundleFileContents.c_str();
    pemBundleLength = pemBundleFileContents.length();
  }

  HttpResponse response = HttpResponse();

  request.customHeaders.push_back(HttpHeader("User-Agent", USER_AGENT));

  if (httpSend(request, response, pemBundleString, pemBundleLength)) {
    printf("request failed\n");
    return 3;
  }

  printf("success\n statusCode:%d headers:%lu response:%lu bytes\n",
         response.statusCode, response.headers.size(), response.body.length());

  if (flag_printResponseHeaders) {
    printf("Response Headers:\n");
    for (auto hdr : response.headers) {
      printf("%s:%s\n", hdr.name.c_str(), hdr.value.c_str());
    }
  }

  return 0;
}
