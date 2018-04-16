#include <sstream>
#include <string.h>

#include <openssl/ocsp.h> // OCSP_parse_url

#include "http.h"

void parseHeaders(const std::string &str, size_t &contentLength,
                  std::vector<HttpHeader> &hdrs, int &statusCode);
/**
 * @brief Constructor - parses URL and initializes members.
 */
HttpRequest::HttpRequest(const char *purl)
    : url(purl), host(), portstr(), path(), isHttps(false), method("GET"),
      postData(), customHeaders() {

  char *_host = NULL;
  char *_portstr = NULL;
  char *_path = NULL;
  int _isHttps = 0;

  if (1 != OCSP_parse_url((char *)url.c_str(), &_host, &_portstr, &_path,
                          &_isHttps)) {
    // printf("Failed to parse URL:%s\n", url);
    return;
  }

  host = std::string(_host);
  portstr = std::string(_portstr);
  path = std::string(_path);
  isHttps = isHttps != 0;
}

/**
 * @brief Builds and returns the HTTP request string based
 * on member field values.
 */
const std::string HttpRequest::generate() {
  std::string request = method + std::string(" ");
  request += path;
  request += " HTTP/1.1\r\n";

  request += "Host: " + std::string(host) + "\r\n";

  for (auto hdr : customHeaders) {
    request += hdr.name + ": " + hdr.value + "\r\n";
  }

  request += "Content-Length: ";
  char tmp[32];
  sprintf(tmp, "%lu", postData.length());
  request += tmp;
  request += "\r\n\r\n";
  request += postData;

  return request;
}

/**
 * Implementation of HttpReader interface.
 */
class HttpReaderImpl : public HttpReader {
public:
  HttpReaderImpl(HttpResponse &resp)
      : _response(resp), _haveHeaders(false), _respHeaderBuffer(),
        _contentLength(0), _bodyReadLen(0), _isFinished(false) {}

  /*
   * Receive buffer from transport.
   * Keeps state in _haveHeaders, _bodyReadLen, _isFinished.
   */
  virtual void onBuffer(const char *readbuf, size_t readlen,
                        size_t buflen) override {
    if (!_haveHeaders) {

      // look for end of header marker : 2 newlines

      const char *pos = strstr(readbuf, "\r\n\r\n");
      if (pos == 0L)
        pos = strstr(readbuf, "\n\n");

      if (pos == 0L) {

        // no end marker, append this buffer to internal header buffer

        _respHeaderBuffer += readbuf;
        return;

      } else {

        // we found the end of header marker

        _respHeaderBuffer += std::string(readbuf, pos);
        _haveHeaders = true;

        parseHeaders(_respHeaderBuffer, _contentLength, _response.headers,
                     _response.statusCode);

        // move to start of body

        while (*pos == '\r' || *pos == '\n')
          pos++;

        // append to internal body buffer

        _bodyReadLen += strlen(pos);
        _response.body += std::string(pos);
      }
    } else {

      // append to internal body buffer

      _bodyReadLen += strlen(readbuf);
      _response.body += readbuf;
    }

    // if readlen < size of buffer, assume finished

    if (_contentLength == 0 && readlen < sizeof(readbuf) - 1)
      _isFinished = true;

    // if read contentLength bytes, assume finished

    if (_contentLength > 0 && _bodyReadLen >= _contentLength)
      _isFinished = true;
  }

  virtual bool isFinished() override { return _isFinished; }

private:
  HttpResponse &_response;
  bool _haveHeaders;
  std::string _respHeaderBuffer;
  size_t _contentLength;
  size_t _bodyReadLen;
  bool _isFinished;
};

HttpReader *HttpReaderNew(HttpResponse &dest) {
  return new HttpReaderImpl(dest);
}

/**
 * Simple string split()
 */
std::vector<std::string> split(const std::string &subject, char delim) {
  std::vector<std::string> retval = std::vector<std::string>();
  std::istringstream f(subject);
  std::string s;

  while (getline(f, s, delim)) {
    retval.push_back(s);
  }
  return retval;
}

/**
 * Parses HTTP headers found in str.
 * Places each header found into hdrs vector.
 * Puts header line into a '_status' header and adds to hdrs.
 * If "Content-Length" header is found, parses value and sets contentLength
 * param
 */
void parseHeaders(const std::string &str, size_t &contentLength,
                  std::vector<HttpHeader> &hdrs, int &statusCode) {
  auto lines = split(str, '\n');
  int i = -1;
  for (auto line : lines) {
    i++;
    // printf("HEADER:%s\n", line.c_str());

    // TODO: trim \r

    if (i == 0) {
      hdrs.push_back(HttpHeader("_status", line));
      // HTTP/1.1 200 OK
      auto words = split(line, ' ');
      if (words.size() >= 3) {
        statusCode = atol(words[1].c_str());
      }

      continue;
    }

    auto parts = split(line, ':');
    if (parts.size() <= 1) {
      printf("Invalid header - no colon\n");
      continue;
    }

    hdrs.push_back(HttpHeader(parts[0], parts[1]));

    if (line.find("Content-Length") != std::string::npos) {
      contentLength = atol(parts[1].c_str());
    }
  }
}
