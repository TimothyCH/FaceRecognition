face_reco : face_reco.cpp face_reco.h eigen_decom.h main.cpp
	g++ -std=c++11 `pkg-config opencv --cflags` face_reco.cpp main.cpp -o face_reco `pkg-config opencv --libs`
