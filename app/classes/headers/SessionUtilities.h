#ifndef SESSIONUTILITIES_H
#define SESSIONUTILITIES_H

#include <cstdint>

enum MessageFlags : char
{
    InitPlayerList = '1',
    ReqInitPlayer = '2',
    ResInitPlayer = '3',
    TargetFieldsReviel = '4',
    BroadcastFieldsReviel = '5'

};

enum MainMenuWidgetIndexes : uint8_t
{
    Home = 0,
    HostServer,
    ClientConnect,
    Settings,
    Exit
};
#endif // SESSIONUTILITIES_H
