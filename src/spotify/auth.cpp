#include <sstream>
#include <string>
#include <array>
#include <Spotify/crypto.hpp>
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
                std::string_view scope)
            : client_id { client_id }
            , redirect_uri { redirect_uri }
            , scope { scope }
            , state { crypto::randomStrGen(43) }
            , code_verifier { crypto::randomStrGen(128) }
            {
                code_challenge = crypto::base64_encode(crypto::sha256(code_verifier));
            }
    };


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
}
