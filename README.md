# libtls_cert_example
C++ example of pinned certs and [LibreSSL](https://www.libressl.org) libtls without the use of libcurl.

The code shows a clear delineation between the TLS layer and HTTP layer.  As opposed to many examples that use libcurl in tandem with openssl or libressl, this example uses tls_connect() to establish the socket and TLS session.  The trade-off is that we have to provide the HTTP logic to render the request and parse the response.  The files http.h and http.cpp provide the barebones to do this at HTTP v1.1.  If you desire redirect following, you have to implement it in your application.

# Certificates
Your CA bundle or list of authorized certificates are placed in a static
string.  It's a concatenation of certs in PEM format.  The PEM format is
simply a Base64 encoding of the binary DER format with BEGIN and END CERTIFICATE
lines.

# Example of successful request using pinned certs
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

# Example of successful request using ca bundle file
Passing a second argument, you can specify a PEM text file that contains a concatenation of trusted certs, rather than using the pinned cert string.
```
tlsexample https://www.wikipedia.org/ path/to/cabundle.pem
success
 statusCode:200 headers:28 response:221418 bytes
```



### Building
You need to provide include and lib paths for LibreSSL (libssl.a, libcrypto.a, libtls.a) and zlib in order to build.  Pass "-G Xcode" argument to cmake to build for xcode, and then replace the 'make' below with `xcodebuild -configuration Release` or simply `xcodebuild` to build Debug.
```
mkdir build && cd build
cmake -DCMAKE_CXX_FLAGS=-I~/dependency/include -DCMAKE_EXE_LINKER_FLAGS=-L~/dependency/lib ..
make
```
