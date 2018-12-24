//
// Created by denis on 12/19/18.
//

#include "SteamController.h"

#include "base/CCEventDispatcher.h"
#include "base/CCDirector.h"

#include "EventSteamController.h"


NS_CC_BEGIN

    const char *SteamController::DEVICE_UNSET = "";

    const std::string &SteamController::getDeviceId() const {
        return deviceId;
    }

    int SteamController::getTag() const {
        return tag;
    }

    void SteamController::onConnected() {
        connectEvent->setIsConnected(true);
        eventDispatcher->dispatchEvent(connectEvent);
    }

    void SteamController::onDisconnected() {
        connectEvent->setIsConnected(false);
        eventDispatcher->dispatchEvent(connectEvent);
        delete this;
    }

    void SteamController::setTag(int tag) {
        SteamController::tag = tag;
    }

	void SteamController::init() {
        eventDispatcher = Director::getInstance()->getEventDispatcher();
        connectEvent = new(std::nothrow) EventSteamController(
                EventSteamController::SteamControllerEventType::CONNECTION, this);
        updateEvent = new(std::nothrow) EventSteamController(EventSteamController::SteamControllerEventType::UPDATE,
                                                             this);
    }

    void SteamController::onUpdated(cocos2d::SteamControllerValues &values) {
        updateEvent->values = values;
        eventDispatcher->dispatchEvent(updateEvent);
    }

NS_CC_END





