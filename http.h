#ifndef _HTTP_H_
#define _HTTP_H_

#include <string>
#include <vector>

// bare minimum support to perform HTTP 1.1 GET

/**
 * Encapsulates HTTP header
 */
struct HttpHeader {
  std::string name;
  std::string value;
  HttpHeader(std::string n, std::string v) : name(n), value(v) {}
};

/**
 * Encapsulates HTTP request
 */
struct HttpRequest {
  std::string url;
  std::string host;
  std::string portstr;
  std::string path;
  bool isHttps;

  std::string method; // defaults to GET
  std::string postData;
  std::vector<HttpHeader> customHeaders;

  /*
   * If unable to parse url, host will be empty string.
   */
  HttpRequest(const char *url);

  /*
   * returns string with HTTP/1.1 method, headers, body, line break
   */
  const std::string generate();
};

/**
 * Encapsulates HTTP response
 */
struct HttpResponse {
  int statusCode;
  std::string body;
  std::vector<HttpHeader> headers;
};

class HttpReader {
public:
  /**
   * @brief Process buffer from transport layer.
   * Usage:
   *   while(!httpReader->isFinished()) {
   *      auto readlen = transport_layer_read(context, buf, buflen);
   *      httpReader->onBuffer(buf,readlen, buflen);
   *   }
   */
  virtual void onBuffer(const char *buf, size_t readlen, size_t buflen) = 0;

  /**
   * IsFinished = if (readlen < buflen) || bytes_read == Content-Length)
   * @returns true if finished reading response or no more data, false if more data expected.
   */
  virtual bool isFinished() = 0;

  virtual ~HttpReader() {}
};

/**
 * returns instance of HttpReader implementation.
 * When isFinished() returns true, dest will contain
 * HTTP response.
 */
HttpReader *HttpReaderNew(HttpResponse &dest);

#endif // _HTTP_H_
