// opendis_manager.h
//
// Copyright (C) 2021 - AlphaPixel (http://www.alphapixel.com)
#include <memory>

class OpenDISManager
{
public:
    static std::unique_ptr<OpenDISManager> create();
    virtual ~OpenDISManager();

    void update(double delta_time_sec);

private:
    OpenDISManager();
};
