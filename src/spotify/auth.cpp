#include <sstream>
#include <iostream>
#include <string>
#include <array>
#include <Spotify/crypto.hpp>
#include <string_view>
#include <curl/curl.h>
#include "Spotify/auth.hpp"

namespace {
    //saving json data
    size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
        size_t totalSize = size * nmemb;
        userp->append(static_cast<char*>(contents), totalSize);
        return totalSize;
    }
}

namespace auth {
    AuthRequest::AuthRequest(std::string_view client_id, 
            std::string_view redirect_uri,
            std::string_view scope)
        : client_id{client_id}
    , redirect_uri{redirect_uri}
    , scope{scope}
    , state{crypto::randomStrGen(43)}
    , code_verifier{crypto::randomStrGen(128)}
    {
        code_challenge = crypto::base64_encode(
                crypto::sha256(code_verifier));
    }

    std::string buildAuthURL(const AuthRequest& request) {
        std::ostringstream url;

        url << "https://accounts.spotify.com/authorize"
            << "?response_type=code"
            << "&client_id=" << request.client_id
            << "&scope=" << crypto::urlEncode(request.scope)
            << "&redirect_uri=" << crypto::urlEncode(request.redirect_uri)
            << "&code_challenge=" << request.code_challenge
            << "&code_challenge_method=S256"
            << "&state=" << request.state;

        return url.str();

    }

    std::string swapCodeForToken(const AuthRequest& request, std::string_view authCode) {
        CURL* curl = curl_easy_init();
        std::string responseBuffer;

        if (!curl) {
            curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
            return "{\"error!!!!!\": \"failed to curl\"}";
        }

        curl_easy_setopt(curl, CURLOPT_URL, "https://accounts.spotify.com/api/token");

        std::ostringstream payload;
        payload << "client_id=" << request.client_id
                << "&grant_type=authorization_code"
                << "&code=" << authCode
                << "&redirect_uri=" << crypto::urlEncode(request.redirect_uri)
                << "&code_verifier=" << request.code_verifier;
        
        std::string postFields = payload.str();
        std::cout << "\n[DEBUG]:\n" << postFields << "\n\n";
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            responseBuffer = "{\"error\": \"" + std::string(curl_easy_strerror(res)) + "\"}";
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        return responseBuffer;
    }
}

