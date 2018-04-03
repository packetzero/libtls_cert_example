#ifndef _HTTP_H_
#define _HTTP_H_

#include <string>
#include <vector>

// bare minimum support to perform HTTP 1.1 GET

struct HttpHeader {
  std::string name;
  std::string value;
  HttpHeader(std::string n, std::string v) : name(n), value(v) {}
};

struct HttpResponse {
  int statusCode;
  std::string body;
  std::vector<HttpHeader> headers;
};

class HttpReader {
public:

  virtual void onBuffer(const char *buf, size_t readlen, size_t buflen) = 0;

  virtual bool isFinished() = 0;

  virtual ~HttpReader() {}
};

HttpReader* HttpReaderNew(HttpResponse& dest);

#endif // _HTTP_H_
