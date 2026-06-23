#include "spotify/spotify.hpp"
#include <iostream>
#include <curl/curl.h>

// constructor implementation
Spotify::Spotify(const auth::AuthRequest& authRequest, std::string accessToken)
    : m_authRequest(authRequest), m_accessToken(std::move(accessToken)) {}

// helper callback for libcurl to write incoming data into string
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// implemention of network engine
std::string Spotify::sendRequest(const std::string& method, const std::string& endpoint, const std::string& body) {
    CURL* curl = curl_easy_init();
    std::string responseString;

    if (curl) {
        std::string url = "https://api.spotify.com" + endpoint;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());

        // now bring OAuth token to request header
        struct curl_slist* headers = nullptr;
        std::string authHeader = "Authorization: Bearer " + m_accessToken;
        headers = curl_slist_append(headers, authHeader.c_str());
        headers = curl_slist_append(headers, "Content Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // now time for the body (payload)!!!
        if (!body.empty()) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        }

        // in case of errors:
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);

        // we're ready to network!!
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        //clean-up
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    return responseString;
}

//GET
std::string Spotify::getTrack(const std::string& trackID) const {
    return const_cast<Spotify*>(this)->sendRequest("GET", "/v1/tracks/" + trackID);
}

//GET with body
std::string Spotify::search(const std::string& query, const std::string& type) {
    return sendRequest("GET", "/v1/search?q=" + query + "&type=" + type);
}

//PUT (play)
bool Spotify::play(const std::string& trackuri) {
    std::string body = "";
    if (!trackuri.empty()) {
        body = "{\"uris\": [\"" + trackuri + "\"]}";
    }
    sendRequest("PUT", "/v1/me/player/play", body);
    return true;
}

//PUT (pause)
bool Spotify::pause() {
    sendRequest("PUT", "/v1/me/player/pause");
    return true;
}