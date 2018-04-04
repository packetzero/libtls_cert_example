#include <sstream>
#include <string.h>

#include <openssl/ocsp.h> // OCSP_parse_url

#include "http.h"

void parseHeaders(const std::string &str, size_t &contentLength, std::vector<HttpHeader> &hdrs, int &statusCode);


HttpRequest::HttpRequest(const char *purl) : url(purl), host(), portstr(), path(), isHttps(false), method("GET"), postData(), customHeaders()
{
//  char tmpurl[1024];
//  strcpy(tmpurl, argv[1]);  // make a copy since OCSP_parse_url url param is not const
  char *_host=NULL;
  char *_portstr=NULL;
  char *_path=NULL;
  int _isHttps = 0;
  
  if (1 != OCSP_parse_url((char *)url.c_str(), &_host, &_portstr, &_path, &_isHttps)) {
    //printf("Failed to parse URL:%s\n", url);
    return;
  }
  
  host = std::string(_host);
  portstr = std::string(_portstr);
  path = std::string(_path);
  isHttps = isHttps != 0;
}

const std::string HttpRequest::generate()
{
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
