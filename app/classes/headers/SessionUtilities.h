#ifndef SESSIONUTILITIES_H
#define SESSIONUTILITIES_H

#include <cstdint>

enum MessageFlags : char
{
    InitPlayerList = '1',
    ReqInitPlayer = '2',
    ResInitPlayer = '3',
    TargetFieldsReveal = '4',
    BroadcastFieldsReveal = '5',
    ReqChangeNicknameDublicate = '6',
    InitPlayerFields = '7',
    StartGame = '8'

};

enum MainMenuWidgetIndexes : uint8_t
{
    Home = 0,
    HostServer,
    ClientConnect,
    InGame
};
#endif // SESSIONUTILITIES_H
