#pragma once
#include "SM5XX/SM5XX.h"
#include <cstdint>
#include <string>

constexpr uint8_t PART_SETUP = 0x00; // Game A, Time, acl, ...
constexpr uint8_t PART_LEFT = 0x01;
constexpr uint8_t PART_RIGHT = 0x02;

constexpr uint8_t CONF_NOTHING = 0x00;
constexpr uint8_t CONF_1_BUTTON_ACTION = 0x01;
constexpr uint8_t CONF_2_BUTTON_UPDOWN = 0x02;
constexpr uint8_t CONF_2_BUTTON_LEFTRIGHT = 0x03;
constexpr uint8_t CONF_4_BUTTON_DIRECTION = 0x04;

constexpr uint8_t BUTTON_NOTHING = 0x00;
constexpr uint8_t BUTTON_GAMEA = 0x01;
constexpr uint8_t BUTTON_GAMEB = 0x02;
constexpr uint8_t BUTTON_TIME = 0x03;
constexpr uint8_t BUTTON_ALARM = 0x04;
constexpr uint8_t BUTTON_ACL = 0x05;

constexpr uint8_t BUTTON_ACTION = 0x10;
constexpr uint8_t BUTTON_LEFT = 0x20;
constexpr uint8_t BUTTON_RIGHT = 0x30;
constexpr uint8_t BUTTON_UP = 0x40;
constexpr uint8_t BUTTON_DOWN = 0x50;


/*
    Multiplexage explain : 
    


*/



class Virtual_Input {
    public : 
        uint8_t left_configuration = CONF_NOTHING;
        uint8_t right_configuration = CONF_NOTHING;
        bool two_player = false;
        bool use_multiplexage = true;

        Virtual_Input(SM5XX* c) : cpu(c) {}   
        virtual ~Virtual_Input() = default; 
        virtual void set_input(uint8_t, uint8_t, bool, uint8_t = 1){};

    protected : 
        // Important: If multiple physical buttons map to the same K bit ("Down").
        // We must OR them together; otherwise an unpressed button will clear the bit
        // each frame and the game will never see it.
        struct OrKBit {
            bool primary_held = false;
            bool secondary_held = false;

            void set_primary(SM5XX* cpu, int group, int line, bool pressed) {
                primary_held = pressed;
                cpu->input_set(group, line, primary_held || secondary_held);
            }

            void set_secondary(SM5XX* cpu, int group, int line, bool pressed) {
                secondary_held = pressed;
                cpu->input_set(group, line, primary_held || secondary_held);
            }
        };

        SM5XX* cpu;
};


///////////////////////////// Game & Watch Input Configuration //////////////////////////////////////////////////////////

////// BALL : SM5A //////
class AC_01 : public Virtual_Input{
    public : 
        AC_01(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
            //use_multiplexage = false;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(0, 2, state, true); break;
                        case BUTTON_GAMEB: cpu->input_set(0, 1, state, true); break;
                        case BUTTON_TIME: cpu->input_set(0, 0, state, true); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 9, !state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 8, !state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Flagman : SM5A //////
class FL_02 : public Virtual_Input{
    public : 
        FL_02(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(3, 2, state); break;
                        case BUTTON_GAMEB: cpu->input_set(3, 1, state); break;
                        case BUTTON_TIME: cpu->input_set(3, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 0, state); break; // 1
                        case BUTTON_DOWN: cpu->input_set(2, 2, state); break; // 3
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 1, state); break; // 2
                        case BUTTON_DOWN: cpu->input_set(2, 3, state); break; // 4
                        default: break; } break;
                default: break;
            }
        }
};


////// Vermin : SM5A //////
class MT_03 : public Virtual_Input{
    public : 
        MT_03(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
            //use_multiplexage = false;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(0, 2, state, true); break;
                        case BUTTON_GAMEB: cpu->input_set(0, 1, state, true); break;
                        case BUTTON_TIME: cpu->input_set(0, 0, state, true); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 9, !state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 8, !state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Fire (first version) / Kosmicheskiy most : SM5A //////
class RC_04 : public Virtual_Input{
    public : 
        RC_04(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
            //use_multiplexage = false;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(0, 2, state, true); break;
                        case BUTTON_GAMEB: cpu->input_set(0, 1, state, true); break;
                        case BUTTON_TIME: cpu->input_set(0, 0, state, true); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 9, !state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 8, !state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Judge : SM5A //////
class IP_05 : public Virtual_Input{
    public : 
        IP_05(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(3, 2, state); break;
                        case BUTTON_GAMEB: cpu->input_set(3, 1, state); break;
                        case BUTTON_TIME: cpu->input_set(3, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 3, state); break; // 1
                        case BUTTON_DOWN: cpu->input_set(2, 2, state); break; // 3
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 1, state); break; // 2
                        case BUTTON_DOWN: cpu->input_set(2, 0, state); break; // 4
                        default: break; } break;
                default: break;
            }
        }
};


////// Manhole : SM5A //////
class MH_06 : public Virtual_Input{
    public : 
        MH_06(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(3, 2, state); break;
                        case BUTTON_GAMEB: cpu->input_set(3, 1, state); break;
                        case BUTTON_TIME: cpu->input_set(3, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 3, state); break; // 
                        case BUTTON_DOWN: cpu->input_set(2, 2, state); break; // 
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 1, state); break; // 
                        case BUTTON_DOWN: cpu->input_set(2, 0, state); break; // 
                        default: break; } break;
                default: break;
            }
        }
};


