# libtls_cert_example
C++ example of pinned certs and LibreSSL libtls without libcurl.

The code shows a clear delineation between the TLS layer and HTTP layer.  As opposed to many examples that use libcurl in tandem with openssl or libressl, this example uses tls_connect() to establish the socket and TLS session.  The trade-off is that we have to provide the HTTP logic to render the request and parse the response.  The http.h and http.cpp is the barebones to do this at HTTP v1.1.

# Certificates
Your CA bundle or list of authorized certificates are placed in a static
string.  It's a concatenation of certs in PEM format.  The PEM format is
simply a Base64 encoding of the binary DER format with BEGIN and END CERTIFICATE
lines.

# Example of successful request
Setting the flag_printRequest and flag_printResponseHeaders to true will yield more output.  By default, only the following is output.
```
tlsexample https://www.google.com/
success
 statusCode:200 headers:16 response:218468 bytes
```

# Example of failed (cert not in pinned bundle)

```
tlsexample: tls_write: certificate verification failed: unable to get local issuer certificate: Undefined error: 0
request failed
Program ended with exit code: 3
```
