#include <unvm/util.hxx>

#include <openssl/err.h>

std::string unvm::GetSSLErrorStack()
{
    std::vector<std::string> stack;

    for (unsigned long error; (error = ERR_get_error());)
    {
        char buf[256];
        ERR_error_string_n(error, buf, sizeof(buf));

        stack.push_back(buf);
    }

    std::string message;
    for (auto it = stack.begin(); it != stack.end(); ++it)
    {
        if (it != stack.begin())
        {
            message += ": ";
        }

        message += *it;
    }

    return message;
}
