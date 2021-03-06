#include "face_reco.h"

int showImage(IplImage* img)
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

int changeImageSize(std::string image_path,int size)
{
		IplImage *img=cvLoadImage(image_path.c_str());
		IplImage*ret=cvCreateImage(cvSize(size,size),img->depth,img->nChannels);
		cvResize(img,ret);
		cvSaveImage(image_path.c_str(),ret);
		cvReleaseImage(&img);
		cvReleaseImage(&ret);
		return 0;
}

IplImage* cutImage(IplImage* src,CvRect*rect)
{
	cvSetImageROI(src,*rect);
	IplImage*dst=cvCreateImage(cvSize(rect->width,rect->height),src->depth,src->nChannels);
	cvCopy(src,dst,NULL);//bug may ecur when meet gray pic.
	cvResetImageROI(src);
	return dst;
}

int addImageToImage(IplImage* src,IplImage* des)
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
void addNameToImage(int num,IplImage* img)
{
	std::stringstream ss;
	num=num/SAMPLE_NUM+1;
	std::string number;
	ss<<num;
	ss>>number;
	ss.clear();
	std::string text="Class ";
	text=text+number;
	if(num==41){//number 41 is the number of zhujiao.
		text="ZhuJiao";
	}
	else if(num==CLASS_NUM){//last number is the number of class cheng.
		text="Cheng";
	}
	CvFont font;
	cvInitFont(&font,CV_FONT_HERSHEY_COMPLEX,1.0,1.0,0,2,8);
	cvPutText(img,text.c_str(),cvPoint(200,30),&font,CV_RGB(255,0,0));
	return;
}


IplImage* getFace(IplImage* img,int src)
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

CvMat* getItemX(IplImage* img)//count the X mat of pic.
{
	CvMat*mat_x=cvCreateMat(img->width*img->height,1,CV_64FC1);
	double*mat_ptr=(double*)(mat_x->data.ptr);
	for(int i=0;i<img->height;i++){
		//opencv may add some information char at the end of each row,so we need to get the point of the first element in the row.
		uchar*img_ptr=(uchar*)(img->imageData+i*img->widthStep);
		for(int j=0;j<img->width;j++){
			double num=(double)*img_ptr;
			*mat_ptr=num;
			img_ptr++;
			mat_ptr++;
		}
	}  
	return mat_x;
}

//input is all the picture path from same classification.
//output is a Mat constructed by the X mat of all the pics.
//the i's column is the X mat of i's picture.
CvMat* getX(std::vector<std::string> path_vec)
{
	//get the point to the first picture to read the size of picture.
	//the second arg is 0 means to transfer it to gray picture.
	//1 for RGB 0 for gray.
	IplImage *temp=cvLoadImage(path_vec[0].c_str(),0);
	CvMat*ret=cvCreateMat(temp->width*temp->height,path_vec.size(),CV_64FC1);//create the ret mat according to the size we get.
	int pic_width=temp->width;
	int pic_height=temp->height;
	cvReleaseImage(&temp);
	int count_row=0;
	double*ret_ptr=NULL;
	double*xmat_ptr=NULL;
	for(auto path=path_vec.begin();path!=path_vec.end();path++){
		ret_ptr=(double*)ret->data.ptr+count_row;
		IplImage *img=cvLoadImage((*path).c_str(),0);
		CvMat *xmat=getItemX(img);
		cvReleaseImage(&img);
		xmat_ptr=(double*)xmat->data.ptr;
		//copy x mat to the ret mat.
		for(int i=0;i<pic_width*pic_height;i++){
			double num=*xmat_ptr;
			*ret_ptr=num;
			ret_ptr=ret_ptr+ret->width;
			xmat_ptr++;
		} 
		count_row++;
	}
	return ret;
}

//input is the X mat get by getX()
//output is the mean vector U.
CvMat* getItemU(CvMat* x_item_mat)
{
	CvMat *u=cvCreateMat(x_item_mat->rows,1,x_item_mat->type);//¿¿U¿¿¿
	double*x_ptr=(double*)x_item_mat->data.ptr;
	double*u_ptr=(double*)u->data.ptr;
	double temp=0;
	for(int i=0;i<x_item_mat->rows;i++)
	{
		temp=0;
		for(int j=0;j<x_item_mat->cols;j++)
		{
			temp=temp+*x_ptr;
			x_ptr++;
		}
		temp=temp/x_item_mat->cols;
		*u_ptr=temp;
		u_ptr++;
	}
	return u;
}

