#include "face_reco.h"

int main(int argc,char*argv[]){
	
	if(argc!=2){
		std::cout<<"Please enter 2 parameters."<<std::endl;
		return -1;
	}
	std::string choose=argv[1];
#ifdef __UNIX__
	signal(SIGINT,quit_signal_handler);
#endif
	if(choose=="train"){
		std::vector<std::vector<std::string> > total_path_vector=initialTotalTrainPathVector();
		trainW(total_path_vector,PATH_TO_W);
		mainLoop();
	}
	else if(choose=="start"){
		mainLoop();
	}
	return 0;
	
}
