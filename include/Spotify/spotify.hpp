#pragma once
#include <string>
#include "auth.hpp"

class Spotify {
private: 
    const auth::AuthRequest& m_authRequest;
    std::string m_accessToken;
    
    std::string sendRequest(const std::string& method, const std::string& endpoint, const std::string& body = "");

public: 
    explicit Spotify(const auth::AuthRequest& authRequest, std::string accessToken);

    std::string getTrack(const std::string& trackID) const;
    std::string search(const std::string& query, const std::string& type = "track");
    bool play(const std::string& trackuri = "");
    bool pause();
};
    