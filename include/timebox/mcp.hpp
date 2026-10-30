#ifndef TIMEBOX_MCP_HPP
#define TIMEBOX_MCP_HPP

#include "timebox/host.hpp"

#include <string>

// One newline-delimited JSON-RPC message. Returns the response line without a
// trailing newline, or an empty string when the message is a notification.
std::string handle_mcp_message(TimeboxHost &host, const std::string &line);

#endif