//get the Sw mat of each classification,that is swi.
CvMat* getItemSw(CvMat* x_item_mat)
{
	CvMat*u=getItemU(x_item_mat);
	CvMat*swi=cvCreateMat(x_item_mat->rows,x_item_mat->rows,x_item_mat->type);//swi is an n*n mat,x is an n*1 mat.
	double*swi_ptr=(double*)swi->data.ptr;
	for(int i=0;i<swi->rows;i++){//initialize swi to 0.
		for(int j=0;j<swi->cols;j++){
			*swi_ptr=0;
			swi_ptr++;
		}
	}
	CvMat*x_col=cvCreateMat(x_item_mat->rows,1,x_item_mat->type);
	CvMat*temp_submat=cvCreateMat(x_item_mat->rows,x_item_mat->cols,x_item_mat->type);//a temp arg for cvGetCol behind,point to the corresponding column at the end.
	CvMat*temp_trans=cvCreateMat(1,x_item_mat->rows,x_item_mat->type);//store the trans of the Mat.
	CvMat*temp_multi=cvCreateMat(x_item_mat->rows,x_item_mat->rows,x_item_mat->type);//store the result of the multiplier of temp_trans and the origin mat.
	for(int i=0;i<x_item_mat->cols;i++){
		x_col=cvGetCol(x_item_mat,temp_submat,i);
		cvSub(x_col,u,x_col);
		cvTranspose(x_col,temp_trans);
		cvGEMM(x_col,temp_trans,1,NULL,0,temp_multi);
		cvAdd(swi,temp_multi,swi);
	}
	return swi;
}

//input is the total vector that is constructed by the vector of all the picture path in classfication.
CvMat* getSw(std::vector<std::vector<std::string> > total_path_vector)
{
	CvMat*temp_xmat=getX(total_path_vector[0]);//init sw with the picture of the first class.
	CvMat*sw=getItemSw(temp_xmat);
	//get all the people's swi,add to sw.
	for(auto path_vector=total_path_vector.begin()+1;path_vector!=total_path_vector.end();path_vector++){
		CvMat*xmat=getX(*path_vector);
		CvMat*swi=getItemSw(xmat);
		cvAdd(swi,sw,sw);
		//cout<<"finish count sw in "<<count++<<endl;
	}
	return sw;
}


//get the mean value of all.
CvMat*getU(std::vector<std::vector<std::string>> total_path_vector){
	CvMat*temp_xmat=getX(total_path_vector[0]);
	CvMat*total_u=getItemU(temp_xmat);//init total_u with the first clas.
	for(auto path_vector=total_path_vector.begin()+1;path_vector!=total_path_vector.end();path_vector++){//count the sum first.
		CvMat*xmat=getX(*path_vector);
		CvMat*u=getItemU(xmat);
		cvAdd(total_u,u,total_u);
	}
	double item_num=total_path_vector.size();
	double*total_u_ptr=(double*)total_u->data.ptr;
	for(int i=0;i<total_u->rows;i++){//divide number.
		for(int j=0;j<total_u->width;j++){
			*total_u_ptr=*total_u_ptr/item_num;
			total_u_ptr++;
		}
	}
	return total_u;
}

CvMat*getSb(std::vector<std::vector<std::string>> total_path_vector){
	CvMat*total_u=getU(total_path_vector);
	CvMat*sb=cvCreateMat(total_u->rows,total_u->rows,total_u->type);//n*n.
	double*sb_ptr=(double*)sb->data.ptr;
	for(int i=0;i<sb->rows;i++){//init to 0.
		for(int j=0;j<sb->width;j++){
			*sb_ptr=0;
			sb_ptr++;
		}
	}
	CvMat*temp_sub=cvCreateMat(total_u->rows,total_u->width,total_u->type);//temp_sub to store u - total_u.
	CvMat*temp_sub_trans=cvCreateMat(total_u->width,total_u->rows,total_u->type);//temp_sub_trans store trans of temp_sub.
	CvMat*temp_multi=cvCreateMat(total_u->rows,total_u->rows,total_u->type);//store the multi of temp_sub_trans and temp_sub.
	for(auto path_vector=total_path_vector.begin();path_vector!=total_path_vector.end();path_vector++){
		CvMat*xmat=getX(*path_vector);//get x mat of this class.
		CvMat*u=getItemU(xmat);//get mean mat.
		cvSub(u,total_u,temp_sub);//temp_sub = u - total_u.
		cvTranspose(temp_sub,temp_sub_trans);
		cvGEMM(temp_sub,temp_sub_trans,1,NULL,0,temp_multi);//multi
		double scale=path_vector->size();
		cvConvertScale(temp_multi,temp_multi,scale);//temp_multi = temp_multi * scale.
		cvAdd(sb,temp_multi,sb);//add to sb.
	}
	return sb;
}

