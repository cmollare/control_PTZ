#include "ControlPTZ.h"
#include "StreamRTSP.h"

#include <iostream>

#include <pthread.h>
#include <time.h>//Fps processing
#include <string>
#include <sstream>

typedef struct
{
	StreamRTSP *stream;
	cv::Mat imgBuf;
	pthread_mutex_t mutex_stock;
}SStream;

typedef struct
{
	Point_<int> point;
	ControlPTZ* ctrlPTZ;
}SMouseEvent;

static void* pthread_img(void* stream)
{
	SStream *str = (SStream *) stream;
	for(;;)
	{
	  pthread_mutex_lock(&(str->mutex_stock));
	  str->imgBuf=str->stream->grabFrame();
	  pthread_mutex_unlock(&(str->mutex_stock));
	}
	return NULL;
}

void on_mouse(int event, int x, int y, int flags, void* param)
{
	SMouseEvent *mouseEvent = (SMouseEvent *) param;
	switch(event)
	{
	  case CV_EVENT_LBUTTONDOWN:
	    mouseEvent->point.x = x;
	    mouseEvent->point.y = y;
	    mouseEvent->ctrlPTZ->HTTPRequestPTZPosRelative((x-704/2)/10, -(y-576/2)/10, 0);
	    cout << "x : " << mouseEvent->point.x << endl;
	    cout << "y : " << mouseEvent->point.y << endl;
	    break;

	  default :
	    break;
	}
	
}

int main()
{	
	SStream stream;
	stream.stream = new StreamRTSP("rtsp://root:axis0@axis-ptz2/axis-media/media.amp");
	//stream.stream = new StreamRTSP("http://root:axis0@axis-ptz2/axis-cgi/mjpg/video.cgi"); //openCV does not read motion jpeg ??? Oo
	

	
	ControlPTZ ctrlPTZ;
	
	///Font initialization///
	//CvFont *font;
	//cvInitFont(font, CV_FONT_HERSHEY_SIMPLEX, 1.0f, 1.0f);
	///End Font initialization///
	
	SMouseEvent mouseEvent;
	mouseEvent.ctrlPTZ = &ctrlPTZ;
	
	///Open the thread for RTSP streaming///
	pthread_t thread_img;
	pthread_create(&thread_img, NULL, pthread_img, &stream);
	///End thread openning///
	
	cv::namedWindow("img",0);
	cvSetMouseCallback("img", on_mouse, &mouseEvent);
	
	///Initialization time calculation///
	time_t start, end;
	double fps;
	int counter = 0;
	double sec;
	//Start the clock
	time(&start);
	///End Initialization time calculation///
	
	
	//loop variables
	cv::Mat imgBuf;
	bool continuer = true;
	int pan =0, tilt =0, zoom =0;
	int step=5, stepZoom=100;
	int lastMousePosX=0, lastMousePosY=0;
	
	cv::VideoWriter record;
	bool isRecording = false;
	int numRecord=0;
	//end loop variables
	
	while(continuer)
	{
		
		pthread_mutex_lock(&stream.mutex_stock);
		imgBuf=stream.imgBuf;
		pthread_mutex_unlock(&stream.mutex_stock);
		
		cv::imshow("img",imgBuf);
		
		
		///Fps processing///
      	  ostringstream oss;
      	  time(&end);
      	  ++counter;
      	  sec = difftime (end, start);
      	  fps = counter / sec;
      	  oss << fps << " fps";
      	  ///End Fps processing///*/
      	  
      	  if(isRecording)
      	  {
			  record << imgBuf;
		  }
      	  
      	short key = cv::waitKey(5);
		switch(key)
		{
			case -1: //nothing's pressed
				break;
			
			case -175: //left arrow
				pan-=step;
				cout << "pan : " << pan << endl;
				cout << "tilt : " << tilt << endl;
				cout << "zoom : " << zoom << endl;
				ctrlPTZ.HTTPRequestPTZ(pan, tilt, zoom);
				break;
				
			case -173: //right arrow
				pan+=step;
				cout << "pan : " << pan << endl;
				cout << "tilt : " << tilt << endl;
				cout << "zoom : " << zoom << endl;
				ctrlPTZ.HTTPRequestPTZ(pan, tilt, zoom);
				break;
				
			case -174: //up arrow
				tilt+=step;
				cout << "pan : " << pan << endl;
				cout << "tilt : " << tilt << endl;
				cout << "zoom : " << zoom << endl;
				ctrlPTZ.HTTPRequestPTZ(pan, tilt, zoom);
				break;
				
			case -172: //down arrow
				tilt-=step;
				cout << "pan : " << pan << endl;
				cout << "tilt : " << tilt << endl;
				cout << "zoom : " << zoom << endl;
				ctrlPTZ.HTTPRequestPTZ(pan, tilt, zoom);
				break;
				
			case 'a':
				zoom-=stepZoom;
				cout << "pan : " << pan << endl;
				cout << "tilt : " << tilt << endl;
				cout << "zoom : " << zoom << endl;
				ctrlPTZ.HTTPRequestPTZ(pan, tilt, zoom);
				break;
				
			case 'z':
				zoom+=stepZoom;
				cout << "pan : " << pan << endl;
				cout << "tilt : " << tilt << endl;
				cout << "zoom : " << zoom << endl;
				ctrlPTZ.HTTPRequestPTZ(pan, tilt, zoom);
				break;
				
			case 32: // space
				cout << "RAZ !!!" << endl;
				pan=tilt=zoom=0;
				ctrlPTZ.HTTPRequestPTZPosAbsolute(-31.8904, -20, 0);
				break;
				
			case 'r': //start/stop recording
				if(isRecording)
				{
					isRecording=false;
					cout << "Stop recording" << endl;
				}
				else
				{
					ostringstream ossRecord;
					ossRecord << "../output/record_" << numRecord++ << ".avi";
					record.open(ossRecord.str(), CV_FOURCC('M','J','P','G'), 25, cv::Size(704, 576));
					if(record.isOpened())
					{
						cout << "Start recording of " << ossRecord.str() << endl;
						isRecording = true;
					}
					else
					{
						cout << "Can't start recording \"" << ossRecord.str() << "\" ..." << endl;
					}
				}
				break;
				
			case 27:
				pthread_cancel(thread_img);
				pthread_join(thread_img, NULL);
				ctrlPTZ.HTTPRequestPTZPosAbsolute(-31.8904, -20, 0);
				delete stream.stream;
				continuer=false;
				cv::waitKey(100);
				break;
				
			default:
				cout << key << endl;
				break;
		}
	}
	
	return 0;
}
