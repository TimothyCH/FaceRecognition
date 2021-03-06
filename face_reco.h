#ifndef FACE_RECO_H
#define FACE_RECO_H

#include <opencv/highgui.h>
#include <opencv/cv.h>

#include <string.h>
#include <string>
#include <vector>
#include <exception>



#define __UNIX__

#include <signal.h>

#include "eigen_decom.h"

#define PATH_TO_W "w_matrix.xml"
#define SAMPLE_BASE_PATH "sample/train/s"
#define CLASS_NUM 42//total number of class to classify.
#define SAMPLE_NUM 8//number of sample in each class.

#define CRITICAL_NUM 110000//the critical smallest value of determine class belongings.

using cv::Mat;


//static function are functions that are only visible in the same file.
int showImage(IplImage* img);
int changeImageSize(std::string image_path,int size);
IplImage* cutImage(IplImage* src,CvRect* rect);
int addImageToImage(IplImage* src,IplImage* des);
void addNameToImage(int num,IplImage* img);

IplImage* getFace(IplImage* img,int src);


CvMat* getItemX(IplImage* img);//count the X Mat of pic.
CvMat* getX(std::vector<std::string> path_vec);
CvMat* getItemU(CvMat* x_item_mat);
CvMat* getItemSw(CvMat* x_item_mat);
CvMat* getSw(std::vector<std::vector<std::string> > total_path_vector);
CvMat* getU(std::vector<std::vector<std::string>> total_path_vector);
CvMat* getSb(std::vector<std::vector<std::string> > total_path_vector);

bool getW(std::vector<std::vector<std::string> > total_path_vector,std::string path_to_w);

CvMat* getY(CvMat* x,CvMat* w);
double countEuclidean(CvMat* a,CvMat* b);
int chooseBest(std::vector<CvMat*> y_vector,CvMat* w,CvMat* x);

std::vector<std::vector<std::string> > initialTotalTrainPathVector();
std::vector<std::string> initialAllPath();
bool trainW(std::vector<std::vector<std::string> > total_path_vector,std::string path_to_w);
std::vector<CvMat*> initialYVector(std::vector<std::vector<std::string>> total_path_vector,CvMat*w);

void mainLoop();


//handler of the opencv unix bug.
static int quit_signal = 0;
static void quit_signal_handler(int signum)
{
	if(quit_signal != 0)
	{
		exit(0);
	}	
	quit_signal = 1;
}

#endif




