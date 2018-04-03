# libtls_cert_example
C++ example of pinned certs and LibreSSL libtls without libcurl.

# Certificates
Your CA bundle or list of authorized certificates are placed in a static
string.  It's a concatenation of certs in PEM format.  The PEM format is
simply a Base64 encoding of the binary DER format with BEGIN and END CERTIFICATE
lines.
