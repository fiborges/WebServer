#include <stdio.h>
#include <curl/curl.h>

// c++ -o fred fred.cpp -lcurl


int main(void) {
    CURL *curl;
    CURLcode res;
    
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://example.com");
        
        // Define que você quer receber o header do response
        curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
        
        // Define que você não quer receber o body do response
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        
        // Realiza o request
        res = curl_easy_perform(curl);
        
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        
        // Obtém o tipo de request (GET ou POST)
        char *requestType;
        curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_METHOD, &requestType);
        printf("Tipo de request: %s\n", requestType);
        
        // Obtém o tamanho do header
        long headerSize;
        curl_easy_getinfo(curl, CURLINFO_HEADER_SIZE, &headerSize);
        printf("Tamanho do header: %ld bytes\n", headerSize);
        
        curl_easy_cleanup(curl);
    }
    
    return 0;
}
