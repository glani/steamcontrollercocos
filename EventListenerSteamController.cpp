//
// Created by denis on 12/19/18.
//

#include "EventListenerSteamController.h"

NS_CC_BEGIN
const std::string EventListenerSteamController::LISTENER_ID = "__cc_steamcontroller";

    EventListenerSteamController *EventListenerSteamController::create() {
        auto ret = new (std::nothrow) EventListenerSteamController();
        if (ret && ret->init())
        {
            ret->autorelease();
        }
        else
        {
            CC_SAFE_DELETE(ret);
        }
        return ret;
    }

    bool EventListenerSteamController::checkAvailable() {
        return true;
    }

    EventListenerSteamController *EventListenerSteamController::clone() {
        return nullptr;
    }

    bool EventListenerSteamController::init() {
        auto listener = [this](Event* event) {
            auto evtController = static_cast<EventSteamController *>(event);
            switch (evtController->getEventType()) {
                case EventSteamController::SteamControllerEventType ::CONNECTION:
                    if (evtController->isConnected()) {
                        if (this->onConnected) {
                            this->onConnected(evtController->getController(), event);
                        }
                    } else {
                        if (this->onDisconnected) {
                            this->onDisconnected(evtController->getController(), event);
                        }
                    }
                    break;

                case EventSteamController::SteamControllerEventType ::UPDATE:
                    if (this->onUpdate) {
                        this->onUpdate(evtController->getController(), event);
                    }
                    break;
            }
        };
        if (EventListener::init(EventListener::Type::CUSTOM, LISTENER_ID, listener))
        {
            return true;
        }
        return false;
    }
NS_CC_END