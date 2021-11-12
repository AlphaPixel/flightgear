#pragma once

namespace Example
{
   /// grabs the time of day
   class Timer
   {
   public:
      Timer();
      void Update();

      double GetSeconds() const;

   private:
      long long _ticks;
   };
}

