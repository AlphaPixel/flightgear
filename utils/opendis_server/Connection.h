#pragma once

#include <string>                        // for param
#include <cstddef>                       // for size_t definition
#include <memory>
#include <simgear/io/raw_socket.hxx>

namespace Example
{
   class Connection
   {
   public:
      void Connect(const std::string& host, int port);
      void Disconnect();

      void Send(const char* buf, int numbytes);

   private:
      void HandleError();

      std::unique_ptr<simgear::Socket> m_socket;
   };
}

