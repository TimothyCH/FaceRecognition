#include "face_reco.h"

static int showImage(IplImage* img)
{
	
		printf("imageheight %d\n",img->height);
		printf("imagwidth %d\n",img->width);
		cvNamedWindow("showImageWindow",CV_WINDOW_AUTOSIZE);
		cvShowImage("showImageWindow",img);
		cvWaitKey(0);
		//cvReleaseImage(&img);
		cvDestroyWindow("showImageWindow");
		return 0;
}

static int changeImageSize(std::string image_path,int size)
{
		IplImage *img=cvLoadImage(image_path.c_str());
		IplImage*ret=cvCreateImage(cvSize(size,size),img->depth,img->nChannels);
		cvResize(img,ret);
		cvSaveImage(image_path.c_str(),ret);
		cvReleaseImage(&img);
		cvReleaseImage(&ret);
		return 0;
}

static IplImage* cutImage(IplImage* src,CvRect*rect)
{
	cvSetImageROI(src,*rect);
	IplImage*dst=cvCreateImage(cvSize(rect->width,rect->height),src->depth,src->nChannels);
	cvCopy(src,dst,NULL);//bug may ecur when meet gray pic.
	cvResetImageROI(src);
	return dst;
}

static int addImageToImage(IplImage* src,IplImage* des)
{
	if(src->width>=des->width||src->height>=des->height)
	{
		throw "the size of src is bigger or the same with des.";
	}
	for(int i=0;i<src->height;i++)
	{
		uchar*src_ptr=(uchar*)(src->imageData+i*src->widthStep);
		uchar*des_ptr=(uchar*)(des->imageData+i*des->widthStep);
		for(int j=0;j<src->width;j++)
		{
			des_ptr[3*j+0]=src_ptr[3*j+0];
			des_ptr[3*j+1]=src_ptr[3*j+1];
			des_ptr[3*j+2]=src_ptr[3*j+2];
		}
	}
	return 0;
}

//used to add the class number to the image.
static void addNameToImage(int num,IplImage* img)
{
	std::stringstream ss;
	num=num/8+1;
	std::string number;
	ss<<num;
	ss>>number;
	ss.clear();
	std::string text="Class ";
	text=text+number;
	if(num==41){//number 41 is the number of zhujiao.
		text="ZhuJiao";
	}
	else if(num==42){//number 42 is the classification number of cheng.
		text="21421251_Cheng";
	}
	CvFont font;
	cvInitFont(&font,CV_FONT_HERSHEY_COMPLEX,1.0,1.0,0,2,8);
	cvPutText(img,text.c_str(),cvPoint(200,30),&font,CV_RGB(255,0,0));
	return;
}


static IplImage* getFace(IplImage* img,int src)
{
	const char *PATH_TO_CASCADE="haarcascade_frontalface_alt.xml";  
	CvHaarClassifierCascade* cascade=(CvHaarClassifierCascade*)cvLoad(PATH_TO_CASCADE,0,0,0);//using the classification trained by opencv.
	//allocate and clear storage space.
	CvMemStorage*storage=cvCreateMemStorage(0);
	cvClearMemStorage(storage);
	//showImage(img);
	CvSeq*objects=cvHaarDetectObjects(
		img,
		cascade,
		storage,
		1.1,
		2,
		CV_HAAR_FIND_BIGGEST_OBJECT,
		cvSize(30,30)
		);
	CvRect*rect=(CvRect*)cvGetSeqElem(objects,0);
	IplImage *face_img=NULL;
	if(!rect)
	{//no face detected.
		if(src == 1)//picture from sample.
		{
			std::cout<<"No face detected,the origin image is used.";	
			face_img = img;
		}
		else//picture from camera.
		{
			return NULL;
		}
	}
	else
	{//detect face,cut it out.
		face_img=cutImage(img,rect);
		if(src == 2)//picture from camera.
		{
			CvScalar color={{0,255,255}};
			cvRectangle(img,cvPoint(rect->x,rect->y),cvPoint(rect->x+rect->width,rect->y+rect->height),color);
		}
	}
	//in need of Equalization,we need to change it to gray pic if it's not gray.
	IplImage*gray=cvCreateImage(cvSize(face_img->width,face_img->height),face_img->depth,1);
	if(face_img->nChannels==1)
	{
		gray=face_img;
	}
	else
	{
		cvCvtColor(face_img,gray,CV_BGR2GRAY);
	}
	cvEqualizeHist(gray,gray);
	IplImage*ret=cvCreateImage(cvSize(32,32),gray->depth,gray->nChannels);//resize
	cvResize(gray,ret);
	//showImage(ret);
	cvReleaseImage(&gray);
	cvReleaseImage(&face_img);
	return ret;
}

static bool isSymmetric(cv::InputArray src)
{
    Mat _src = src.getMat();
    if(_src.cols != _src.rows)
        return false;
    for (int i = 0; i < _src.rows; i++) {
        for (int j = 0; j < _src.cols; j++) {
            double a = _src.at<double> (i, j);
            double b = _src.at<double> (j, i);
            if (a != b) {
                return false;
            }
        }
    }
    return true;
}


