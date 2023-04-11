#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define BUFFER_SIZE 1024

using namespace std;

int main(int argc, char* argv[])
{
    if (argc != 3) {
        // Replace Host Address and Port with your port to decide to forward.
        cerr << "Usage: " << argv[0] << " <Host> <Port>" << endl;
        return 1;
    }

    const char* host = argv[1];
    int port = atoi(argv[2]);
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        cerr << "Failed to create socket." << endl;
        return 1;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(host);
    addr.sin_port = htons(port);
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        cerr << "Failed to connect to remote host." << endl;
        return 1;
    }
    SSL_CTX* ctx = SSL_CTX_new(TLSv1_2_client_method());
    SSL* ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock);
    SSL_connect(ssl);
    while (true) {
        char buffer[BUFFER_SIZE] = {0};
        int numBytes = SSL_read(ssl, buffer, BUFFER_SIZE);
        if (numBytes <= 0) {
            cerr << "Error reading data from socket." << endl;
            break;
        }
        int encryptedBytes = SSL_write(ssl, buffer, numBytes);
        if (encryptedBytes <= 0) {
            cerr << "Error sending encrypted data back to socket." << endl;
            break;
        }
    }
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    close(sock);

    return 0;
}
