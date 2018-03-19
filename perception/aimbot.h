#ifndef AIMBOT_H_
#define AIMBOT_H_

#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;

struct armor_loc{
    float center_x;
    float center_y;
    float width;
    float height;
    float ang;
};

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
     * destructor for generic aimbot class;
     */
     ~aimbot();

     /**
      * Process image (frame) in current video buffer; pure virtual function to be implemented
      * @return vector of cv::Rect object(s)
      */
      virtual vector<armor_loc> get_hitbox(void) const = 0;

private:
    /* to be added */

};

/**
* Magic function that highlights red or blue area
* @param src_img ref to src_img
* @param dst_img ref to an empty img
* @param color_type string. It should be either "blue" or "red"
*/
void distill_color(const Mat & src_img, Mat & dst_img, string color_type) {
    std::vector<cv::Mat> bgr;
    cv::split(src_img, bgr);
    if (color_type == "red") {
        cv::subtract(bgr[2], bgr[1], dst_img);
    } else {
      //assume it's blue
        cv::subtract(bgr[0], bgr[2], dst_img);
    }
}

#endif