//input train sample path.
//train w and store to path_to_w.
bool getW(std::vector<std::vector<std::string>> total_path_vector,std::string path_to_w){
	std::cout<<"counting sw and sb..."<<std::endl;
	CvMat*sw=getSw(total_path_vector);
	CvMat*sb=getSb(total_path_vector);
	std::cout<<"finish count sw and sb."<<std::endl;
	std::cout<<"the width of sw is "<<sw->width;
	std::cout<<",the height of sw is "<<sw->height<<"."<<std::endl;
	std::cout<<"the width of sb is "<<sb->width;
	std::cout<<",the height of sb is "<<sb->height<<"."<<std::endl;
	CvMat*sw_invert=cvCreateMat(sw->rows,sw->cols,sw->type);
	std::cout<<"counting the invert of sw..."<<std::endl;
	cvInvert(sw,sw_invert,CV_SVD);//get the invert of sw,CV_SVD will do pseudo insert.
	std::cout<<"counting the multiplier of sw_invert and sb..."<<std::endl;
	CvMat*multi=cvCreateMat(sw->rows,sw->cols,sw->type);
	cvGEMM(sw_invert,sb,1,NULL,0,multi);
	std::cout<<"the width of multiplier is "<<multi->width;
	std::cout<<",the height of multiplier is "<<multi->height<<"."<<std::endl;
	Mat temp_sw(multi);//trans CvMat to Mat to use EigenvalueDecomposition.
	std::cout<<"counting eigen_values and eigen_vectors forming w..."<<std::endl;
	EigenvalueDecomposition eig(temp_sw);
	Mat sw_eigen_value=eig.eigenvalues();//eigen_value is listed from small to big.
	Mat sw_eigen_vector=eig.eigenvectors();
	CvMat ret=sw_eigen_vector;//size of ret 1024*1024
	std::cout<<"the size of eigen_vector matrix is "<<ret.rows<<"*"<<ret.cols<<"."<<std::endl;
	CvMat temp=cvMat(ret.rows,ret.cols,ret.type);
	CvMat w=cvMat(ret.rows,200,ret.type);
	w=*(cvGetCols(&ret,&temp,0,200));//get the first 200 column to be w.
	std::cout<<"pick the first 200 cols to form w."<<std::endl;
	cvSave(path_to_w.c_str(),&w);
	std::cout<<"store w successfully to "<<path_to_w<<std::endl;
	return true;
}

//get the transposition of w and multied x to get y.
CvMat* getY(CvMat* x,CvMat* w)
{
	CvMat*w_trans=cvCreateMat(w->cols,w->rows,w->type);
	cvTranspose(w,w_trans);
	CvMat*y=cvCreateMat(w_trans->rows,x->cols,x->type);
	cvGEMM(w_trans,x,1,NULL,0,y);
	cvReleaseMat(&w_trans);
	return y;
}

//count the square of the euclidean distance between two vector.
double countEuclidean(CvMat*a,CvMat*b){
	if(a->cols!=b->cols||a->rows!=b->rows){
		throw "the two matrix is not of the same size.";
	}
	else{
		double ret=0;
		for(int i=0;i<a->rows;i++){
			for(int j=0;j<a->cols;j++){
				ret=ret+(cvmGet(a,i,j)-cvmGet(b,i,j))*(cvmGet(a,i,j)-cvmGet(b,i,j));
			}
		}
		return ret;
	}
}

int chooseBest(std::vector<CvMat*>y_vector,CvMat* w,CvMat* x)
{
	CvMat*y_new=getY(x,w);
	int best=-1;
	double smallest_num=10000000000;//init to the biggest.
	double temp;
	for(int i=0;i<(int)y_vector.size();i++){
		temp=countEuclidean(y_new,y_vector[i]);
		if(temp<=smallest_num){
			smallest_num=temp;
			best=i;
		}
	}
	//test
	std::cout<<"smallest num :"<<smallest_num<<std::endl;

	if(smallest_num < CRITICAL_NUM)
	{
		return best;
	}
	return -1;
}

