#include "Connection.h"
#include "Logging.h"

#include <sstream>
#include <cstring>

using namespace Example;

void Connection::Connect(const std::string& host, int port)
{
   m_socket.reset(new simgear::Socket());
   const bool isStreamingSocket = false; // Create UDP socket
   if (!m_socket->open(isStreamingSocket))
   {
      std::cerr << "ERROR: unable to create broadcast socket" << std::endl;
      abort();
   }

   m_socket->setBlocking(false);
   m_socket->setBroadcast(true);

   if (m_socket->connect(host.c_str(), port) == -1)
   {
      std::cerr << "ERROR: unable to connect to broadcast socket: " << host << " " << std::to_string(port) << std::endl;
      abort();
   }
}

void Connection::Disconnect()
{
   m_socket.reset();
}

void Connection::Send(const char* buf, int numbytes)
{
   if( numbytes > 0)
   {
      m_socket->send(buf, numbytes);
   }
}

void Connection::HandleError()
{
   abort();
}
