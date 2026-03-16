// IMapDraftPlugin.h

#ifndef IMAPDRAFTPLUGIN_H
#define IMAPDRAFTPLUGIN_H

#include <NppPlugin.h>

class IMapDraftPlugin : public NppPlugin
{
public:
    IMapDraftPlugin();
    ~IMapDraftPlugin();

    void init();
    void uninit();

    // Additional plugin methods and members can be added here
};

#endif // IMAPDRAFTPLUGIN_H
