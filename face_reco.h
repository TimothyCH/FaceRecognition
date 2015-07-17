#include <opencv/highgui.h>
#include <opencv/cv.h>

#include <string.h>
#include <string>
#include <vector>
#include <exception>

using cv::Mat;

const std::string path_to_w = "w_matrix.xml";

static int showImage(IplImage* img);
static int changeImageSize(std::string image_path,int size);
static IplImage* cutImage(IplImage* src,CvRect* rect);
static int addImageToImage(IplImage* src,IplImage* des);
static void addNameToImage(int num,IplImage* img);

static IplImage* getFace(IplImage* img,int src);
static bool isSymmetric(cv::InputArray src);

