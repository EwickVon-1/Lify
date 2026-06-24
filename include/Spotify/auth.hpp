#pragma once
#include <string>
#include <string_view>

namespace auth {
    struct AuthRequest {
        std::string client_id;
        std::string redirect_uri;
        std::string scope;
        std::string state;
        std::string code_verifier;
        std::string code_challenge;

        AuthRequest(std::string_view client_id,
                std::string_view redirect_uri,
                std::string_view scope);
    };

    std::string buildAuthURL(const AuthRequest& request);

    std::string swapCodeForToken(const AuthRequest& request, std::string_view authCode);
} 
