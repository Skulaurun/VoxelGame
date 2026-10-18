#include "Server/Server.hpp"

#include <Engine/Core/Log.hpp>

int main() {
    Log::init();

    auto server = std::make_unique<Server::Server>();
    server->run();
}
