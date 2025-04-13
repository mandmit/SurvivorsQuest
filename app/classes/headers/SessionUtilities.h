#ifndef SESSIONUTILITIES_H
#define SESSIONUTILITIES_H

enum MessageFlags : char
{
    InitPlayerList = '1',
    ReqInitPlayer = '2',
    ResInitPlayer = '3',
    TargetFieldsReviel = '4',
    BroadcastFieldsReviel = '5'

};

class SessionUtilities
{
public:
    SessionUtilities();
};

#endif // SESSIONUTILITIES_H
