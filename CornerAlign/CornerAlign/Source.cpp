/*
Bassam Arshad
0259149

CV - CSCI 6368
HW03

*/


#include <opencv2\opencv.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include "HarrisCorner.h"
#include "GFTT.h"

using namespace std;
using namespace cv;
void rotateImage(Mat &img_object, Mat&img_scene,float angle);
float getOverallDistance(vector<Point> newPointLocScene, vector<Point> newPointLocObj);

int main()
{

	Mat img_object = imread("fighter.jpg");
	Mat img_scene;// = imread("fighter_rotated1.jpg");

	namedWindow("Original Image", 0);
	imshow("Original Image", img_object); 

	float startingAngle = 60;
	//rotateImage(img_object,img_scene, startingAngle);
	
	float angle= startingAngle;

	while (angle<=360)
	{
		

		rotateImage(img_object, img_scene, angle);

		namedWindow("Rotated Image", 0);
		putText(img_scene, "Object Rotated By "+ to_string(angle)+" degrees", cvPoint(30, 30), FONT_HERSHEY_COMPLEX_SMALL, 1, Scalar(0, 0, 255), 1, CV_AA);
		imshow("Rotated Image", img_scene);
		
		


	cout << angle;

	// Get my Keypoints from the method of choice
	vector<KeyPoint> keypoints_object, keypoints_scene;

	//Uncomment the method you would use for extracting corners

	//Harris Corner
	//HarrisCorner harris;
	//keypoints_object=harris.getHarrisCorners(img_object, 180);
	//keypoints_scene = harris.getHarrisCorners(img_scene, 180);

	//GFTT Corners
	GFTT gfttcorner;
	keypoints_object = gfttcorner.getGFTTCorners(img_object, 25);
	keypoints_scene = gfttcorner.getGFTTCorners(img_scene, 25);


	////////////////////////

	vector<Point2f> CornerPtsObj, CornerPtsScene;
	KeyPoint::convert(keypoints_object, CornerPtsObj);
	KeyPoint::convert(keypoints_scene, CornerPtsScene);

	//Convert gftt points to <Point>
	vector<Point> CornerPtsObjP, CornerPtsSceneP;

	//Converting Point2f to Point
	cv::Mat(CornerPtsObj).convertTo(CornerPtsObjP, cv::Mat(CornerPtsObjP).type());
	cv::Mat(CornerPtsScene).convertTo(CornerPtsSceneP, cv::Mat(CornerPtsSceneP).type());


	Mat outImgObj = Mat(img_object.rows, img_object.cols, CV_8UC3, CV_RGB(255, 255, 255));
	Mat outImgScene = Mat(img_scene.rows, img_scene.cols, CV_8UC3, CV_RGB(255, 255, 255));

	//draw the corner points
	drawContours(outImgObj, Mat(CornerPtsObjP), -1, CV_RGB(0, 0, 255), 5);  // Blue Points
	drawContours(outImgScene, Mat(CornerPtsSceneP), -1, CV_RGB(0, 0, 0), 5); // Black Points

	//Get a Rectangle to bound them
	cv::Rect rObj = cv::boundingRect(CornerPtsObjP);
	rectangle(outImgObj, rObj, Scalar(138, 43, 226));

	cv::Rect rScene = cv::boundingRect(CornerPtsSceneP);
	rectangle(outImgScene, rScene, Scalar(138, 226, 43));

	//Mark the center of the rectangle - this is our reference
	// Center of the Rectangles
	Point rObjCenter = Point(rObj.x + rObj.width / 2, rObj.y + rObj.height / 2);
	Point rSceneCenter = Point(rScene.x + rScene.width / 2, rScene.y + rScene.height / 2);

	drawMarker(outImgObj, rObjCenter, CV_RGB(255, 0, 0), 0, 10, 3, 8);  //Red Marker
	drawMarker(outImgScene, rSceneCenter, CV_RGB(0, 255, 0), 0, 30, 1, 8); // Green Marker

	Mat newImgObj(outImgObj.rows * 2, outImgObj.cols * 2, CV_8UC3, CV_RGB(255, 255, 255));
	copyMakeBorder(outImgObj, newImgObj, outImgObj.rows / 2, outImgObj.rows / 2, outImgObj.cols / 2, outImgObj.cols / 2, 1);

	//New location of the corner points - got shifted by outImgObj.rows/2 and outImgObj.cols/2
	vector<Point> newPointLocObj, newPointLocScene;
	Point temp;
		//From object image
		for (int i = 0; i < CornerPtsObjP.size(); i++)
		{
			temp = CornerPtsObjP[i];
			newPointLocObj.push_back(Point(temp.x + outImgObj.cols / 2, temp.y + outImgObj.rows / 2));
			//drawMarker(newImgObj, Point(temp.x + outImgObj.cols / 2, temp.y + outImgObj.rows / 2), CV_RGB(0, 0, 0), 0, 20, 1, 8);
		}


	// Getting the rectangle centroid co-ordinate from this image  -->newImgObj
	
	//Rectangle center in expanded image
	Point newImgobjCenter = Point(outImgObj.cols / 2 + rObjCenter.x, outImgObj.rows / 2 + rObjCenter.y);
	//drawMarker(newImgObj, newImgobjCenter, Scalar(22, 143, 22), 0, 20, 2, 8);
	cout << "\n Centroid of Rectangle in newImageObj " << newImgobjCenter;
	
	//Create o/p image for the scene of the same size as newImgObj
	Mat newImgScene(newImgObj.rows, newImgObj.cols, CV_8UC3, CV_RGB(255, 255, 255));
	Mat sceneROI = outImgScene(rScene);
	
	//Making the sizes even
	if (sceneROI.rows % 2 != 0) { resize(sceneROI, sceneROI, Size(sceneROI.cols, sceneROI.rows + 1)); }
	if (sceneROI.cols % 2 != 0) { resize(sceneROI, sceneROI, Size(sceneROI.cols + 1, sceneROI.rows)); }

	//Shifting/Moving the SceneROI by a new location inorder to align the centroid of the SceneROI with the centroid of the ObjImage 
	sceneROI.copyTo(newImgScene.rowRange(newImgobjCenter.y - sceneROI.rows / 2, sceneROI.rows / 2 + newImgobjCenter.y).colRange(newImgobjCenter.x - sceneROI.cols / 2, sceneROI.cols / 2 + newImgobjCenter.x));

	//Getting the position of the corner points in newScene image
	Point temp1;
	for (int i = 0; i < CornerPtsSceneP.size(); i++)
	{
		temp1 = CornerPtsSceneP[i];
		//cout <<"\n" << temp1.x - rScene.x;
		newPointLocScene.push_back(Point(temp1.x - rScene.x + newImgobjCenter.x - rScene.width / 2, temp1.y - rScene.y + newImgobjCenter.y - rScene.height / 2));
		//drawMarker(newImgScene, Point( temp1.x - rScene.x  +  newImgobjCenter.x  - rScene.width/2, temp1.y -rScene.y + newImgobjCenter.y - rScene.height/2), CV_RGB(255, 0, 0), 0, 20, 1, 8);
	}
	
	cout << "\n Corner Points in Object  " << newPointLocObj.size();
	cout << " \n Corner Points in Scene  " << newPointLocScene.size();

	// Now we have the points from both the object and scene(rotated) images - , where we have aligned them by their centroid - by placing them in an equal sized images ...
	//Points are in newPointLocScene and newPointLocObj
	//We will conduct a brute force distance calc - by calculating the distance at multiple angles and then summing all the distances ... the overall least distance sum will be out our match position ..

	float overAllDist;
	
	overAllDist = getOverallDistance(newPointLocScene, newPointLocObj);

	cout << " \n OverAll Distance between the two objects  " << overAllDist;

	//vector<float> distances;
	//distances.push_back(overAllDist);

	


	Mat mergeImage;
	addWeighted(newImgObj, 0.5, newImgScene, 0.5, 0.0, mergeImage);
	
	namedWindow("Bounded Corners Object", 0);
	imshow("Bounded Corners Object", newImgObj);

	namedWindow("Bounded Corners Scene", 0);
	imshow("Bounded Corners Scene", newImgScene);

	namedWindow("Overlay Image (Objects and Scene) ", 0);
	putText(mergeImage, "Distance Score between the Corners (Object vs. Rotated ) " + to_string(overAllDist) , cvPoint(30, 30), FONT_HERSHEY_COMPLEX_SMALL, 0.8, Scalar(0, 0, 0), 1, CV_AA);
	imshow("Overlay Image (Objects and Scene) ", mergeImage);

	angle = angle + 10;
	waitKey();

}


	return 0;
}

