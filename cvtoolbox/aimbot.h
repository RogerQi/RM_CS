#ifndef AIMBOT_H_
#define AIMBOT_H_

#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;

/**
 * A base class for assisted aiming
 */
class aimbot{
public:
    /**
     * constructor for generic aimbot class; initialize configuration into memory
     */
    aimbot();

    /**
     * deconstructor for generic aimbot class;
     */
     ~aimbot();

     /**
      * Process image (frame) in current video buffer; pure virtual function to be implemented
      * @return vector of cv::Rect object(s)
      */
      virtual vector<Mat> get_hitbox(void) const = 0;

private:
    /* to be added */

};

#endif
