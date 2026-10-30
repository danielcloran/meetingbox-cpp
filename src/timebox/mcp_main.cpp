#include "timebox/host.hpp"
#include "timebox/mcp.hpp"

#include <iostream>
#include <string>

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  TimeboxHost host;
  host.start();
  std::cerr << "timebox_mcp: stdio JSON-RPC, " << TimeboxHost::kCapacity
            << " slots, headless SDL\n";

  std::string line;
  while (std::getline(std::cin, line)) {
    const std::string response = handle_mcp_message(host, line);
    if (!response.empty()) {
      std::cout << response << '\n';
      std::cout.flush();
    }
  }
  host.shutdown();
  return 0;
}
