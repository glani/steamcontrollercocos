//
// Created by denis on 12/19/18.
//

#ifndef CCWARIO_EVENTSTEAMCONTROLLER_H
#define CCWARIO_EVENTSTEAMCONTROLLER_H

#include "platform/CCPlatformMacros.h"
#include "base/CCEventCustom.h"
#include "SteamControllerCommons.h"

NS_CC_BEGIN

class Event;
class SteamController;

class EventSteamController : public EventCustom {
public:

    enum class SteamControllerEventType
    {
        CONNECTION,
        UPDATE
    };

    EventSteamController(SteamControllerEventType type, SteamController* controller);

    virtual ~EventSteamController();

    SteamControllerEventType getEventType() const;

    void setEventType(SteamControllerEventType eventType);

    bool isConnected() const;

    void setIsConnected(bool connected);

    const SteamControllerValues &getValues() const;

    SteamController *getController() const;

protected:
    SteamControllerEventType eventType;
    SteamController* controller;
    SteamControllerValues values;
    bool connected;

    friend class EventListenerSteamController;
    friend class SteamController;
};

NS_CC_END


#endif //CCWARIO_EVENTSTEAMCONTROLLER_H
