#include "Client/Client.hpp"

#include <Engine/Core/Log.hpp>

int main() {
    Log::init();

    auto client = std::make_unique<Client>();
    client->run();
}
