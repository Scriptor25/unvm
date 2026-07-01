#pragma once

#include <http/client.hxx>

#include <memory>

namespace unvm
{
    toolkit::result<std::unique_ptr<http::Transport>> CreateTransport();
}
