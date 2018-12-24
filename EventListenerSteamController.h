//
// Created by denis on 12/19/18.
//

#ifndef CCWARIO_EVENTLISTENERSTEAMCONTROLLER_H
#define CCWARIO_EVENTLISTENERSTEAMCONTROLLER_H

#include "platform/CCPlatformMacros.h"
#include "base/CCEventListener.h"
#include "EventSteamController.h"

NS_CC_BEGIN

class Event;
class SteamController;

class EventListenerSteamController : public EventListener {
public:
    static const std::string LISTENER_ID;

    /** Create a controller event listener.
     *
     * @return An autoreleased EventListenerController object.
     */
    static EventListenerSteamController* create();

    /// Overrides
    virtual bool checkAvailable() override;

    EventListenerSteamController* clone() override;

    std::function<void(SteamController*, Event*)> onConnected;
    std::function<void(SteamController*, Event*)> onDisconnected;

    std::function<void(SteamController*, Event*)> onUpdate;


protected:
    bool init();

};


NS_CC_END


#endif //CCWARIO_EVENTLISTENERSTEAMCONTROLLER_H