////// Helmet : SM5A //////
class CN_07 : public Virtual_Input{
    public : 
        CN_07(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(3, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(3, 1, state); break;                        
                        case BUTTON_GAMEA: cpu->input_set(3, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 9, !state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 8, !state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Lion : SM5A //////
class LN_08 : public Virtual_Input{
    public : 
        LN_08(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(3, 2, state); break;
                        case BUTTON_GAMEB: cpu->input_set(3, 1, state); break;
                        case BUTTON_TIME: cpu->input_set(3, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 3, state); break; // 
                        case BUTTON_DOWN: cpu->input_set(2, 2, state); break; // 
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 1, state); break; // 
                        case BUTTON_DOWN: cpu->input_set(2, 0, state); break; // 
                        default: break; } break;
                default: break;
            }
        }
};


////// Parachute : SM5A //////
class PR_21 : public Virtual_Input{
    public : 
        PR_21(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(3, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(3, 1, state); break;                        
                        case BUTTON_GAMEA: cpu->input_set(3, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 8, !state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 9, !state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// octopus : SM5A //////
class OC_22 : public Virtual_Input{
    public : 
        OC_22(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(3, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(3, 1, state); break;                        
                        case BUTTON_GAMEA: cpu->input_set(3, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 8, !state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 9, !state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Popeye : SM5A //////
class PP_23 : public Virtual_Input{
    public : 
        PP_23(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(3, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(3, 1, state); break;                        
                        case BUTTON_GAMEA: cpu->input_set(3, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 8, !state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 9, !state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Chef : SM5A //////
class FP_24 : public Virtual_Input{
    public : 
        FP_24(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(3, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(3, 1, state); break;                        
                        case BUTTON_GAMEA: cpu->input_set(3, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(2, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(2, 2, state); break;
                        default: break; } break;
                default: break;
            }
        }
};

////// Mickey_Mouse/Egg/Ataka asteroidov/Biathlon/Circus/Hockey/Kosmicheskiy polyot/Kot-rybolov/
////// Kvaka-zadavaka/Morskaja ataka/Nochnye vorishki/Nu, pogodi!/Okhota/Razvedchiki kosmosa/
////// Vesyolye futbolisty : SM5A //////
class MC_25 : public Virtual_Input{
    public : 
        MC_25(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }


        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(3, 2, state); break;
                        case BUTTON_GAMEB: cpu->input_set(3, 1, state); break;
                        case BUTTON_TIME: cpu->input_set(3, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 3, state); break; // 
                        case BUTTON_DOWN: cpu->input_set(2, 2, state); break; // 
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 1, state); break; // 
                        case BUTTON_DOWN: cpu->input_set(2, 0, state); break; // 
                        default: break; } break;
                default: break;
            }
        }
};


////// Fire (wide screen) : SM5A //////
class FR_27 : public Virtual_Input{
    public : 
        FR_27(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(3, 2, state); break;
                        case BUTTON_GAMEB: cpu->input_set(3, 1, state); break;
                        case BUTTON_TIME: cpu->input_set(3, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 8, !state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 9, !state); break;
                        default: break; } break;
                default: break;
            }
        }
};

/////// Space Mission / Spider SM_11/SG_21 : SM5A //////
class SM_11 : public Virtual_Input{
    public : 
        SM_11(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(3, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(3, 1, state); break;                        
                        case BUTTON_GAMEA: cpu->input_set(3, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 9, !state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 8, !state); break;
                        default: break; } break;
                default: break;
            }
        }
}; 

////// Super Goal Keeper SK_10 : SM5A //////
class SK_10 : public Virtual_Input{
    public : 
        SK_10(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(3, 2, state); break;
                        case BUTTON_GAMEB: cpu->input_set(3, 1, state); break;
                        case BUTTON_TIME: cpu->input_set(3, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 3, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 2, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 1, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};

////// Autoslalom IM_23 : SM5A //////
class IM_23 : public Virtual_Input{
    public : 
        IM_23(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(3, 2, state); break;
                        case BUTTON_GAMEB: cpu->input_set(3, 1, state); break;
                        case BUTTON_TIME: cpu->input_set(3, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 1, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 2, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 3, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};

////// Turtle Bridge : SM10 //////
class TL_28 : public Virtual_Input{
    public : 
        TL_28(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};

////// Fire Attack : SM10 //////
class ID_29 : public Virtual_Input{
    public : 
        ID_29(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 1, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};

////// Snoopy Tennis : SM10 //////
class SP_30 : public Virtual_Input{
    public : 
        SP_30(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 1, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Oil Panic : SM10 //////
class OP_51 : public Virtual_Input{
    public : 
        OP_51(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Donkey Kong : SM10 //////
class DK_52 : public Virtual_Input{
    public : 
        DK_52(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(2, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(2, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(2, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(1, 0, state); break;
                        case BUTTON_UP: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(1, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Mickey & Donald : SM10 //////
class DM_53 : public Virtual_Input{
    public : 
        DM_53(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_2_BUTTON_LEFTRIGHT;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 1, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(0, 0, state); break;
                        case BUTTON_LEFT: cpu->input_set(0, 2, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Green House : SM10 //////
class GH_54 : public Virtual_Input{
    public : 
        GH_54(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(2, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(2, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(2, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(1, 0, state); break;
                        case BUTTON_UP: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(1, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Donkey Kong  : SM10 //////
class JR_55 : public Virtual_Input{
    public : 
        JR_55(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(2, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(2, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(2, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(1, 0, state); break;
                        case BUTTON_UP: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(1, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Mario Bros  : SM10 //////
class MW_56 : public Virtual_Input{
    public : 
        MW_56(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 1, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Rain Shower  : SM10 //////
class LP_57 : public Virtual_Input{
    public : 
        LP_57(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(2, 0, state); break;
                        case BUTTON_UP: cpu->input_set(2, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(2, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 2, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Life Boat  : SM10 //////
class TC_58 : public Virtual_Input{
    public : 
        TC_58(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 1, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Pinball  : SM11 //////
class PB_59 : public Virtual_Input{
    public : 
        PB_59(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 2, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 1, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Black Jack  : SM12 //////
class BJ_60 : public Virtual_Input{
    public : 
        BJ_60(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 0, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 1, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 3, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 2, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Squish  : SM10 //////
class MG_61 : public Virtual_Input{
    public : 
        MG_61(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_2_BUTTON_LEFTRIGHT;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEB: cpu->input_set(1, 2, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 1, state); break;
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_LEFT: cpu->input_set(0, 3, state); break;
                        case BUTTON_RIGHT: cpu->input_set(0, 1, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Bomb Sweeper  : SM10 //////
class BD_62 : public Virtual_Input{
    public : 
        BD_62(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_NOTHING;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_LEFT: cpu->input_set(0, 0, state); break;
                        case BUTTON_UP: cpu->input_set(0, 1, state); break;
                        case BUTTON_RIGHT: cpu->input_set(0, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Safe Buster  : SM11 //////
class JB_63 : public Virtual_Input{
    public : 
        JB_63(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 1, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Gold Cliff : SM12 //////
class MV_64 : public Virtual_Input{
    public : 
        MV_64(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(2, 2, state); break;
                        case BUTTON_GAMEB: cpu->input_set(2, 1, state); break;
                        case BUTTON_TIME: cpu->input_set(2, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_LEFT: cpu->input_set(0, 0, state); break;
                        case BUTTON_UP: cpu->input_set(0, 1, state); break;
                        case BUTTON_RIGHT: cpu->input_set(0, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(1, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Zelda (double screen) : SM12 //////
class ZL_65 : public Virtual_Input{
    public : 
        ZL_65(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(2, 2, state); break;
                        case BUTTON_GAMEB: cpu->input_set(2, 1, state); break;
                        case BUTTON_TIME: cpu->input_set(2, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_LEFT: cpu->input_set(0, 0, state); break;
                        case BUTTON_UP: cpu->input_set(0, 1, state); break;
                        case BUTTON_RIGHT: cpu->input_set(0, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(1, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// DonkeyKong Jr (Wide Screen) : SM10 //////
class DJ_101 : public Virtual_Input{
    public : 
        DJ_101(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(2, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(2, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(2, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(1, 0, state); break;
                        case BUTTON_UP: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(1, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Manhole (Wide Screen) : SM10 //////
class NH_103 : public Virtual_Input{
    public : 
        NH_103(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break; // 
                        case BUTTON_DOWN: cpu->input_set(0, 3, state); break; // 
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 1, state); break; // 
                        case BUTTON_DOWN: cpu->input_set(0, 0, state); break; // 
                        default: break; } break;
                default: break;
            }
        }
};



////// Donkey Kong Jr (panorama) : SM11 //////
class CJ_93 : public Virtual_Input{
    public : 
        CJ_93(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(2, 2, state); break;
                        case BUTTON_GAMEB: cpu->input_set(2, 1, state); break;
                        case BUTTON_TIME: cpu->input_set(2, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(1, 0, state); break;
                        case BUTTON_UP: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(1, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Super Mario Bros : SM11 //////
class YM_801 : public Virtual_Input{
    public : 
        YM_801(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(0, 1, state); break;
                        case BUTTON_TIME: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(1, 0, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_DOWN: cpu->input_set(1, 2, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(2, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Ice Climber : SM11 //////
class DR_802 : public Virtual_Input{
    public : 
        DR_802(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(0, 0, state); break;
                        case BUTTON_GAMEA: cpu->input_set(0, 1, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(1, 0, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_DOWN: cpu->input_set(1, 2, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(3, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Balloon Fight : SM11 //////
class BF_803 : public Virtual_Input{
    public : 
        BF_803(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(0, 1, state); break;
                        case BUTTON_TIME: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(1, 0, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_DOWN: cpu->input_set(1, 2, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(2, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Mario's Cement Factory (Table Top) : SM11 //////
class CM_72 : public Virtual_Input{
    public : 
        CM_72(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_LEFTRIGHT;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(0, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(0, 2, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Snoopy (Table Top) : SM11 //////
class SM_91 : public Virtual_Input{
    public : 
        SM_91(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_LEFTRIGHT;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(0, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(0, 2, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Mario's Bombs Away (Table Top) : SM11 //////
class TB_94 : public Virtual_Input{
    public : 
        TB_94(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_LEFTRIGHT;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(0, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(0, 2, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};



////// Popeye (Table Top) : SM11 //////
class PG_92 : public Virtual_Input{
    public : 
        PG_92(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_LEFTRIGHT;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(0, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(0, 2, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};

////// Donkey Kong Circus / Mickey Mouse (panorama)  : SM11 //////
class DC_95 : public Virtual_Input{
    public : 
        DC_95(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 2, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 1, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Mario's Cement Factory (Widescreen) : SM10 //////
class ML_102 : public Virtual_Input{
    public : 
        ML_102(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_LEFTRIGHT;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(0, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(0, 2, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Tropical Fish : SM10 //////
class TF_104 : public Virtual_Input{
    public : 
        TF_104(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 1, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Mario The Juggler  : SM11 //////
class MB_108 : public Virtual_Input{
    public : 
        MB_108(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Spitball Sparky : SM11 //////
class BU_201 : public Virtual_Input{
    public : 
        BU_201(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_LEFTRIGHT;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(0, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 2, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Crab Grab : SM11 //////
class UD_202 : public Virtual_Input{
    public : 
        UD_202(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_LEFTRIGHT;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(0, 0, state); break;
                        case BUTTON_LEFT: cpu->input_set(0, 2, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 1, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                default: break;
            }
        }
};

////// Shuttle Voyage : SM11 //////
class MG_8 : public Virtual_Input{
    public : 
        MG_8(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(7, 0, state); break; // Mode
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(6, 0, state); break; // 
                        case BUTTON_DOWN: cpu->input_set(6, 1, state); break; // 
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(6, 3, state); break;
                        default: break; } break;
                default: break;
            }
        }
};

////// Diver's Adventure : SM11 //////
class DA_37 : public Virtual_Input{
    public : 
        DA_37(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 0, state); break; 
                        case BUTTON_DOWN: cpu->input_set(0, 1, state); break; 
                        default: break; } break;
                default: break;
            }
        }
};


////// Clever Chicken (Tronica) : SM11 //////
class CC_38V : public Virtual_Input{
    public : 
        CC_38V(SM5XX* c) : Virtual_Input(c) {
            // Based on MAME hh_sm510.cpp (trclchick): IN.0 uses Right/Left/Down (no Up).
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_4_BUTTON_DIRECTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: down.set_primary(cpu, 0, 3, state); break;  // D-pad Up -> DOWN input
                        case BUTTON_DOWN: cpu->input_set(0, 1, state); break;        // LEFT input
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: down.set_secondary(cpu, 0, 3, state); break; // X -> DOWN input
                        case BUTTON_DOWN: cpu->input_set(0, 0, state); break;      // RIGHT input
                        default: break; } break;
                default: break;
            }
        }

    private:
        OrKBit down;
};

////// Space Rescue / Thunder Ball MG_9/FR_23 : SM11 //////
class MG_9 : public Virtual_Input{
    public : 
        MG_9(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 3, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;                        
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                default: break;            }
        }
};


////// punch_out : SM11 //////
class BX_301 : public Virtual_Input{
    public : 
        BX_301(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (player) {
                case 1:
                    switch (part) {
                        case PART_SETUP:
                            switch (button) {
                                case BUTTON_TIME: cpu->input_set(6, 0, state); break;
                                case BUTTON_GAMEB: cpu->input_set(6, 1, state); break;
                                case BUTTON_GAMEA: cpu->input_set(6, 2, state); break;
                                default: break; } break;
                        case PART_LEFT:
                            switch (button) {
                                case BUTTON_RIGHT: cpu->input_set(5, 0, state); break;
                                case BUTTON_LEFT: cpu->input_set(5, 1, state); break;
                                case BUTTON_UP: cpu->input_set(3, 1, state); break;
                                case BUTTON_DOWN: cpu->input_set(3, 0, state); break;
                                default: break; } break;
                        case PART_RIGHT:
                            switch (button) {
                                case BUTTON_ACTION: cpu->input_set(1, 0, state); break;
                                default: break; } break;
                        default: break;
                    }
                    break;
                case 2:
                    switch (part) {
                        case PART_LEFT:
                            switch (button) {
                                case BUTTON_RIGHT: cpu->input_set(4, 2, state); break;
                                case BUTTON_LEFT: cpu->input_set(4, 3, state); break;
                                case BUTTON_UP: cpu->input_set(2, 3, state); break;
                                case BUTTON_DOWN: cpu->input_set(2, 2, state); break;
                                default: break; } break;
                        case PART_RIGHT:
                            switch (button) {
                                case BUTTON_ACTION: cpu->input_set(0, 2, state); break;
                                default: break; } break;
                        default: break;
                    }
                    break;
            }
        }
};

/////// Space Adventure SA_12 : SM11 //////
class SA_12 : public Virtual_Input{
    public : 
        SA_12(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(7, 0, state); break; // Mode
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(5, 2, state); break; // JOYSTICK_RIGHT
                        case BUTTON_DOWN: cpu->input_set(5, 0, state); break; // Left/Sound
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(5, 1, state); break; // Fire
                        default: break; } break;
                default: break;
            }
        }
};




//////////////////// David and John (Open source Game) ////////////////////

class d_and_j : public Virtual_Input{
    public : 
        d_and_j(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_LEFTRIGHT;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_LEFT: cpu->input_set(1, 0, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        default: break; } break;
                default: break;
            }
        }
};

class d_and_j_fabric : public Virtual_Input{
    public : 
        d_and_j_fabric(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 0, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                default: break;
            }
        }
};

class d_and_j_esquive : public Virtual_Input{
    public : 
        d_and_j_esquive(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_2_BUTTON_LEFTRIGHT;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_LEFT: cpu->input_set(1, 0, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 0, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                default: break;
            }
        }
};




//////////////////// Tiger ////////////////////

class TSONIC2 : public Virtual_Input{
    public : 
        TSONIC2(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Start
                        case BUTTON_GAMEB: cpu->input_set(5, 0, state); break; // Pause
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(4, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(1, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(2, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(2, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(3, 1, state); break;
                        default: break; } break;
                default: break;
            }
        }
};

class TSFIGHT2 : public Virtual_Input{
    public : 
        TSFIGHT2(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_4_BUTTON_DIRECTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Start
                        case BUTTON_GAMEB: cpu->input_set(5, 0, state); break; // Game
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(4, 0, state); break; // Punch/Special Move
                        case BUTTON_RIGHT: cpu->input_set(3, 1, state); break; // Kick Right
                        case BUTTON_LEFT: cpu->input_set(4, 1, state); break; // Kick Left
                        case BUTTON_DOWN: cpu->input_set(4, 0, state); break; // Punch/Special Move
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                default: break;
            }
        }
};

class TDDRAGON : public Virtual_Input{
    public : 
        TDDRAGON(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(5, 3, state, true); break; // Start
                        case BUTTON_GAMEB: cpu->input_set(4, 0, state); break; // Status / Pause
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(3, 0, state); break; // Punch - Pick / Action on UP
                        case BUTTON_DOWN: cpu->input_set(3, 1, state); break; // kick / Action on Down
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                default: break;
            }
        }
};




class TSIMQUEST : public Virtual_Input{
    public : 
        TSIMQUEST(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_4_BUTTON_DIRECTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Start
                        case BUTTON_GAMEB: cpu->input_set(5, 0, state); break; // Max Score
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(4, 0, state); break; // Jump Up
                        case BUTTON_DOWN: cpu->input_set(3, 1, state); break; // Sword
                        case BUTTON_LEFT: cpu->input_set(3, 1, state); break; // Sword 
                        case BUTTON_RIGHT: cpu->input_set(4, 1, state); break; // Jump Forward
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                default: break;
            }
        }
};



class TDDRAGON3 : public Virtual_Input{
    public : 
        TDDRAGON3(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_4_BUTTON_DIRECTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Start
                        case BUTTON_GAMEB: cpu->input_set(5, 0, state); break; // Max Score
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(4, 0, state); break; // Throw
                        case BUTTON_RIGHT: cpu->input_set(3, 1, state); break; // Punch
                        case BUTTON_LEFT: cpu->input_set(4, 1, state); break; // Kick 
                        case BUTTON_DOWN: cpu->input_set(4, 1, state); break; // Kick
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


class TSDDRAGON : public Virtual_Input{
    public : 
        TSDDRAGON(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_4_BUTTON_DIRECTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Start
                        case BUTTON_GAMEB: cpu->input_set(5, 0, state); break; // Max Score
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(3, 1, state); break; // Special Technique
                        case BUTTON_RIGHT: cpu->input_set(4, 0, state); break; // Kick right
                        case BUTTON_LEFT: cpu->input_set(0, 0, state); break; // Kick left
                        case BUTTON_DOWN: cpu->input_set(4, 1, state); break; // Punch
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


class TGOLDNAXE : public Virtual_Input{
    public : 
        TGOLDNAXE(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_4_BUTTON_DIRECTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Start
                        case BUTTON_GAMEB: cpu->input_set(5, 0, state); break; // Max Score
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(3, 1, state); break; // Magic
                        case BUTTON_RIGHT: cpu->input_set(4, 1, state); break; // Attack Right
                        case BUTTON_LEFT: cpu->input_set(4, 0, state); break; // Attack Left
                        case BUTTON_DOWN: cpu->input_set(3, 1, state); break; // Magic
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


class TGAUNT : public Virtual_Input{
    public : 
        TGAUNT(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_4_BUTTON_DIRECTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Start
                        case BUTTON_GAMEB: cpu->input_set(5, 0, state); break; // Max Score
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(4, 1, state); break; // key
                        case BUTTON_LEFT: cpu->input_set(3, 1, state); break; // BOMB
                        case BUTTON_RIGHT: cpu->input_set(4, 0, state); break; // Attack 
                        case BUTTON_DOWN: cpu->input_set(0, 0, state); break; // Pick
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


class TMCHAMMER : public Virtual_Input{
    public : 
        TMCHAMMER(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_4_BUTTON_DIRECTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Start
                        case BUTTON_GAMEB: cpu->input_set(5, 2, state); break; // Max Score
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(1, 1, state); break; // Leg Leaps Up/Jump
                        case BUTTON_LEFT: cpu->input_set(0, 0, state); break; // Leg Footwork Left
                        case BUTTON_RIGHT: cpu->input_set(1, 0, state); break; // Leg Footwork Right
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break; // Leg Leaps Down
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break; // ARM UP
                        case BUTTON_RIGHT: cpu->input_set(2, 2, state); break; // ARM Right
                        case BUTTON_LEFT: cpu->input_set(3, 2, state); break; // ARM Left
                        case BUTTON_DOWN: cpu->input_set(4, 2, state); break; // ARM Down
                        default: break; } break;
                default: break;
            }
        }
};


class TSHARR2 : public Virtual_Input{
    public : 
        TSHARR2(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(5, 3, state, true); break; // Start
                        case BUTTON_GAMEB: cpu->input_set(4, 0, state); break; // Max Score
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(3, 0, state); break; // key
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////////// Experimental games (MANUFACTURER_EXPERIMENTAL) //////////
// Button assignments derived directly from MAME's hh_sm510.cpp INPUT_PORTS
// definitions for each romset (IN.N port index -> group, bit position ->
// line - e.g. bit 0x04 is line 2 - confirmed against TSFIGHT2/TMCHAMMER
// above, whose existing group/line values match their MAME ports exactly
// under this same translation). PART_LEFT/PART_RIGHT button choices were
// then picked to match each console's actual physical button layout, using
// the MAME-rendered console screenshots as reference (see
// CONVERT_ROM/GNW_LIST.md's "Tiger (Experimental)" section).

////// Batman (Tiger) : SM510 //////
// Physical: left = 4-way d-pad (only Up/Right/Down wired - no Left on this
// particular romset); right = 2 stacked buttons, "Pick" above "Attack".
class TBATMAN : public Virtual_Input{
    public :
        TBATMAN(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(5, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(4, 0, state); break; // Max Score
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(3, 0, state); break; // Pick
                        case BUTTON_DOWN: cpu->input_set(3, 1, state); break; // Attack
                        default: break; } break;
                default: break;
            }
        }
};

////// Spider-Man (Tiger, 1991) : SM510 //////
// Physical: left = full 4-way d-pad; right = "PUNCH" button above a
// "RESCUE"/"KICK" rocker (Left=Rescue, Right=Kick).
class TSPIDMAN : public Virtual_Input{
    public :
        TSPIDMAN(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_4_BUTTON_DIRECTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(5, 0, state); break; // Status
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(4, 0, state); break; // Shoot Up/Punch
                        case BUTTON_RIGHT: cpu->input_set(3, 1, state); break; // Shoot Right/Kick
                        case BUTTON_LEFT: cpu->input_set(4, 1, state); break; // Shoot Left/Rescue
                        default: break; } break;
                default: break;
            }
        }
};

////// X-Men (Tiger) : SM510 //////
// Physical: left = full 4-way d-pad; right = "ATTACK/PICK" rocker (Left=Pick,
// Right=Attack) above a "PUNCH/CLAWS" button.
class TXMEN : public Virtual_Input{
    public :
        TXMEN(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_4_BUTTON_DIRECTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(5, 0, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(3, 1, state); break; // Attack/Pick Right
                        case BUTTON_LEFT: cpu->input_set(4, 1, state); break; // Attack/Pick Left
                        case BUTTON_DOWN: cpu->input_set(4, 0, state); break; // Punch/Claws
                        default: break; } break;
                default: break;
            }
        }
};

////// Mortal Kombat (Tiger) : SM510 //////
// Physical: left = full 4-way d-pad; right = two 2-way rockers stacked,
// "H PUNCH L" above "H KICK L". Mapped to the 4-way pad as
// Up=Punch High, Right=Kick High, Down=Punch Low, Left=Kick Low.
class TMKOMBAT : public Virtual_Input{
    public :
        TMKOMBAT(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_4_BUTTON_DIRECTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(5, 0, state); break; // Select
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 0, state); break; // Punch High
                        case BUTTON_RIGHT: cpu->input_set(3, 1, state); break; // Kick High
                        case BUTTON_DOWN: cpu->input_set(4, 0, state); break; // Punch Low
                        case BUTTON_LEFT: cpu->input_set(4, 1, state); break; // Kick Low
                        default: break; } break;
                default: break;
            }
        }
};

////// Superman (Tiger) : SM511 //////
// Physical: left = left/right-only rocker (no up/down on this romset);
// right = 2 stacked buttons, "Punch" above "Call".
class TSUPERMAN : public Virtual_Input{
    public :
        TSUPERMAN(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_LEFTRIGHT;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        // NOTE: this chip variant (shared with Gargoyles: Night Flight,
        // tgargnf in MAME) doesn't use the first scan line - MAME's own
        // port comments read S2, S3, S4... instead of the usual S1, S2,
        // S3..., unlike every other class in this file. The `group` used
        // here is the actual physical S-line minus 1 (S2 -> group 1, S3 ->
        // group 2, ...), NOT the port's position in the list (which would
        // be off by one for this game specifically) - confirmed against
        // TSONIC2 above, whose MAME source has this exact same S2-start
        // quirk and already encodes the same +1 offset. `group` is ignored
        // entirely for the not_multiplex=true Start call (see
        // SM5XX::input_set), so that one was unaffected by this bug.
        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(6, 0, state); break; // Max Score
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(2, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(2, 2, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(4, 1, state); break; // Punch
                        case BUTTON_DOWN: cpu->input_set(5, 0, state); break; // Call
                        default: break; } break;
                default: break;
            }
        }
};

////// Battletoads (Tiger) : SM510 //////
// Physical: left = full 4-way d-pad; right = "ATTACK" rocker (Left/Right)
// above "SELECT" (single button).
class TBTOADS : public Virtual_Input{
    public :
        TBTOADS(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_4_BUTTON_DIRECTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(5, 0, state); break; // Score
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(3, 1, state); break; // Attack Right
                        case BUTTON_LEFT: cpu->input_set(4, 1, state); break; // Attack Left
                        case BUTTON_DOWN: cpu->input_set(4, 0, state); break; // Select
                        default: break; } break;
                default: break;
            }
        }
};

////// Karnov (Tiger) : SM510 //////
// NOTE: this romset uses a different bit layout than most others in this
// file (Up/Right on bit 0 instead of the usual bit 2/bit 1, Left on bit 1,
// Max Score on bit 2 instead of bit 0) - verified directly against its own
// MAME INPUT_PORTS rather than assumed from another game.
// Physical: left = full 4-way d-pad; right = "FIRE-BALL"/"BOOMERANG"
// rocker above "SHIELD" (single button).
class TKARNOV : public Virtual_Input{
    public :
        TKARNOV(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_4_BUTTON_DIRECTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(5, 2, state); break; // Max Score
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 0, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 0, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 1, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_LEFT: cpu->input_set(3, 1, state); break; // Fire-Ball
                        case BUTTON_RIGHT: cpu->input_set(4, 1, state); break; // Boomerang
                        case BUTTON_DOWN: cpu->input_set(4, 2, state); break; // Shield
                        default: break; } break;
                default: break;
            }
        }
};

////// Hook (Tiger) : SM510 //////
// Physical: left = full 4-way d-pad ("Sword Up"/"Sword Down/Dodge"); right
// = "SWING" rocker (Left/Right) above "ATTACK/PICK" (single button).
class THOOK : public Virtual_Input{
    public :
        THOOK(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_4_BUTTON_DIRECTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(5, 0, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break; // Sword Up
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break; // Sword Down/Dodge
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(3, 1, state); break; // Swing Right
                        case BUTTON_LEFT: cpu->input_set(4, 1, state); break; // Swing Left
                        case BUTTON_DOWN: cpu->input_set(4, 0, state); break; // Attack/Pick
                        default: break; } break;
                default: break;
            }
        }
};

////// Back to the Future (Tiger) : SM510 //////
// Physical: left = full 4-way d-pad ("Repair"/"Brake"); right = 2 stacked
// buttons, "Accelerate" above "Rescue". Only 6 IN.x ports total on this
// romset (Start is on IN.5, not IN.6 like most others).
class TBTTF : public Virtual_Input{
    public :
        TBTTF(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(5, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(4, 0, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break; // Repair
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break; // Brake
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(3, 0, state); break; // Accelerate
                        case BUTTON_DOWN: cpu->input_set(3, 1, state); break; // Rescue
                        default: break; } break;
                default: break;
            }
        }
};

////// Transformers: Generation 2 (Tiger) : SM510 //////
// Physical: left = full 4-way d-pad; right = "CALL" (single button) above
// "TRANSFORM"/"ATTACK" rocker (Left/Right).
class TTRANSF2 : public Virtual_Input{
    public :
        TTRANSF2(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_4_BUTTON_DIRECTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(5, 0, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(4, 0, state); break; // Call
                        case BUTTON_LEFT: cpu->input_set(4, 1, state); break; // Transform
                        case BUTTON_RIGHT: cpu->input_set(3, 0, state); break; // Attack
                        default: break; } break;
                default: break;
            }
        }
};

////// Vindicators (Tiger) : SM510 //////
// Physical: left = 4-way-shaped d-pad (only Up/Right/Left wired - no Down
// on this romset); right = "FIRE" (single button) above "GUN" rocker
// (Left/Right).
class TVINDICTR : public Virtual_Input{
    public :
        TVINDICTR(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_4_BUTTON_DIRECTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(5, 0, state); break; // Max Score
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(4, 0, state); break; // Fire
                        case BUTTON_LEFT: cpu->input_set(4, 1, state); break; // Gun Left
                        case BUTTON_RIGHT: cpu->input_set(3, 1, state); break; // Gun Right
                        default: break; } break;
                default: break;
            }
        }
};

////// Jurassic Park (Tiger) : SM510 //////
// Physical: left = full 4-way d-pad ("Jump/Climb"/"Forward"); right = 2
// stacked buttons, "Swing/Power" above "Call". Note "Swing/Power" shares
// IN.0 with the d-pad's Up (same group, different line) rather than living
// on its own port like most other games' right-side buttons.
class TJPARK : public Virtual_Input{
    public :
        TJPARK(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(5, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(4, 0, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 0, state); break; // Jump/Climb
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break; // Forward
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break; // Swing/Power
                        case BUTTON_DOWN: cpu->input_set(3, 1, state); break; // Call
                        default: break; } break;
                default: break;
            }
        }
};

////// Skeleton Warriors: The Dark Crusade (Tiger) : SM510 //////
// Physical: left = full 4-way d-pad; right = "ATTACK" rocker (Left/Right)
// above "PICK/CALL" (single button).
class TSKELWARR : public Virtual_Input{
    public :
        TSKELWARR(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_4_BUTTON_DIRECTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(5, 0, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(3, 1, state); break; // Attack Right
                        case BUTTON_LEFT: cpu->input_set(4, 1, state); break; // Attack Left
                        case BUTTON_DOWN: cpu->input_set(4, 0, state); break; // Pick/Call
                        default: break; } break;
                default: break;
            }
        }
};

////// The Addams Family (Tiger) : SM510 //////
// Physical: left = full 4-way d-pad; right = "TAKE" (single button) above
// "THROW"/"SELECT" rocker (Left=Select, Right=Throw).
class TADDAMS : public Virtual_Input{
    public :
        TADDAMS(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_4_BUTTON_DIRECTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(5, 0, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(3, 1, state); break; // Take
                        case BUTTON_UP: cpu->input_set(4, 0, state); break; // Throw
                        case BUTTON_LEFT: cpu->input_set(4, 1, state); break; // Select
                        default: break; } break;
                default: break;
            }
        }
};

////// Home Alone (Tiger) : SM510 //////
// Physical: left = full 4-way d-pad; right = "TRAP" rocker (Left/Right)
// above "CLIMB" (single button).
class THALONE : public Virtual_Input{
    public :
        THALONE(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_4_BUTTON_DIRECTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(5, 0, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(3, 1, state); break; // Trap Right
                        case BUTTON_UP: cpu->input_set(4, 0, state); break; // Climb
                        case BUTTON_LEFT: cpu->input_set(4, 1, state); break; // Trap Left
                        default: break; } break;
                default: break;
            }
        }
};

////// Home Alone 2: Lost in New York (Tiger) : SM510 //////
// Physical: same shape as Home Alone above - left = full 4-way d-pad;
// right = "TRAP/FLASH" rocker (Left/Right) above "CLIMB/PICK" (single
// button).
class THALONE2 : public Virtual_Input{
    public :
        THALONE2(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_4_BUTTON_DIRECTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(5, 0, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(3, 1, state); break; // Trap/Flash Right
                        case BUTTON_UP: cpu->input_set(4, 0, state); break; // Climb/Pick
                        case BUTTON_LEFT: cpu->input_set(4, 1, state); break; // Trap/Flash Left
                        default: break; } break;
                default: break;
            }
        }
};

////// X-Men: Project X (Tiger) : SM510 //////
// Physical: left = full 4-way d-pad ("Pick" on Down); right = "JUMP"
// (single button) above "MOVE" (single button) - only 6 IN.x ports total
// (Start is on IN.5, not IN.6; Pause/GameB on IN.4, not IN.5).
class TXMENPX : public Virtual_Input{
    public :
        TXMENPX(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(5, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(4, 0, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break; // Pick
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(3, 0, state); break; // Jump
                        case BUTTON_DOWN: cpu->input_set(3, 1, state); break; // Move
                        default: break; } break;
                default: break;
            }
        }
};

////// Wayne's World (Tiger) : SM510 //////
// Physical: left = full 4-way d-pad (Up labeled "High Five" - still just
// the Up direction); right = "HOCKEY" rocker (Left/Right) above
// "SHOOT/CASSANDRA" (single button).
class TWWORLD : public Virtual_Input{
    public :
        TWWORLD(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_4_BUTTON_DIRECTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(5, 0, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break; // High Five
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(3, 1, state); break; // Hockey Right
                        case BUTTON_UP: cpu->input_set(4, 0, state); break; // Shoot/Cassandra
                        case BUTTON_LEFT: cpu->input_set(4, 1, state); break; // Hockey Left
                        default: break; } break;
                default: break;
            }
        }
};

////// The Nightmare Before Christmas (Tiger) : SM510 //////
// Physical: left = full 4-way d-pad ("JUMP" on Up); right = single big
// oval button literally labeled "ACTION" on the console - only 5 IN.x
// ports total (Pause/GameB on IN.4, Start on IN.5).
class TNMAREBC : public Virtual_Input{
    public :
        TNMAREBC(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(5, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(4, 0, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break; // Jump
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(3, 0, state); break; // Action
                        default: break; } break;
                default: break;
            }
        }
};

////// Operation: Aliens (Tiger) : SM510 //////
// Physical: left = full 4-way d-pad; right = "ATTACK" (single button)
// above "PICK" (single button) - only 5 IN.x ports total.
class TOPALIENS : public Virtual_Input{
    public :
        TOPALIENS(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(5, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(4, 0, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(3, 0, state); break; // Attack
                        case BUTTON_DOWN: cpu->input_set(3, 1, state); break; // Pick
                        default: break; } break;
                default: break;
            }
        }
};

////// The Shadow (Tiger) : SM510 //////
// Physical: left = full 4-way d-pad; right = "SHADOW" (single button)
// above "KICK"/"PUNCH" rocker (Left=Kick, Right=Punch).
class TSHADOW : public Virtual_Input{
    public :
        TSHADOW(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_4_BUTTON_DIRECTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(5, 0, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(3, 1, state); break; // Punch
                        case BUTTON_UP: cpu->input_set(4, 0, state); break; // Shadow
                        case BUTTON_LEFT: cpu->input_set(4, 1, state); break; // Kick
                        default: break; } break;
                default: break;
            }
        }
};

////// Space Jam (Tiger) : SM510 //////
// Physical: left = full 4-way d-pad; right = "SHOOT/BLOCK" (single button)
// above "TUNE/STEAL" (single button) - only 6 IN.x ports total.
class TSJAM : public Virtual_Input{
    public :
        TSJAM(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(5, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(4, 0, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(3, 0, state); break; // Shoot/Block
                        case BUTTON_DOWN: cpu->input_set(3, 1, state); break; // Tune/Steal
                        default: break; } break;
                default: break;
            }
        }
};

////// Street Fighter 2010: The Final Fight (Tiger) : SM510 //////
// Physical: left = full 4-way d-pad; right = "PUNCH" rocker (Left/Right)
// above "KICK" (single button).
class TSF2010 : public Virtual_Input{
    public :
        TSF2010(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_4_BUTTON_DIRECTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(5, 0, state); break; // Select Planet
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(3, 1, state); break; // Punch Right
                        case BUTTON_UP: cpu->input_set(4, 0, state); break; // Kick
                        case BUTTON_LEFT: cpu->input_set(4, 1, state); break; // Punch Left
                        default: break; } break;
                default: break;
            }
        }
};

////// Swamp Thing (Tiger) : SM510 //////
// Physical: left = full 4-way d-pad; right = "PUNCH/FIRE" (single button)
// above "THROW" (single button) - only 6 IN.x ports total.
class TSWAMPT : public Virtual_Input{
    public :
        TSWAMPT(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(5, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(4, 0, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(3, 0, state); break; // Punch/Fire
                        case BUTTON_DOWN: cpu->input_set(3, 1, state); break; // Throw
                        default: break; } break;
                default: break;
            }
        }
};

////// The Incredible Crash Dummies (Tiger) : SM510 //////
// Physical: left = full 4-way d-pad ("BRAKE"/"CRASH" on Left/Right); right
// = "ACCELERATE" (single button) above "PICK" (single button) - only 6
// IN.x ports total.
class TDUMMIES : public Virtual_Input{
    public :
        TDUMMIES(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(5, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(4, 0, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break; // Crash
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break; // Brake
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(3, 0, state); break; // Accelerate
                        case BUTTON_DOWN: cpu->input_set(3, 1, state); break; // Pick
                        default: break; } break;
                default: break;
            }
        }
};

////// Robocop 2 (Tiger) : SM510 //////
// Physical: left = full 4-way d-pad ("RESCUE" on Left, "PICK" on Down);
// right = "SHOOT" (single button) above "SHOOT" rocker (Left/Right).
class TROBOCOP2 : public Virtual_Input{
    public :
        TROBOCOP2(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_4_BUTTON_DIRECTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(5, 0, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break; // Rescue
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break; // Pick
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(3, 1, state); break; // Shoot Right
                        case BUTTON_UP: cpu->input_set(4, 0, state); break; // Shoot Up
                        case BUTTON_LEFT: cpu->input_set(4, 1, state); break; // Shoot Left
                        default: break; } break;
                default: break;
            }
        }
};

////// Robocop 3 (Tiger) : SM510 //////
// Same shape as Robocop 2 above. Physical: left = full 4-way d-pad
// ("JUMP/FLY" on Up, "RESCUE" on Left, "PICK" on Down); right = "SHOOT"
// (single button) above "SHOOT" rocker (Left/Right).
class TROBOCOP3 : public Virtual_Input{
    public :
        TROBOCOP3(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_4_BUTTON_DIRECTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(5, 0, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break; // Jump/Fly
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break; // Rescue
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break; // Pick
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(3, 1, state); break; // Shoot Right
                        case BUTTON_UP: cpu->input_set(4, 0, state); break; // Shoot Up
                        case BUTTON_LEFT: cpu->input_set(4, 1, state); break; // Shoot Left
                        default: break; } break;
                default: break;
            }
        }
};

////// Independence Day (Tiger) : SM510 //////
// NOTE: this romset's bit layout for the left d-pad's Up is on line 0, not
// the usual line 2 (confirmed directly against its own MAME INPUT_PORTS -
// every other button here follows the normal convention). Physical: left =
// full 4-way d-pad ("SHIELD" on Up, "ALERT" on Down); right = "FIRE"
// (single button) above "VELOCITY" (single button) - only 6 IN.x ports.
class TINDAY : public Virtual_Input{
    public :
        TINDAY(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(5, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(4, 0, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 0, state); break; // Shield
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break; // Alert
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break; // Fire
                        case BUTTON_DOWN: cpu->input_set(3, 1, state); break; // Velocity
                        default: break; } break;
                default: break;
            }
        }
};

////// The Flash (Tiger) : SM510 //////
// NOTE: unlike every other game in this file, Left and Right aren't on the
// same IN port here - "Punch/Brake" (Left) is on IN.1 while "Run" (Right)
// is on IN.3, and there's no Up/Down at all. Confirmed directly against
// MAME's own INPUT_PORTS, not assumed. Physical: left = "PUNCH/BRAKE" /
// "RUN" rocker; right = "TIE" (single button) above "PICK" (single
// button) - 8 IN.x ports total (the most of any game in this batch).
class TFLASH : public Virtual_Input{
    public :
        TFLASH(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_LEFTRIGHT;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(7, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(6, 0, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break; // Punch/Brake
                        case BUTTON_RIGHT: cpu->input_set(3, 1, state); break; // Run
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(4, 1, state); break; // Tie
                        case BUTTON_DOWN: cpu->input_set(5, 0, state); break; // Pick
                        default: break; } break;
                default: break;
            }
        }
};

////// Pit-Fighter (Tiger) : SM510 //////
// Genuinely different shape from every other game in this file: MAME
// defines two independent 4-way joysticks (JOYSTICKLEFT_*/JOYSTICKRIGHT_*)
// rather than one movement d-pad plus face buttons - this console's two
// controls really are a left-fist stick and a right-fist stick, so they
// map directly onto PART_LEFT/PART_RIGHT's existing 4-direction slots with
// no compromise. Physical: both sides are 4-way sticks labeled "ATTACK"
// (up) / "PICK" (down).
class TPITFIGHT : public Virtual_Input{
    public :
        TPITFIGHT(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_4_BUTTON_DIRECTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(5, 2, state); break; // Select
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break; // Left Attack High
                        case BUTTON_RIGHT: cpu->input_set(2, 2, state); break;
                        case BUTTON_LEFT: cpu->input_set(3, 2, state); break; // Left Attack Low
                        case BUTTON_DOWN: cpu->input_set(4, 2, state); break; // Pick On Left
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_LEFT: cpu->input_set(0, 0, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 0, state); break; // Right Attack Low
                        case BUTTON_UP: cpu->input_set(1, 1, state); break; // Right Attack High
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break; // Pick On Right
                        default: break; } break;
                default: break;
            }
        }
};

////// Apollo 13 (Tiger) : SM510 //////
// Physical: left = full 4-way d-pad; right = 2x2 button grid, "THRUSTER
// LEFT"/"THRUSTER RIGHT" on top, "PARACHUTE"/"CAPTURE" on bottom (same
// physical grid shape confirmed via screenshot on Batman Forever/GoldenEye
// below - reading order top-left/top-right/bottom-left/bottom-right maps
// to Up/Right/Left/Down respectively for all of this batch's grid-style
// consoles).
class TAPOLLO13 : public Virtual_Input{
    public :
        TAPOLLO13(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_4_BUTTON_DIRECTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(5, 0, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 0, state); break; // Thruster Left
                        case BUTTON_RIGHT: cpu->input_set(4, 0, state); break; // Thruster Right
                        case BUTTON_LEFT: cpu->input_set(3, 1, state); break; // Parachute
                        case BUTTON_DOWN: cpu->input_set(4, 1, state); break; // Capture
                        default: break; } break;
                default: break;
            }
        }
};

////// Batman Forever: Double Dose of Doom (Tiger) : SM510 //////
// Physical: left = full 4-way d-pad; right = 2x2 button grid, "GRAPPLING
// GUN"/"BATARANG" on top, "HELP"/"THRUSTER" on bottom (see screenshot -
// confirmed via CONVERT_ROM/rom/console/tbatfor.png).
class TBATFOR : public Virtual_Input{
    public :
        TBATFOR(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_4_BUTTON_DIRECTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(5, 0, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 0, state); break; // Grappling Gun
                        case BUTTON_RIGHT: cpu->input_set(4, 0, state); break; // Batarang
                        case BUTTON_LEFT: cpu->input_set(3, 1, state); break; // Help
                        case BUTTON_DOWN: cpu->input_set(4, 1, state); break; // Thruster
                        default: break; } break;
                default: break;
            }
        }
};

////// Dennis the Menace (Tiger) : SM510 //////
// Physical: left = full 4-way d-pad ("BRAKE" on Down); right = 2x2 button
// grid, "SLINGSHOT LEFT"/"SLINGSHOT RIGHT" on top, "PICK LEFT"/"PICK
// RIGHT" on bottom (same grid shape as Batman Forever/GoldenEye above).
class TDENNIS : public Virtual_Input{
    public :
        TDENNIS(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_4_BUTTON_DIRECTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(5, 0, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break; // Brake
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 0, state); break; // Slingshot Left
                        case BUTTON_RIGHT: cpu->input_set(4, 0, state); break; // Slingshot Right
                        case BUTTON_LEFT: cpu->input_set(3, 1, state); break; // Pick Left
                        case BUTTON_DOWN: cpu->input_set(4, 1, state); break; // Pick Right
                        default: break; } break;
                default: break;
            }
        }
};

////// Judge Dredd (Tiger) : SM510 //////
// Physical: left = full 4-way d-pad; right = 2x2 button grid,
// "KICK"/"PUNCH" on top, "PICK/CALL"/"FIRE" on bottom (same grid shape as
// Batman Forever/GoldenEye above).
class TJDREDD : public Virtual_Input{
    public :
        TJDREDD(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_4_BUTTON_DIRECTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(5, 0, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 0, state); break; // Kick
                        case BUTTON_RIGHT: cpu->input_set(4, 0, state); break; // Punch
                        case BUTTON_LEFT: cpu->input_set(3, 1, state); break; // Pick/Call
                        case BUTTON_DOWN: cpu->input_set(4, 1, state); break; // Fire
                        default: break; } break;
                default: break;
            }
        }
};

////// 007: GoldenEye (Tiger) : SM510 //////
// Physical: left = full 4-way d-pad; right = 2x2 button grid,
// "KICK"/"PUNCH" on top, "Q"/"FIRE" on bottom (confirmed via screenshot -
// CONVERT_ROM/rom/console/tgoldeye.png).
class TGOLDEYE : public Virtual_Input{
    public :
        TGOLDEYE(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_4_BUTTON_DIRECTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(5, 0, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 0, state); break; // Kick
                        case BUTTON_RIGHT: cpu->input_set(4, 0, state); break; // Punch
                        case BUTTON_LEFT: cpu->input_set(3, 1, state); break; // Q
                        case BUTTON_DOWN: cpu->input_set(4, 1, state); break; // Fire
                        default: break; } break;
                default: break;
            }
        }
};

////// Kazaam (Tiger) : SM510 //////
// Physical: left = full 4-way d-pad; right = 2x2 button grid,
// "KICK"/"PUNCH FORWARD" on top, "PICK/RESCUE"/"PUNCH BACK" on bottom
// (same grid shape as Batman Forever/GoldenEye above).
class TKAZAAM : public Virtual_Input{
    public :
        TKAZAAM(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_4_BUTTON_DIRECTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(5, 0, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 0, state); break; // Kick
                        case BUTTON_RIGHT: cpu->input_set(4, 0, state); break; // Punch Forward
                        case BUTTON_LEFT: cpu->input_set(3, 1, state); break; // Pick/Rescue
                        case BUTTON_DOWN: cpu->input_set(4, 1, state); break; // Punch Back
                        default: break; } break;
                default: break;
            }
        }
};

////// Batman: The Animated Series (Tiger) : SM510 //////
// NOTE: this chip variant doesn't use the first scan line, same quirk as
// Superman/Gargoyles above (MAME's own port comments read S2, S3, S4...
// instead of S1, S2, S3...) - every group value here is the actual S-line
// minus 1. Physical: left = "JUMP"/"FAST" rocker (no left/right at all on
// this romset - confirmed against its own MAME INPUT_PORTS); right =
// single big oval button labeled "THROW/ATTACK".
class TBATMANA : public Virtual_Input{
    public :
        TBATMANA(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(6, 0, state); break; // Max Score
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(1, 2, state); break; // Jump
                        case BUTTON_DOWN: cpu->input_set(3, 1, state); break; // Fast
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(4, 1, state); break; // Throw/Attack
                        default: break; } break;
                default: break;
            }
        }
};

////// Mighty Max (Tiger) : SM510 //////
// Same S2-start chip quirk as Batman: The Animated Series/Superman/
// Gargoyles above - every group value here is the actual S-line minus 1.
// Physical: left/right-only rocker (no up/down on this romset); right = 2
// stacked buttons, "PICK" above "CALL" (confirmed via screenshot -
// CONVERT_ROM/rom/console/tmigmax.png).
class TMIGMAX : public Virtual_Input{
    public :
        TMIGMAX(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_LEFTRIGHT;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(6, 3, state, true); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(6, 0, state); break; // Max Score
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(2, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(2, 2, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(4, 1, state); break; // Pick
                        case BUTTON_DOWN: cpu->input_set(5, 0, state); break; // Call
                        default: break; } break;
                default: break;
            }
        }
};

////// Double Dribble (Konami) : SM510 //////
// Physical: left = full 4-way d-pad; right = single unlabeled round button
// (MAME gives this no PORT_NAME - the console art itself has no text on
// it either, matching every other classic Konami handheld below).
class KDRIBBLE : public Virtual_Input{
    public :
        KDRIBBLE(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(2, 0, state); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(1, 0, state); break; // Level Select
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_LEFT: cpu->input_set(0, 0, state); break;
                        case BUTTON_UP: cpu->input_set(0, 1, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(1, 3, state); break; // Action
                        default: break; } break;
                default: break;
            }
        }
};

////// Contra (Konami) : SM511 //////
// Physical: left = "LEFT"/"RIGHT" rocker (no up/down on this romset);
// right = single unlabeled round button.
class KCONTRA : public Virtual_Input{
    public :
        KCONTRA(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_LEFTRIGHT;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(2, 0, state); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(1, 0, state); break; // Select
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_LEFT: cpu->input_set(0, 0, state); break;
                        case BUTTON_RIGHT: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(1, 3, state); break; // Action
                        default: break; } break;
                default: break;
            }
        }
};

////// Top Gun (Konami) : SM510 //////
// Same shape as Double Dribble above. Physical: left = full 4-way d-pad;
// right = single unlabeled round button.
class KTOPGUN : public Virtual_Input{
    public :
        KTOPGUN(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(2, 0, state); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(1, 0, state); break; // Select
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_LEFT: cpu->input_set(0, 0, state); break;
                        case BUTTON_UP: cpu->input_set(0, 1, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(1, 3, state); break; // Action
                        default: break; } break;
                default: break;
            }
        }
};

////// Gradius (Konami) : SM511 //////
// Physical: left = "UP"/"DOWN" rocker (no left/right on this romset);
// right = single unlabeled round button.
class KGRADIUS : public Virtual_Input{
    public :
        KGRADIUS(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(0, 0, state); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(0, 2, state); break; // Select
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(1, 0, state); break;
                        case BUTTON_DOWN: cpu->input_set(1, 1, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(1, 2, state); break; // Action
                        default: break; } break;
                default: break;
            }
        }
};

////// Teenage Mutant Ninja Turtles (Konami) : SM511 //////
// Genuinely two joysticks, like Pit-Fighter earlier in this file: MAME
// defines separate JOYSTICKLEFT_*/JOYSTICKRIGHT_* ports. Physical: left =
// full 4-way d-pad; right = same round 4-way-shaped control, but only
// Left/Right are actually wired on this romset (confirmed via screenshot -
// the case has two identical-looking cross buttons even though the right
// one only responds to two of its four positions here).
class KTMNT : public Virtual_Input{
    public :
        KTMNT(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_2_BUTTON_LEFTRIGHT;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(1, 3, state); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(1, 0, state); break; // Game Select
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_LEFT: cpu->input_set(0, 0, state); break;
                        case BUTTON_UP: cpu->input_set(0, 1, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_LEFT: cpu->input_set(2, 0, state); break;
                        case BUTTON_RIGHT: cpu->input_set(2, 1, state); break;
                        default: break; } break;
                default: break;
            }
        }
};

////// Skate or Die (Konami) : SM511 //////
// Physical: left = 4-way-shaped d-pad, but only Left/Up/Right are wired on
// this romset (no Down); right = "UP"/"DOWN" rocker, split across two
// separate MAME ports (still just one physical rocker).
class KSKATEDIE : public Virtual_Input{
    public :
        KSKATEDIE(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(1, 3, state); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(1, 2, state); break; // Continue
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_LEFT: cpu->input_set(0, 0, state); break;
                        case BUTTON_UP: cpu->input_set(0, 1, state); break;
                        case BUTTON_RIGHT: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 2, state); break;
                        default: break; } break;
                default: break;
            }
        }
};

////// The Adventures of Bayou Billy (Konami) : SM511 //////
// Physical: left = full 4-way d-pad; right = "UP"/"DOWN" rocker.
class KBILLY : public Virtual_Input{
    public :
        KBILLY(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(1, 3, state); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(1, 2, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_LEFT: cpu->input_set(0, 0, state); break;
                        case BUTTON_UP: cpu->input_set(0, 1, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 1, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 2, state); break;
                        default: break; } break;
                default: break;
            }
        }
};

////// Bottom of the Ninth (Konami) : SM511 //////
// Physical: left = full 4-way d-pad; right = single unlabeled round button.
class KBOTTOM9 : public Virtual_Input{
    public :
        KBOTTOM9(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(1, 3, state); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(1, 2, state); break; // Level/Time
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_LEFT: cpu->input_set(0, 0, state); break;
                        case BUTTON_UP: cpu->input_set(0, 1, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 2, state); break; // Action
                        default: break; } break;
                default: break;
            }
        }
};

////// The Lone Ranger (Konami) : SM511 //////
// Physical: left = "LEFT"/"RIGHT" rocker (no up/down on this romset);
// right = single unlabeled round button (shares a MAME port with Left,
// which is fine - group/line addressing doesn't care which physical side
// a line ends up wired to).
class KLONERAN : public Virtual_Input{
    public :
        KLONERAN(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_LEFTRIGHT;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(1, 3, state); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(1, 2, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_LEFT: cpu->input_set(0, 0, state); break;
                        case BUTTON_RIGHT: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 1, state); break; // Action
                        default: break; } break;
                default: break;
            }
        }
};

////// Bill Elliott's NASCAR Racing (Konami) : SM511 //////
// Also serves Chequered Flag (kchqflag, reuses this romset's hardware
// entirely - MAME's own SYST() entry names knascar_state directly as its
// device class) and Top Gun: Second Mission (ktopgun2, verified
// bit-for-bit identical port layout to this one, just a different game).
// Physical: left = "LEFT"/"RIGHT" rocker; right = "UP"/"DOWN" rocker -
// both two-stick, no diagonals on either side.
class KNASCAR : public Virtual_Input{
    public :
        KNASCAR(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_LEFTRIGHT;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(0, 3, state); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(0, 2, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_LEFT: cpu->input_set(0, 0, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 0, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 1, state); break;
                        case BUTTON_DOWN: cpu->input_set(1, 1, state); break;
                        default: break; } break;
                default: break;
            }
        }
};

////// Blades of Steel (Konami) : SM511 //////
// Physical: left = full 4-way d-pad; right = single unlabeled round button.
class KBLADES : public Virtual_Input{
    public :
        KBLADES(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(1, 3, state); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(1, 2, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_LEFT: cpu->input_set(0, 0, state); break;
                        case BUTTON_UP: cpu->input_set(0, 1, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 0, state); break; // Action
                        default: break; } break;
                default: break;
            }
        }
};

////// Teenage Mutant Ninja Turtles II: Splinter Speaks (Konami) : SM511 //////
// Physical: left = full 4-way d-pad; right = single unlabeled round button.
class KTMNT2 : public Virtual_Input{
    public :
        KTMNT2(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(0, 3, state); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(0, 1, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(1, 0, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 1, state); break;
                        case BUTTON_UP: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(1, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 0, state); break; // Action
                        default: break; } break;
                default: break;
            }
        }
};

////// NFL Football (Konami) : SM511 //////
// Same shape as Blades of Steel above. Physical: left = full 4-way d-pad;
// right = single unlabeled round button.
class KNFL : public Virtual_Input{
    public :
        KNFL(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(1, 3, state); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(1, 2, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_LEFT: cpu->input_set(0, 0, state); break;
                        case BUTTON_UP: cpu->input_set(0, 1, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 0, state); break; // Action
                        default: break; } break;
                default: break;
            }
        }
};

////// Star Trek: 25th Anniversary (Konami) : SM511 //////
// NOTE: MAME's own port definition ties Up and Down to the exact same
// physical line via a "FAKE" port condition, commented directly in MAME's
// source as "Up/Down are electronically the same button" - both mapped to
// the same (group,line) here to match. Physical: left = "LEFT"/"RIGHT"
// rocker with a combined "UP/DOWN" position; right = single unlabeled
// round button.
class KST25 : public Virtual_Input{
    public :
        KST25(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(1, 3, state); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(1, 2, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_LEFT: cpu->input_set(0, 0, state); break;
                        case BUTTON_UP: cpu->input_set(0, 1, state); break; // Up/Down (same line)
                        case BUTTON_DOWN: cpu->input_set(0, 1, state); break; // Up/Down (same line)
                        case BUTTON_RIGHT: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 2, state); break; // Action
                        default: break; } break;
                default: break;
            }
        }
};

////// Top Gun: Second Mission (Konami) : SM511 //////
// See KNASCAR above - bit-for-bit identical port layout, dispatched
// straight to that class.

////// Teenage Mutant Ninja Turtles 3: Shredder's Last Stand (Konami) : SM511 //////
// NOTE: despite sharing a model line with Teenage Mutant Ninja Turtles:
// Basketball (ktmntbb) below, this romset's INPUT_PORTS are NOT
// bit-for-bit identical to it - confirmed directly against MAME's own
// port definitions, correcting an earlier assumption in this file. See
// KTMNTBB's own comment for the differences. Physical: left = full 4-way
// d-pad; right = single unlabeled round button.
class KTMNT3 : public Virtual_Input{
    public :
        KTMNT3(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(0, 3, state); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(0, 2, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_LEFT: cpu->input_set(0, 0, state); break;
                        case BUTTON_UP: cpu->input_set(0, 1, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 0, state); break;
                        case BUTTON_DOWN: cpu->input_set(1, 1, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 3, state); break; // Action
                        default: break; } break;
                default: break;
            }
        }
};

////// Teenage Mutant Ninja Turtles: Basketball (Konami) : SM511 //////
// NOT the same port layout as KTMNT3 above despite the similar name/model
// line - confirmed directly against MAME's own port definitions. All four
// directions live on a single port here (KTMNT3 splits them across two),
// and Start/Pause live on a different port/line entirely. This was the
// cause of the game being unable to start: the old shared class sent
// Start to a line that's actually wired to "D-Pad Right" on this romset.
// Physical: left = full 4-way d-pad; right = single unlabeled round
// button.
class KTMNTBB : public Virtual_Input{
    public :
        KTMNTBB(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(1, 3, state); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(1, 2, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_LEFT: cpu->input_set(0, 0, state); break;
                        case BUTTON_UP: cpu->input_set(0, 1, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 2, state); break;
                        case BUTTON_RIGHT: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 3, state); break; // Action
                        default: break; } break;
                default: break;
            }
        }
};

////// Bucky O'Hare (Konami) : SM511 //////
// Physical: left = "UP"/"DOWN" rocker (no left/right on this romset);
// right = single unlabeled round button (shares a MAME port with Up).
class KBUCKY : public Virtual_Input{
    public :
        KBUCKY(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(0, 3, state); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(0, 2, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 0, state); break;
                        case BUTTON_DOWN: cpu->input_set(1, 0, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 1, state); break; // Action
                        default: break; } break;
                default: break;
            }
        }
};

////// Garfield (Konami) : SM511 //////
// Physical: left = full 4-way d-pad; right = single unlabeled round button.
class KGARFLD : public Virtual_Input{
    public :
        KGARFLD(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(1, 3, state); break; // Power On/Start
                        case BUTTON_GAMEB: cpu->input_set(1, 2, state); break; // Pause
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_LEFT: cpu->input_set(0, 0, state); break;
                        case BUTTON_UP: cpu->input_set(0, 1, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 0, state); break;
                        case BUTTON_DOWN: cpu->input_set(1, 1, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 3, state); break; // Action
                        default: break; } break;
                default: break;
            }
        }
};

////// Treasure Island (Tronica) : SM511 //////
// Physical: left = full 4-way d-pad; right = single button labeled
// "Start/Jump/Pick" on the console (any of A/B/X/Y triggers it - MAME's
// own port comment notes this button is electronically the same line as a
// separate clock-setting "Hour" function, not modeled here - see below).
// "Game A"/"Game B" map directly onto our GameA/GameB slots by name.
// Clock-only functions (Min, Alarm, and the "Hour" half of the shared
// Start/Jump/Pick line) have no equivalent slot in this scheme and are
// intentionally left unmapped, same as every other game's ACL/reset.
class TRTREISL : public Virtual_Input{
    public :
        TRTREISL(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(3, 1, state); break; // Game A
                        case BUTTON_GAMEB: cpu->input_set(3, 2, state); break; // Game B
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(1, 0, state); break;
                        case BUTTON_UP: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(1, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 0, state); break; // Start/Jump/Pick
                        default: break; } break;
                default: break;
            }
        }
};

////// Vesolaya arifmetika (Elektronika) : SM511 //////
// Physical: left/right-only... actually up/down-only rockers on both
// sides (no left/right at all on either) - a two-stick, vertical-only
// layout. "Game" and "Start" map onto our GameA/GameB slots by name;
// "Time" maps to the Time (L) slot. "Alarm" has no equivalent slot here
// and is left unmapped, same as every other clock-utility button in this
// batch.
class VESARIF : public Virtual_Input{
    public :
        VESARIF(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(0, 0, state); break; // Game
                        case BUTTON_GAMEB: cpu->input_set(0, 1, state); break; // Start
                        case BUTTON_TIME: cpu->input_set(0, 2, state); break; // Time
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(1, 0, state); break;
                        case BUTTON_DOWN: cpu->input_set(1, 1, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(1, 3, state); break;
                        case BUTTON_DOWN: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                default: break;
            }
        }
};

////// Bassmate Computer (Telko / Nintendo) : SM511 //////
// Not a game at all - a real fishing-computer novelty device with a
// keypad of 9 distinct labeled function buttons (Compute, Wind, Cover,
// Time of Day, Water Temp F, Water Clarity, Structure, Water Depth(ft),
// Season, Alarm - see the console screenshot), nothing resembling a
// d-pad. There is no physically-meaningful way to preserve this on our
// PART_LEFT/PART_RIGHT diamond layout, so the 8 non-Compute/Alarm buttons
// are assigned to the 8 available direction slots in the same order MAME
// itself lists their port bits in, purely for a stable, documented
// mapping - see switch/standard_controls.md for the exact button-by-
// button listing, since none of this is guessable from the Switch
// controller alone the way every other game in this file is.
class BASSMATE : public Virtual_Input{
    public :
        BASSMATE(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_4_BUTTON_DIRECTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(0, 0, state); break; // Compute
                        case BUTTON_GAMEB: cpu->input_set(0, 1, state); break; // Alarm
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(1, 0, state); break; // Wind
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break; // Cover
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break; // Time of Day
                        case BUTTON_DOWN: cpu->input_set(1, 3, state); break; // Water Temp F
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 0, state); break; // Water Clarity
                        case BUTTON_RIGHT: cpu->input_set(2, 1, state); break; // Structure
                        case BUTTON_LEFT: cpu->input_set(2, 2, state); break; // Water Depth(ft)
                        case BUTTON_DOWN: cpu->input_set(2, 3, state); break; // Season
                        default: break; } break;
                default: break;
            }
        }
};


////// Altair (Nauchpribor bootleg of Elektronika IM-22 / Vesyolye futbolisty) : SM5A //////
// Same board/ports as MC_25 (IM_22 among others) - MAME's own driver
// comment says this unit "uses the same screen as IM-22", and naltair.svg
// confirms it (identical viewBox/docname to vfutbol.svg - the same LCD
// scan). Left/Right stick Up/Down and Game A/B/Time are wired identically
// to MC_25 below; the one real difference (IN.0, unused on every other
// gnw_mmouse-family game) is repurposed here as a 4-bit "Dosimeter
// Reading" input fed from a settings-menu value, not a normal button - see
// g_dosimeter_reading (switch_settings.h) and its per-frame feed into
// k_input group 1 (this engine's group numbering runs one higher than
// MAME's IN.N port index - group 0 is reserved for the non-multiplexed
// ACL/alpha/beta lines) in switch/source/main.cpp. Nothing to do here for
// that input: this class only owns the buttons that behave like every
// other gnw_mmouse-family game.
class NALTAIR : public Virtual_Input{
    public :
        NALTAIR(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(3, 2, state); break;
                        case BUTTON_GAMEB: cpu->input_set(3, 1, state); break;
                        case BUTTON_TIME: cpu->input_set(3, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 3, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 2, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 1, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Star Fox (Nelsonic) : SM530 //////
// SM530's K register is a single non-multiplexed 8-bit input (op_kta/op_keta
// read it as two nibbles), not 8 multiplexed groups like every other core
// above - so every input_set() call here passes not_multiplex=true and
// "line" is just the bit position in that one register (0-7), matching
// MAME's IN.0 port exactly (see hh_sm510.cpp's nstarfox INPUT_PORTS_START,
// cross-checked against KTA/KETA's own bit split in SM530_instruction.cpp).
// Two real button-layout revisions exist on physical units but MAME treats
// them as the same game/same input port, so this covers both.
class NSTARFOX : public Virtual_Input{
    public :
        NSTARFOX(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_2_BUTTON_LEFTRIGHT; // laser / bomb - two independent action buttons, not literal left/right (same convention KTMNT uses)
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 4, state, true); break;
                        case BUTTON_RIGHT: cpu->input_set(0, 5, state, true); break;
                        case BUTTON_DOWN: cpu->input_set(0, 6, state, true); break;
                        case BUTTON_LEFT: cpu->input_set(0, 7, state, true); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_LEFT: cpu->input_set(0, 1, state, true); break; // laser
                        case BUTTON_RIGHT: cpu->input_set(0, 2, state, true); break; // bomb
                        default: break; } break;
                default: break;
            }
        }
};


////// Super Mario Bros. 3 / Super Mario World (Nelsonic) : SM530 //////
// Same IN.0 bit layout as NSTARFOX above (same base gamewatch_state driver -
// nstarfox's INPUT_PORTS_START( nstarfox ) only PORT_MODIFYs bit1/bit2's
// label from mode/set to laser/bomb, same bits either way), but here bit1/
// bit2 are real mode/set buttons (cycle watch mode, adjust time) rather
// than gameplay actions, so they're mapped as PART_SETUP Game A/B - matching
// this codebase's convention for mode-select-style buttons elsewhere -
// instead of NSTARFOX's PART_RIGHT action pair.
class NSMB3 : public Virtual_Input{
    public :
        NSMB3(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_NOTHING;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(0, 1, state, true); break; // mode
                        case BUTTON_GAMEB: cpu->input_set(0, 2, state, true); break; // set
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 4, state, true); break;
                        case BUTTON_RIGHT: cpu->input_set(0, 5, state, true); break;
                        case BUTTON_DOWN: cpu->input_set(0, 6, state, true); break;
                        case BUTTON_LEFT: cpu->input_set(0, 7, state, true); break;
                        default: break; } break;
                default: break;
            }
        }
};

class NSMW : public Virtual_Input{
    public :
        NSMW(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_NOTHING;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(0, 1, state, true); break; // mode
                        case BUTTON_GAMEB: cpu->input_set(0, 2, state, true); break; // set
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 4, state, true); break;
                        case BUTTON_RIGHT: cpu->input_set(0, 5, state, true); break;
                        case BUTTON_DOWN: cpu->input_set(0, 6, state, true); break;
                        case BUTTON_LEFT: cpu->input_set(0, 7, state, true); break;
                        default: break; } break;
                default: break;
            }
        }
};



//////////////////// Get good Game & Watch input Configuration ////////////////////

inline Virtual_Input* get_input_config(SM5XX* cpu, std::string ref_game){
    /* SM5A */
    if (ref_game == "AC_01") { return new AC_01(cpu); } // BALL
    else if (ref_game == "FL_02") { return new FL_02(cpu); } // Flagman
    else if (ref_game == "MT_03") { return new MT_03(cpu); } // Vermin
    else if (ref_game == "RC_04") { return new RC_04(cpu); } // Fire
    else if (ref_game == "IP_05" || ref_game == "IP_15") { return new IP_05(cpu); } // Judge
    else if (ref_game == "MH_06") { return new MH_06(cpu); } // Manhole
    else if (ref_game == "CN_07" || ref_game == "CN_17") { return new CN_07(cpu); } // Helmet    
    else if (ref_game == "LN_08") { return new LN_08(cpu); } // Lion
    else if (ref_game == "PR_21") { return new PR_21(cpu); } // Parachute
    else if (ref_game == "OC_22" || ref_game == "IM_03") { return new OC_22(cpu); } // Octopus / Tayny okeana
    else if (ref_game == "PP_23") { return new PP_23(cpu); } // Popeye
    else if (ref_game == "FP_24" || ref_game == "IM_04") { return new FP_24(cpu); } // Chef / Vesyolyy povar
    else if (ref_game == "MC_25" || ref_game == "EG_26" || ref_game == "IM_53") { return new MC_25(cpu); } // Mickey Mouse / Egg / Ataka asteroidov
    else if (ref_game == "IM_19" || ref_game == "IM_11" || ref_game == "IM_10") { return new MC_25(cpu); } // Biathlon / Circus / Hockey
    else if (ref_game == "IM_50" || ref_game == "IM_32" || ref_game == "IM_33") { return new MC_25(cpu); } // Kosmicheskiy polyot / Kot-rybolov / Kvaka-zadavaka
    else if (ref_game == "IM_51" || ref_game == "IM_49" || ref_game == "IM_02") { return new MC_25(cpu); } // Morskaja ataka / Nochnye vorishki / Nu, pogodi!
    else if (ref_game == "IM_16" || ref_game == "IM_13" || ref_game == "IM_22") { return new MC_25(cpu); } // Okhota / Razvedchiki kosmosa / Vesyolye futbolisty
    else if (ref_game == "FR_27" || ref_game == "IM_09") { return new FR_27(cpu); } // Fire (Wide Screen) / Kosmicheskiy most
    else if (ref_game == "SM_11" || ref_game == "SG_21") { return new SM_11(cpu); } // Space Mission / Spider (Tronica)
    else if (ref_game == "SK_10") { return new SK_10(cpu); } // Super Goal Keeper (Tronica)
    else if (ref_game == "IM_23") { return new IM_23(cpu); } // Autoslalom (Elektronika)

    /* SM510 */
    else if (ref_game == "TL_28") { return new TL_28(cpu); } // Turtle Bridge
    else if (ref_game == "ID_29") { return new ID_29(cpu); } // Fire Attack
    else if (ref_game == "SP_30") { return new SP_30(cpu); } // Snoopy Tennis
    else if (ref_game == "OP_51") { return new OP_51(cpu); } // Oil Panic
    else if (ref_game == "DK_52") { return new DK_52(cpu); } // Donkey kong
    else if (ref_game == "DM_53") { return new DM_53(cpu); } // Mickey & Donald 
    else if (ref_game == "GH_54") { return new GH_54(cpu); } // Green House 
    else if (ref_game == "JR_55") { return new JR_55(cpu); } // Donkey Kong II 
    else if (ref_game == "MW_56") { return new MW_56(cpu); } // Mario Bros 
    else if (ref_game == "LP_57") { return new LP_57(cpu); } // Rain Shower 
    else if (ref_game == "TC_58") { return new TC_58(cpu); } // Life Boat 
    else if (ref_game == "MG_61") { return new MG_61(cpu); } // Squish 
    else if (ref_game == "DJ_101") { return new DJ_101(cpu); } // DK JR (Wide Screen)
    else if (ref_game == "ML_102") { return new ML_102(cpu); } // Mario's Cement Factory (Wide Screen)
    else if (ref_game == "NH_103") { return new NH_103(cpu); } // Manhole (Wide Screen)
    else if (ref_game == "TF_104") { return new TF_104(cpu); } // Tropical Fish
    else if (ref_game == "BU_201") { return new BU_201(cpu); } // Spitball Sparky
    else if (ref_game == "UD_202") { return new UD_202(cpu); } // Crab Grab
    else if (ref_game == "MG_8" || ref_game == "TG_18")   { return new MG_8(cpu); }   // Shuttle Voyage / Thief in Garden (Tronica)
    else if (ref_game == "DA_37")   { return new DA_37(cpu); }   // Diver's Adventure (Tronica)
    else if (ref_game == "CC_38V")  { return new CC_38V(cpu); }  // Clever Chicken (Tronica)
    else if (ref_game == "MG_9" || ref_game == "FR_23") { return new MG_9(cpu); } // Space Rescue / Thunder Ball (Tronica)
    

    /* SM511/SM512 */
    else if (ref_game == "PB_59") { return new PB_59(cpu); } // Pinball
    else if (ref_game == "BJ_60") { return new BJ_60(cpu); } // Black jack
    else if (ref_game == "BD_62") { return new BD_62(cpu); } // Bomb Sweeper
    else if (ref_game == "JB_63") { return new JB_63(cpu); } // Safe Buster
    else if (ref_game == "MV_64") { return new MV_64(cpu); } // Gold Cliff
    else if (ref_game == "ZL_65") { return new ZL_65(cpu); } // Zelda    
    else if (ref_game == "CM_72" || ref_game == "CM_72A") { return new CM_72(cpu); } // Mario's Cement Factory
    else if (ref_game == "SM_91") { return new SM_91(cpu); } // Snoopy (table top)
    else if (ref_game == "PG_92") { return new PG_92(cpu); } // Popeye (table top)
    else if (ref_game == "CJ_93" || ref_game == "IM_12") { return new CJ_93(cpu); } // DK JR (Panorama) / Vinni-Pukh
    else if (ref_game == "TB_94") { return new TB_94(cpu); } // Mario's Bombs Away
    else if (ref_game == "DC_95" || ref_game == "MK_96") { return new DC_95(cpu); } // Donkey Kong Circus / Mickey Mouse
    else if (ref_game == "YM_801" || ref_game == "YM_105") { return new YM_801(cpu); } // Super Mario Bros
    else if (ref_game == "DR_802" || ref_game == "DR_106") { return new DR_802(cpu); } // Climber
    else if (ref_game == "BF_803" || ref_game == "BF_107") { return new BF_803(cpu); } // Balloon Fight
    else if (ref_game == "MB_108") { return new MB_108(cpu); } // Mario The Juggler
    else if (ref_game == "BX_301" || ref_game == "AK_302" || ref_game == "HK_303") { return new BX_301(cpu); } // all vs g&w (3)
    else if (ref_game == "SA_12") { return new SA_12(cpu); } // Space Adventure (Tronica)
    
    /* David and John */
    else if(ref_game == "D_AND_J_CLIMBER" || ref_game == "D_AND_J_AVIGNON" || ref_game == "D_AND_J_RYTHME")
        { return new d_and_j(cpu); }
    else if (ref_game == "D_AND_J_FABRIC"){ return new d_and_j_fabric(cpu); }
    else if (ref_game == "D_AND_J_ESQUIVE"){ return new d_and_j_esquive(cpu); }
    
    /* Tiger */
    else if (ref_game == "TSONIC2" || ref_game == "TSONIC"){ return new TSONIC2(cpu); } // Sonic 1 / Sonic 2
    else if (ref_game == "TSFIGHT2" || ref_game == "TALTBEAST" // Street Fighter II / Altered Best
            || ref_game == "TDDRAGON2"){ return new TSFIGHT2(cpu); } // Double Dragon 2
    else if (ref_game == "TDDRAGON" || ref_game == "TMEGAMAN3"  // Double Dragon / Mega man 3
            || ref_game == "TGAIDEN" || ref_game == "TGAIDEN3" // Ninja gaiden / Ninja gaiden 3
            || ref_game == "TSTRIDER" ){ return new TDDRAGON(cpu); } // Strider 
    else if (ref_game == "TSIMQUEST"){ return new TSIMQUEST(cpu); } // Castlevania II
    else if (ref_game == "TDDRAGON3"){ return new TDDRAGON3(cpu); } // Double Dragon 3
    else if (ref_game == "TSDDRAGON"){ return new TSDDRAGON(cpu); } // Super Double Dragon
    else if (ref_game == "TGOLDNAXE"){ return new TGOLDNAXE(cpu); } // Golden Axe
    else if (ref_game == "TGAUNT"){ return new TGAUNT(cpu); } // Gautlet
    else if (ref_game == "TSHARR2"){ return new TSHARR2(cpu); } // Space Harrier II
    else if (ref_game == "TMCHAMMER"){ return new TMCHAMMER(cpu); } // MC Hammer

    /* Tiger (Experimental) */
    else if (ref_game == "TBATMAN"){ return new TBATMAN(cpu); } // Batman
    else if (ref_game == "TSPIDMAN"){ return new TSPIDMAN(cpu); } // Spider-Man
    else if (ref_game == "TXMEN"){ return new TXMEN(cpu); } // X-Men
    else if (ref_game == "TMKOMBAT"){ return new TMKOMBAT(cpu); } // Mortal Kombat
    else if (ref_game == "TSUPERMAN" || ref_game == "TGARGNF"){ return new TSUPERMAN(cpu); } // Superman / Gargoyles: Night Flight (identical ports)
    else if (ref_game == "TBTOADS"){ return new TBTOADS(cpu); } // Battletoads
    else if (ref_game == "TKARNOV"){ return new TKARNOV(cpu); } // Karnov
    else if (ref_game == "THOOK"){ return new THOOK(cpu); } // Hook
    else if (ref_game == "TBTTF"){ return new TBTTF(cpu); } // Back to the Future
    else if (ref_game == "TTRANSF2"){ return new TTRANSF2(cpu); } // Transformers: Generation 2
    else if (ref_game == "TVINDICTR"){ return new TVINDICTR(cpu); } // Vindicators
    else if (ref_game == "TJPARK"){ return new TJPARK(cpu); } // Jurassic Park
    else if (ref_game == "TSKELWARR"){ return new TSKELWARR(cpu); } // Skeleton Warriors: The Dark Crusade
    else if (ref_game == "TADDAMS"){ return new TADDAMS(cpu); } // The Addams Family
    else if (ref_game == "THALONE"){ return new THALONE(cpu); } // Home Alone
    else if (ref_game == "THALONE2"){ return new THALONE2(cpu); } // Home Alone 2: Lost in New York
    else if (ref_game == "TXMENPX"){ return new TXMENPX(cpu); } // X-Men: Project X
    else if (ref_game == "TWWORLD"){ return new TWWORLD(cpu); } // Wayne's World
    else if (ref_game == "TNMAREBC"){ return new TNMAREBC(cpu); } // The Nightmare Before Christmas
    else if (ref_game == "TOPALIENS"){ return new TOPALIENS(cpu); } // Operation: Aliens
    else if (ref_game == "TSHADOW"){ return new TSHADOW(cpu); } // The Shadow
    else if (ref_game == "TSJAM"){ return new TSJAM(cpu); } // Space Jam
    else if (ref_game == "TSF2010"){ return new TSF2010(cpu); } // Street Fighter 2010: The Final Fight
    else if (ref_game == "TSWAMPT"){ return new TSWAMPT(cpu); } // Swamp Thing
    else if (ref_game == "TDUMMIES"){ return new TDUMMIES(cpu); } // The Incredible Crash Dummies
    else if (ref_game == "TROBOCOP2" || ref_game == "TROCKTEER"){ return new TROBOCOP2(cpu); } // Robocop 2 / The Rocketeer (identical ports, renamed)
    else if (ref_game == "TROBOCOP3"){ return new TROBOCOP3(cpu); } // Robocop 3
    else if (ref_game == "TINDAY"){ return new TINDAY(cpu); } // Independence Day
    else if (ref_game == "TFLASH"){ return new TFLASH(cpu); } // The Flash
    else if (ref_game == "TPITFIGHT"){ return new TPITFIGHT(cpu); } // Pit-Fighter
    else if (ref_game == "TAPOLLO13"){ return new TAPOLLO13(cpu); } // Apollo 13
    else if (ref_game == "TBATFOR"){ return new TBATFOR(cpu); } // Batman Forever: Double Dose of Doom
    else if (ref_game == "TDENNIS"){ return new TDENNIS(cpu); } // Dennis the Menace
    else if (ref_game == "TJDREDD"){ return new TJDREDD(cpu); } // Judge Dredd
    else if (ref_game == "TGOLDEYE"){ return new TGOLDEYE(cpu); } // 007: GoldenEye
    else if (ref_game == "TKAZAAM"){ return new TKAZAAM(cpu); } // Kazaam
    else if (ref_game == "TBATMANA"){ return new TBATMANA(cpu); } // Batman: The Animated Series
    else if (ref_game == "TMIGMAX"){ return new TMIGMAX(cpu); } // Mighty Max
    else if (ref_game == "TROBHOOD"){ return new TGAUNT(cpu); } // Robin Hood (identical ports to Gauntlet, renamed)
    else if (ref_game == "KDRIBBLE"){ return new KDRIBBLE(cpu); } // Double Dribble (Konami)
    else if (ref_game == "KCONTRA"){ return new KCONTRA(cpu); } // Contra (Konami)
    else if (ref_game == "KTOPGUN"){ return new KTOPGUN(cpu); } // Top Gun (Konami)
    else if (ref_game == "KGRADIUS"){ return new KGRADIUS(cpu); } // Gradius (Konami)
    else if (ref_game == "KTMNT"){ return new KTMNT(cpu); } // Teenage Mutant Ninja Turtles (Konami)
    else if (ref_game == "KSKATEDIE"){ return new KSKATEDIE(cpu); } // Skate or Die (Konami)
    else if (ref_game == "KBILLY"){ return new KBILLY(cpu); } // The Adventures of Bayou Billy (Konami)
    else if (ref_game == "KBOTTOM9"){ return new KBOTTOM9(cpu); } // Bottom of the Ninth (Konami)
    else if (ref_game == "KLONERAN"){ return new KLONERAN(cpu); } // The Lone Ranger (Konami)
    else if (ref_game == "KNASCAR" || ref_game == "KCHQFLAG" || ref_game == "KTOPGUN2"){ return new KNASCAR(cpu); } // NASCAR Racing / Chequered Flag / Top Gun: Second Mission (identical ports)
    else if (ref_game == "KBLADES"){ return new KBLADES(cpu); } // Blades of Steel (Konami)
    else if (ref_game == "KTMNT2"){ return new KTMNT2(cpu); } // Teenage Mutant Ninja Turtles II: Splinter Speaks (Konami)
    else if (ref_game == "KNFL"){ return new KNFL(cpu); } // NFL Football (Konami)
    else if (ref_game == "KST25"){ return new KST25(cpu); } // Star Trek: 25th Anniversary (Konami)
    else if (ref_game == "KTMNT3"){ return new KTMNT3(cpu); } // TMNT 3: Shredder's Last Stand
    else if (ref_game == "KTMNTBB"){ return new KTMNTBB(cpu); } // TMNT: Basketball (NOT identical ports to KTMNT3 - see class comment)
    else if (ref_game == "KBUCKY"){ return new KBUCKY(cpu); } // Bucky O'Hare (Konami)
    else if (ref_game == "KGARFLD"){ return new KGARFLD(cpu); } // Garfield (Konami)
    else if (ref_game == "TRTREISL"){ return new TRTREISL(cpu); } // Treasure Island (Tronica)
    else if (ref_game == "VESARIF"){ return new VESARIF(cpu); } // Vesolaya arifmetika (Elektronika)
    else if (ref_game == "BASSMATE"){ return new BASSMATE(cpu); } // Bassmate Computer (Telko / Nintendo)
    else if (ref_game == "NALTAIR"){ return new NALTAIR(cpu); } // Altair (Nauchpribor)

    /* SM530 */
    else if (ref_game == "NSTARFOX"){ return new NSTARFOX(cpu); } // Star Fox (Nelsonic)
    else if (ref_game == "NSMB3"){ return new NSMB3(cpu); } // Super Mario Bros. 3 (Nelsonic)
    else if (ref_game == "NSMW"){ return new NSMW(cpu); } // Super Mario World (Nelsonic)

    return nullptr;
}
