#include <sstream>
#include <string.h>

#include "http.h"

void parseHeaders(const std::string &str, size_t &contentLength, std::vector<HttpHeader> &hdrs, int &statusCode);

class HttpReaderImpl : public HttpReader {
public:
  HttpReaderImpl(HttpResponse& resp) : _response(resp), _haveHeaders(false), _respHeaderBuffer(), _contentLength(0), _bodyReadLen(0), _isFinished(false) { }

  /*
   * Receive buffer from transport.  Assume headers, followed by body.
   */
  virtual void onBuffer(const char *readbuf, size_t readlen, size_t buflen) override
  {
    if (!_haveHeaders) {
      const char *pos = strstr(readbuf, "\r\n\r\n");
      if (pos == 0L) pos = strstr(readbuf, "\n\n");

      if (pos == 0L) {
        _respHeaderBuffer += readbuf;
        return;
      } else {
        _respHeaderBuffer += std::string(readbuf, pos);
        _haveHeaders = true;

        parseHeaders(_respHeaderBuffer, _contentLength, _response.headers, _response.statusCode);

        while (*pos == '\r' || *pos == '\n') pos++;
        _bodyReadLen += strlen(pos);
        _response.body += std::string(pos);
      }
    } else {
      _bodyReadLen += strlen(readbuf);
      _response.body += readbuf;
    }
    if (_contentLength == 0 && readlen < sizeof(readbuf)-1) _isFinished = true;
    if (_contentLength > 0 && _bodyReadLen >= _contentLength) _isFinished = true;
  }

  virtual bool isFinished() override { return _isFinished; }

private:
  HttpResponse& _response;
  bool          _haveHeaders;
  std::string   _respHeaderBuffer;
  size_t        _contentLength;
  size_t        _bodyReadLen;
  bool          _isFinished;
};

HttpReader* HttpReaderNew(HttpResponse& dest) { return new HttpReaderImpl(dest); }

std::vector<std::string> split(const std::string &subject, char delim)
{
  std::vector<std::string> retval = std::vector<std::string>();
  std::istringstream f(subject);
  std::string s;

  while(getline(f,s, delim)) {
    retval.push_back(s);
  }
  return retval;
}

/**
 * Parses HTTP headers found in str.
 * Places each header found into hdrs vector.
 * Puts header line into a '_status' header and adds to hdrs.
 * If "Content-Length" header is found, parses value and sets contentLength param
 */
void parseHeaders(const std::string &str, size_t &contentLength, std::vector<HttpHeader> &hdrs, int &statusCode)
{
  auto lines = split(str, '\n');
  int i=-1;
  for (auto line : lines) {
    i++;
    //printf("HEADER:%s\n", line.c_str());

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