//add all train sample path to total_path_vector.
std::vector<std::vector<std::string> > initialTotalTrainPathVector()
{
	std::vector<std::string> path_vector;
	std::vector<std::vector<std::string>> total_path_vector;
	std::string base_path=SAMPLE_BASE_PATH;
	std::stringstream ss;
	std::string numi;
	std::string numj;
	std::string path;
	total_path_vector.clear();
	for(int i=1;i<=CLASS_NUM;i++){
		path_vector.clear();
		for(int j=1;j<=SAMPLE_NUM;j++){
			ss<<i;
			ss>>numi;
			ss.clear();
			ss<<j;
			ss>>numj;
			ss.clear();
			path=base_path+numi+"/"+numj+".pgm";
			path_vector.push_back(path);
		}
		total_path_vector.push_back(path_vector);
	}
	return total_path_vector;
}

//return all the pic of all class.
std::vector<std::string> initialAllPath()
{
	std::vector<std::string> all_path;
	std::vector<std::vector<std::string> > total_path_vector=initialTotalTrainPathVector();
	for(int i=0;i<(int)total_path_vector.size();i++){
		for(int j=0;j<(int)total_path_vector[i].size();j++){
			all_path.push_back(total_path_vector[i][j]);
		}
	}
	return all_path;
}

bool trainW(std::vector<std::vector<std::string> > total_path_vector,std::string path_to_w){
		getW(total_path_vector,path_to_w);
		return true;
}

std::vector<CvMat*> initialYVector(std::vector<std::vector<std::string> > total_path_vector,CvMat*w){
	std::vector<CvMat*> y_vector;
	for(int i=0;i<(int)total_path_vector.size();i++){
		for(int j=0;j<(int)total_path_vector[i].size();j++){
			IplImage*temp=cvLoadImage(total_path_vector[i][j].c_str(),0);
			CvMat*x=getItemX(temp);
			CvMat*y=getY(x,w);
			y_vector.push_back(y);
			cvReleaseImage(&temp);
			cvReleaseMat(&x);
		}
	}
	return y_vector;
}

void mainLoop(){
	std::string path_to_w = PATH_TO_W;
	CvMat*w=(CvMat*)cvLoad(path_to_w.c_str());//read w from file.
	std::cout<<"load w matrix successfully."<<std::endl;
	CvMat*sub=cvCreateMat(w->rows,w->cols,w->type);
	w=cvGetCols(w,sub,0,35);//pick the first 35 column as the new w.
	std::cout<<"change w matrix to 35 cols."<<std::endl;
	CvCapture*capture=cvCreateCameraCapture(-1);//arg is the number of camera,start from 0,-1 for random pick.
	IplImage*frame=NULL;
	IplImage*face=NULL;
	cvNamedWindow("ShowImage",CV_WINDOW_AUTOSIZE);
	std::vector<std::vector<std::string>> total_path_vector=initialTotalTrainPathVector();
	std::vector<std::string> all_path=initialAllPath();
	std::vector<CvMat*> y_vector=initialYVector(total_path_vector,w);
	std::cout<<"successfully initiate y_vector."<<std::endl;
	std::cout<<"starting camera..."<<std::endl;
	int num;
	char u;
	while(1){
		frame=cvQueryFrame(capture);
		if(!frame) break;

#ifdef __UNIX__
		if(quit_signal == 1)//handler unix opencv bug.
		{
			exit(0);
		}
#endif

		face=getFace(frame,2);
		if(face!=NULL){
			CvMat*x=getItemX(face);
			num=chooseBest(y_vector,w,x);
			if(num!=-1){
				//test
				std::cout<<"num is:"<<num<<std::endl;
				IplImage *new_face=cvLoadImage(all_path[num].c_str());//attention the picture load here will be added to the frame,so load as RGB.
				addImageToImage(new_face,frame);
				addNameToImage(num,frame);
				cvShowImage("ShowImage",frame);
				u=cvWaitKey(0);
				if(u==27){
					break;
				}
			}
			else{
				cvShowImage("ShowImage",frame);
				cvWaitKey(27);//add wait key incase of flush,too fast to see it.
			}
		}
		else{
			cvShowImage("ShowImage",frame);
			cvWaitKey(27);
		}
	}
	cvReleaseCapture(&capture);  
	cvDestroyWindow("ShowImage");  
}


