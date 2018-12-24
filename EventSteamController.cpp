//
// Created by denis on 12/19/18.
//

#include <cocos/base/CCEventCustom.h>
#include "EventSteamController.h"
#include "EventListenerSteamController.h"


NS_CC_BEGIN
    class EventListenerSteamController;
    cocos2d::EventSteamController::SteamControllerEventType cocos2d::EventSteamController::getEventType() const {
        return eventType;
    }

    bool cocos2d::EventSteamController::isConnected() const {
        return connected;
    }

    void cocos2d::EventSteamController::setIsConnected(bool connected) {
        EventSteamController::connected = connected;
    }

    EventSteamController::EventSteamController(EventSteamController::SteamControllerEventType type,
                                               SteamController *controller):
                                               EventCustom(EventListenerSteamController::LISTENER_ID),
                                               eventType(type),
                                               controller(controller) {

    }

    EventSteamController::~EventSteamController() {

    }

    const SteamControllerValues &EventSteamController::getValues() const {
        return values;
    }

    SteamController *EventSteamController::getController() const {
        return controller;
    }

NS_CC_END
