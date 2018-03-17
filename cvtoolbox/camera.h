#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <mutex>
#include <vector>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

/**
 * A base class for generalized camera level processing
 */
class CameraBase {
public:

    /**
     * constructor for CameraBase class
     */
    CameraBase();
    
    /**
     * read an image and copy into src Mat
     * @param dst destination place holder for the stored image
     */
    void get_img(Mat &dst);
   
    /**
     * excecuted in a seperate thread to increase frame rate
     * and to allow multi camera processing
     */
    void set_img();

    /**
     * to be implemented according to the specs of a specific camera
     * @return Mat object read directly from the camera
     */
    virtual Mat cam_read() = 0;
protected:
    unsigned short _write_index;
    unsigned short  _read_index;
    vector<Mat> _buffer;
    mutex _lock;
};

#endif
