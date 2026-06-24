#include <iostream>
#include <stdexcept>
#include <string>
#include <Spotify/crypto.hpp>
#include <Spotify/spotify.hpp>
#include <Spotify/auth.hpp>
#include <fstream>
#include <sstream>
#include <string_view>

void testHarness();

int main() {
    testHarness();
    return 0;
}

std::string getClientIDFromConfig() {
    std::ifstream file("config.txt");
    std::string line;
    while (std::getline(file, line)) {
        if (line.rfind("client_id=", 0) == 0) {
            return line.substr(10); //reads all after client_id=
        }
    }
    throw std::runtime_error("could not find client_id in config.txt");
}

std::string extractJsonValue(std::string_view json, std::string_view key) {
    std::string searchKey = "\"" + std::string(key) + "\":\"";
    size_t startPos = json.find(searchKey);
    if (startPos == std::string::npos) return "";

    startPos += searchKey.length();
    size_t endPos = json.find("\"", startPos);
    if (endPos == std::string::npos) return "";

    return std::string(json.substr(startPos, endPos - startPos));
}

void runSpotifyController() {
    using namespace std::string_view_literals;

    std::string clientID = getClientIDFromConfig();
    //use token to establish connection with api
    auth::AuthRequest request(
        clientID,
        "https://google.com"sv, //dummy
        "user-modify-playback-state user-read-playback-state"sv
    );

    //login link output
    std::string loginURL = auth::buildAuthURL(request);
    std::cout << "\n=-=-=-SPOTIFY-=-=-AUTHENTICATION-=-=-=\n";
    std::cout << "=-=-=-LOGIN-=-=-URL-=-=-=:\n\n" << loginURL << "\n\n";

    //capture token manually
    std::cout << "Copy resulting URL and paste it below: ";
    std::string fullRedirectURL; 
    std::cin >> fullRedirectURL;

    size_t codePos = fullRedirectURL.find("code=");
    if (codePos == std::string::npos) {
        std::cout << "Error: URL not valid.\n";
        return;
    }

    codePos += 5;
    size_t ampPos = fullRedirectURL.find("&", codePos);
    std::string authCode = (ampPos == std::string::npos) 
        ? fullRedirectURL.substr(codePos)
        : fullRedirectURL.substr(codePos, ampPos - codePos);

    std::cout << "\nExchanging auth code for token... please wait...\n";

    std::string rawJSONResponse = auth::swapCodeForToken(request, authCode);

    std::string accessToken = extractJsonValue(rawJSONResponse, "access_token");
    if (accessToken.empty()) {
        std::cout << "Swap Error: " << rawJSONResponse << "\n";
        return;
    }
    //now with token, initialize engine clientside
    Spotify spotify(request, accessToken);
    std::cout << "\n-=-=-=-=-=-Lify Connected-=-=-=-=-=-\n";
    std::cout << "\nCOMMANDS AS FOLLOWS: play, pause, track <id>, back\n\n";

    std::string action;
    while (std::cout << "lify> " && std::cin >> action) {
        if (action == "play") {
            spotify.play();
            std::cout << "playback started/resumed.\n";
        } else if (action == "pause") {
            spotify.pause();
            std::cout << "playback paused.\n";
        } else if (action == "track") {
            std::string trackId;
            std::cin >> trackId;
            std::cout << "grabbing track info...\n" << spotify.getTrack(trackId) << "\n";
        } else if (action == "back") { 
            break;
        } else {
            std::cout << "unknown command.\n";    
        }
    }
}

std::string readMessage() {
    std::string message;

    if (!std::getline(std::cin, message)) 
        throw std::runtime_error("Failed to read input.");

    if (!message.empty() && message[0] == ' ')
        message.erase(0, 1);

    return message;
}

void testHarness() {
    std::string command;
    std::string message;

    while (std::cin >> command) {
        if (command == "sha") {
            std::cout << crypto::toHex(crypto::sha256(readMessage())) << '\n';

        } else if (command == "base64") {
            std::string message = readMessage();

            std::span<const unsigned char> bytes{
                reinterpret_cast<const unsigned char*>(message.data()),
                    message.size()
            };

            std::cout << crypto::base64_encode(bytes) << '\n';
        
        } else if (command == "spotify") {
            runSpotifyController();

        } else if (command == "exit") {
            break;
        } else {
            std::cout << "Invalid command\n";
        }
    }
}


