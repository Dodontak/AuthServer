cd "$(dirname "$0")"
openssl genpkey -algorithm RSA -out server.key -pkeyopt rsa_keygen_bits:2048
openssl req -new -x509 -key server.key -out server.crt -days 365