#pragma once
#include "ProtocolFactory.h"
#define REGISTER_PROTOCOL(PROTOCOL_CLASS) \
    namespace { \
        struct PROTOCOL_CLASS##Register { \
            PROTOCOL_CLASS##Register() { \
                ProtocolFactory::registerProtocol(PROTOCOL_CLASS::ID, [](){ \
                    return std::make_unique<PROTOCOL_CLASS>(); \
                }); \
            } \
        }; \
        static PROTOCOL_CLASS##Register global_##PROTOCOL_CLASS##_register; \
    } 