#ifndef IR_AIMBOT_H_
#define IR_AIMBOT_H_

#include "aimbot.h"

/**
 * A mod for assisted aiming; inspired by algorithm from DJI RoboRTS
 */
class ir_aimbot: public aimbot{
public:
    /**
     * constructor; initialize configuration into memory
     */
    ir_aimbot();

    /**
    * class destructor
    */
    ~ir_aimbot();

    /**
    * Process image (frame) in current video buffer; pure virtual function to be implemented
    * @return vector of cv::Rect object(s)
    */
    vector<Mat> get_hitbox(void) const;

private:
    void detect_lights();
    void filter_lights();
    void detect_armor();
    void filter_armor();
    void possible_armor();
};

#endif
