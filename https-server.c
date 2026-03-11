#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<openssl/err.h>
#include<pthread.h>
#include<openssl/ssl.h>



void* http_server(void* arg){
    int server_fd = socket(AF_INET,SOCK_STREAM,0); //创建socket

    struct sockaddr_in addr;//创建地址
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    addr.sin_addr.s_addr = INADDR_ANY;
    
    int suc = bind(server_fd,(struct sockaddr*)&addr,sizeof(addr));//绑定socket和本地地址
    if(suc != 0){printf("bind falled.");return NULL;}

    listen(server_fd,10);//socket设置为监听模式
    printf("http 服务器启动，监听 88 端口\n");

    while(1){
        int client_fd = accept(server_fd,NULL,NULL);

        char buffer[1024] = {0};
        if(recv(client_fd,buffer,sizeof(buffer),0) == -1){printf("接收失败");return NULL;}
        printf("收到请求：\n%s\n",buffer);

        // 在 http_server 收到请求后：
        char path[256] = {0};
        sscanf(buffer, "GET %s HTTP", path); 
        char response[512];
        sprintf(response, 
            "HTTP/1.1 301 Moved Permanently\r\n"
            "Location: https://10.0.0.1%s\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n"
            "\r\n", path);
        send(client_fd, response, strlen(response), 0);
        close(client_fd);
    }

    close(server_fd);
    return 0;
}

void* https_server(void* arg) {
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    
    const SSL_METHOD *method = TLS_server_method();
    SSL_CTX *ctx = SSL_CTX_new(method);

    SSL_CTX_use_certificate_file(ctx, "keys/cnlab.cert", SSL_FILETYPE_PEM);
    SSL_CTX_use_PrivateKey_file(ctx, "keys/cnlab.prikey", SSL_FILETYPE_PEM);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(443); // HTTPS 标准端口
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 10);

    printf("HTTPS 服务已启动，监听 443 端口...\n");

    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        SSL *ssl = SSL_new(ctx);
        SSL_set_fd(ssl, client_fd);

        if (SSL_accept(ssl) > 0) {
            char buffer[2048] = {0};
            SSL_read(ssl, buffer, sizeof(buffer));

            char method[10], path[256];
            sscanf(buffer, "%s %s", method, path);
            char *filename = path + 1; // 去掉开头的/
            if(strlen(filename) == 0) filename = "index.html";

            FILE *f = fopen(filename, "rb");
            if (!f) {
                // 404 处理
                char *res404 = "HTTP/1.1 404 Not Found\r\nContent-Length: 9\r\n\r\nNot Found";
                SSL_write(ssl, res404, strlen(res404));
            } else {
                fseek(f, 0, SEEK_END);
                long total_size = ftell(f);
                rewind(f);

                char *range_ptr = strstr(buffer, "Range: bytes=");
                if (range_ptr) {
                    //206 Partial Content
                    long start = 0, end = total_size - 1;
                    sscanf(range_ptr, "Range: bytes=%ld-%ld", &start, &end);
                    if (end == 0 || end >= total_size) end = total_size - 1;

                    long len = end - start + 1;
                    char header[256];
                    sprintf(header, "HTTP/1.1 206 Partial Content\r\n"
                                    "Content-Range: bytes %ld-%ld/%ld\r\n"
                                    "Content-Length: %ld\r\n\r\n", start, end, total_size, len);
                    SSL_write(ssl, header, strlen(header));
                    
                    fseek(f, start, SEEK_SET);
                    char *file_buf = malloc(len);
                    fread(file_buf, 1, len, f);
                    SSL_write(ssl, file_buf, len);
                    free(file_buf);
                } else {
                    //200 OK
                    char header[256];
                    sprintf(header, "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\n\r\n", total_size);
                    SSL_write(ssl, header, strlen(header));
                    
                    char *file_buf = malloc(total_size);
                    fread(file_buf, 1, total_size, f);
                    SSL_write(ssl, file_buf, total_size);
                    free(file_buf);
                }
                fclose(f);
            }
        }

        //清理连接
        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(client_fd);
    }

    SSL_CTX_free(ctx);
    return NULL;
}

int main() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, http_server, NULL);
    pthread_create(&t2, NULL, https_server, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    return 0;
}