void rotateImage(Mat &img_object,Mat&img_scene, float angle1)
{

	//Rotate My image by an angle
	double angle = angle1;

	// get rotation matrix for rotating the image around its center
	cv::Point2f center(img_object.cols / 2.0, img_object.rows / 2.0);
	cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);
	// determine bounding rectangle
	cv::Rect bbox = cv::RotatedRect(center, img_object.size(), angle).boundingRect();
	// adjust transformation matrix
	rot.at<double>(0, 2) += bbox.width / 2.0 - center.x;
	rot.at<double>(1, 2) += bbox.height / 2.0 - center.y;

	cv::warpAffine(img_object, img_scene, rot,
		bbox.size(),
		cv::INTER_LINEAR,
		cv::BORDER_CONSTANT,
		cv::Scalar(255, 255, 255));
	//resize(img_scene, img_scene, img_object.size(), 1.0, 1.0);
}

float getOverallDistance(vector<Point> newPointLocScene, vector<Point> newPointLocObj)
{
	float distance=0, currentLowest;
	float overallLowest = 0;

	for (int i = 0; i < newPointLocObj.size(); i++)
	{
		currentLowest = 10000000;
		for (int j = 0; j < newPointLocScene.size(); j++)
		{
			distance = norm(newPointLocObj[i] - newPointLocScene[j]);
			if (distance < currentLowest)
			{
				currentLowest = distance;
			}

		}

		overallLowest = overallLowest + currentLowest;
	}

	return overallLowest;
}
