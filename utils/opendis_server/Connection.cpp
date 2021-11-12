#include "Connection.h"
#include "Logging.h"

#include <sstream>
#include <cstring>

using namespace Example;

void Connection::Connect(unsigned int port, const std::string& host, bool listening)
{
}

void Connection::Disconnect()
{
}

void Connection::Send(const char* buf, size_t numbytes)
{
   if( numbytes < 1 )
   {
      return;
   }
}

size_t Connection::Receive(char* buf)
{
   return 0;
}

void Connection::HandleError()
{
   abort();
}
