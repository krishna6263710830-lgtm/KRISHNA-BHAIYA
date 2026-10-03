#pragma once

enum class EType : int {
    BOT,
    BULLET
};

enum class EAim : int {
    DISTANCE,
    CROSS_HAIR
};

enum class EAimTarget : int {
    HEAD,
    CHEST,
    BODY
};

enum EAimTrigger {
    NONE,
    SHOOTING,
    SCOPING,
    BOTH,
    ANY
};

namespace config_t
{
    bool low_device{false};
    
    namespace ui
    {
    }
    
    namespace protection {
        bool island{false};
        bool lobby{false};
    }
    
    namespace esp
    {
        namespace visual
        {
        }
        
        namespace vehicle
        {
        }
    }

    namespace aim
    {
    }

    namespace misc
    {

        bool wallhack;
        bool enabled;
        bool rainbow;
        bool colorfull;
        bool glow;
        bool wireframe;       
        float *color; 
        
    }
    
};
