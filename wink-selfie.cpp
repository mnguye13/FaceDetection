//ECE 2574
//Programmer: MINH NGUYEN
//4/28//2017
//File: wink-selfie.cpp


#include "wink-selfie.h"

void main(int argc, char * argv[])
{
	try
	{
		ofstream out("detectionLog.txt");
		if (argc < 2)
		{
			throw exception("You must enter the input file name!");
		}

		int arg2length = (int)((string)argv[1]).length();

		int dirLength = 0; //Index of the null character in the Directory_location
		for (int i = 0; i < arg2length; i++)
		{
			if (argv[1][i] == '/')
			{
				argv[1][i] = '\\';
			}
			if (argv[1][i] == '\\')
			{
				bool DotCheck = false; // checking if there's a dot
				bool SlashCheck = false;
				for (int i = dirLength + 1; i < arg2length; i++)
				{
					if (DotCheck && (argv[1][i] == '\\' || argv[1][i] == '.'))
					{
						throw exception("Error! Bad Directory_location!");
					}
					else if (argv[1][i] == '\\')
					{
						SlashCheck = true;
					}
					else if (argv[1][i] == '.')
					{
						DotCheck = true;
					}
				}
				if (DotCheck)
				{
					break;
				}
			}
			dirLength++;
		}
		dirLength++;

		char * Directory_location = new char[dirLength + 1];
		if (Directory_location == NULL)
		{
			throw exception("Error! Failed to allocate memory for the Directory_location char array!");
		}
		if (dirLength != -1)
		{
			int i;
			for (i = 0; i < dirLength - 1; i++)
			{
				Directory_location[i] = argv[1][i];
			}
			Directory_location[i] = '\\';
			i++;
			Directory_location[i] = '\0';
		}

		string FileNameTest = "test.txt";
		char * CharTestFile = new char[dirLength + FileNameTest.length() + 1];
		if (CharTestFile == NULL)
		{
			throw exception("Error! Can't allocate any more memory!");
		}
		int i;
		for (i = 0; i < dirLength; i++)
		{
			CharTestFile[i] = Directory_location[i];
		}
		for (int start = i; i < dirLength + FileNameTest.length(); i++)
		{
			CharTestFile[i] = FileNameTest[i - start];
		}
		CharTestFile[i] = '\0';

		ofstream test(CharTestFile); //creates new Directory_location
		if (test.fail())
		{
			cerr << "The output file path " << argv[1] << " does not exist! Creating it!" << endl;
			_mkdir(Directory_location);
		}
		else
		{
			test.close();
			remove(CharTestFile);
		}

		VideoCapture CAP; //Object used to capture data
		if (argc > 2) // check if there is video file 
		{
			if (!CAP.open(argv[2]))
			{
				throw exception("Error! Video File cannot be opened or corrupted !!");
			}
		}
		else // use webcam
		{
			if (!CAP.open(0))
			{
				throw exception("Error! Webcam device cannot load!!");
			}
			//Set frame
			CAP.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
			CAP.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
		}

		if (!CAP.isOpened()) 
		{
			throw exception("Error! Video File cannot be opened or corrupted!!");
		}

		cv::CascadeClassifier face_cascade("haarcascade_frontalface_default.xml"); // face detection file
		cv::CascadeClassifier eyes_cascade("haarcascade_eye.xml"); // eye detection file

		if (face_cascade.empty())
		{
			throw exception("Error! Face detection file cannot load!");
		}
		if (eyes_cascade.empty())
		{
			throw exception("Error! Eye detection file cannot load!");
		}


		//processing the video
		int WinksDetection = 0;
		std::vector<cv::Rect> faces; //A list of detected faces
		std::vector<cv::Rect> eyes; //A list of detected eyes
		cv::Mat * frame = new Mat(); //Matrix to store the current frame of the webcam or video
		queue<Mat*> MemoryFrame;
		int FramesBlinks = 0;
		int FramesWinks = 0;
		list<WinkEyeDetection> Last5Winks;

		while (CAP.read(*frame))
		{
			if (*(frame->size.p) == 0) //reached the last frame of the video
			{
				throw exception("Error! Bad frame!");
			}

			if (MemoryFrame.size() >= FRAMES_TO_REMEMBER)
			{
				MemoryFrame.pop();
			}
			MemoryFrame.push(frame);
			cv::Mat * oldFrame = MemoryFrame.front();

			list<FaceRecognition> knownFaces;

			face_cascade.detectMultiScale(*frame, faces, 1.3, 5);
			for (int i = 0; i < faces.size(); i++)
			{
				rectangle(*frame, faces[i], Scalar(255, 0, 0), 1, 16, 0);
				FaceRecognition New_Face_Reg;
				New_Face_Reg.faceRectangle = &faces[i];
				New_Face_Reg.numberOfEyes = 0;
				New_Face_Reg.winkSide = NA;
				knownFaces.push_front(New_Face_Reg);
			}

			eyes_cascade.detectMultiScale(*frame, eyes, 1.3, 5);
			for (int i = 0; i < eyes.size(); i++)
			{
				cv::Rect eyesRect = eyes[i];
				for (int j = 0; j < faces.size(); j++)
				{
					cv::Rect faceRect = faces[j];
					if (eyesRect.x >= faceRect.x && eyesRect.x < faceRect.x + faceRect.width && eyesRect.y >= faceRect.y && eyesRect.y < faceRect.y + faceRect.width && eyesRect.y + eyesRect.height / 2 < faceRect.y + faceRect.height / 2)
					{
						list<FaceRecognition>::iterator it; //Iterator to find the face which corresponds to this eye
						for (it = knownFaces.begin(); it != knownFaces.end(); ++it)
						{
							if (it->faceRectangle == &faces[j])
							{
								it->numberOfEyes++;
								if (it->numberOfEyes > 2)
								{
									std::cout << "Detected " << it->numberOfEyes << " eyes! Unstable Face !" << endl;
								}
								else if (it->numberOfEyes == 1)
								{
									WinkEyeDetection side;
									if (eyesRect.x < faceRect.x + faceRect.width / 2)
									{
										side = LEFT; //left eye
									}
									else
									{
										side = RIGHT; //right eye
									}
									it->winkSide = side;
								}

								rectangle(*frame, eyesRect, Scalar(0, 255, 0), 1, LINE_AA, 0);
								break;
							}
						}
					}
				}
			}

			list<FaceRecognition>::iterator it;
			bool WinkFound = false;
			bool BlinkFound = false;
			for (it = knownFaces.begin(); it != knownFaces.end(); ++it)
			{
				if (it->numberOfEyes == 1)// check if there's a wink
				{
					FramesBlinks = 0;
					WinkFound = true;
					FramesWinks++;

					if (FramesWinks == FRAME_BLINK_DELAY + 1)
					{
						if (Last5Winks.size() >= 5)
						{
							Last5Winks.pop_back();
						}
						Last5Winks.push_front(it->winkSide);
						WinkEyeDetection s[5];
						list<WinkEyeDetection>::iterator it2;
						int index;
						for (it2 = Last5Winks.begin(), index = 0; it2 != Last5Winks.end() && index < 5; ++it2, ++index)
						{
							s[index] = *it2;
						}
						for (; index < 5; index++)
						{
							s[index] = NA;
						}
						//0 has the most recent wink
						if (s[0] == RIGHT && s[1] == RIGHT)// if detect 2 right winks 
						{
							cout << "Detected 2 right winks! Changed console background colors to Blue and Text to White !" << endl;
							out << "Detected 2 right winks! "<<endl;
							system("color 1f");
						}
						else if (s[0] == LEFT && s[1] == LEFT)// if detect 2 left winks
						{
							cout << "Detected 2 left winks! Changed console background colors to Red and Text to White !" << endl;
							out << "Detected 2 left winks! " << endl;
							system("color 4f");
						}

						WinksDetection++;
						std::cout << "Detected wink!" << endl;
						out << "Detected wink! Picture Captured" << endl;
						std::cout << "Picture Captured!" << endl;
						string prefix = "winkCaptured";
						string extension = ".png";
						ostringstream stream;
						stream << prefix;
						stream << WinksDetection;
						stream << extension;
						string whatToAppend = stream.str();

						int totalNameLength = dirLength + whatToAppend.length();

						char * outputFileName = new char[totalNameLength + 1];
						if (outputFileName == NULL)
						{
							throw exception("Error! Could not allocate memory for an output file name!");
						}
						int i;
						for (i = 0; i < dirLength; i++)
						{
							outputFileName[i] = Directory_location[i];
						}
						int start = i;
						for (; i < totalNameLength; i++)
						{
							outputFileName[i] = whatToAppend[i - start];
						}
						outputFileName[i] = '\0';

						cv::imwrite(outputFileName, *oldFrame);
						break;
					}
				}
				else if (WinksDetection > 0 && it->numberOfEyes == 0)
				{
					FramesWinks = 0;
					FramesBlinks++;
					BlinkFound = true;
					if (FramesBlinks >= FRAME_BLINK_DELAY + 1)// if detect both eyes shut
					{
						std::cout << "Detected both eyes shut! Stop Capturing !" << endl;
						out << "Done Capturing !" << endl;
						cout << "Program will now exit." << endl;
						cout << "Check output folder for viewing pictures " << endl;
						system("start notepad.exe detectionLog.txt");
						return;
					}
				}
			}
			if (!WinkFound)
			{
				FramesWinks = 0;
			}
			if (!BlinkFound)
			{
				FramesBlinks = 0;
			}

			cv::imshow("Image Capture", *frame);
			cv::waitKey(6);

			frame = new Mat();
		}
		out << "Done Capturing !" << endl;
		out.close();
		cout << "Program will now exit." << endl;
		cout << "Check output folder for viewing pictures " << endl;
		system("start notepad.exe detectionLog.txt");
		
	}
	catch (exception ex)
	{
		cerr << ex.what() << endl;
	}
	cout << "Program will now exit." << endl;
	cout << "Check output folder for viewing pictures " << endl;
	system("start notepad.exe detectionLog.txt");
}