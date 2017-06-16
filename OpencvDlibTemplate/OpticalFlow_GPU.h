#pragma once
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <assert.h>
#include <stdexcept>
#include <exception>
#include <array>

#include <fstream>
#include <sstream>
#include <iostream>

#define DEPRECATE_SECTION

using std::cout;
using std::cerr;
using std::endl;



class OpticalFlow_GPU
{
public:

#ifndef DEPRECATE_SECTION
	static const int NUMBER_OF_ANGLE = 45;
#endif

	class VectorSystem
	{
	public:

		cv::Mat vectorMap;
		std::vector<cv::Vec2f> vectorData;
#ifndef DEPRECATE_SECTION
		cv::Mat histImage;
#endif
		VectorSystem();
		VectorSystem(int gridLength, double vectorThreshold);

		/**
		* @fn	void VectorSystem::lockCreating();
		*
		* @brief	Lock to run this function in order to avoid running this function repeatedly in the same round.
		*/

		void lockCreating();

		/**
		* @fn	void VectorSystem::unlockCreating();
		*
		* @brief	Same meaning as lockCreating() has, please refer to lockCreating().
		*/

		void unlockCreating();

		/**
		* @fn	void VectorSystem::createVectorGrids(const cv::Mat vectorDataMap);
		*
		* @brief	Creates vector grids.
		*
		* @param	vectorDataMap	The vector data map.
		*/

		void createVectorGrids(const cv::Mat vectorDataMap);

		/**
		* @fn	void VectorSystem::drawVector(bool isFixedVectorLength = false);
		*
		* @brief	Draw vector.
		*
		* @param	isFixedVectorLength	(Optional) Determine which type of vector will be produced,
		* 								false if you want to produce the various length vectors,
		* 								true if you want to produce the same length vectors which only have angle information.
		*/

		void drawVector(bool isFixedVectorLength = false);

		/**
		* @fn	void VectorSystem::produceVectorData(bool isFixedVectorLength = false);
		*
		* @brief	Produce vector data.
		*
		* @param	isFixedVectorLength	(Optional) Determine which type of vector will be produced,
		* 								false if you want to produce the various length vectors,
		* 								true if you want to produce the same length vectors which only have angle information.
		*/

		void produceVectorData(bool isFixedVectorLength = false);

#ifndef DEPRECATE_SECTION
		void calculateHistogram();

		void drawHistogram();

		void filterVector();
#endif

	private:

		/**
		* @struct	GridVectorData
		*
		* @brief	Grid vector data, which saves a certain amount of vectors bound by a window and process them with specific method, finally produce one vector to represent this grid vector.
		*/

		typedef struct GridVectorData
		{
			struct
			{
				bool operator()(cv::Vec2f vector1, cv::Vec2f vector2)
				{
					return cv::norm(vector1) < cv::norm(vector2);
				}
			}VectorCompare;

			int gridLength;
			cv::Vec2f vector;
			double angleOfVector;
			std::vector<cv::Vec2f> vectorArray;
			bool isValid = false;
			GridVectorData();
			GridVectorData(const int gridLength);
			void pushData(const cv::Vec2f vector);
			void computeAverage();
			void computeMedian();
			void calculateAngleOfVector();
			void reset();

		}GridVectorData;


#ifndef DEPRECATE_SECTION
		typedef struct GridData
		{
			static const int GRID_LENGTH = 10;
			double angle;
			cv::Vec3d color;
			std::vector<double> angleArray;
			double angleSum = 0;
			int validCount = 0;

			cv::Vec3d colorSum = { 0.0, 0.0, 0.0 }; // temp
			bool isValid = false;
			void pushData(const double angle, const cv::Vec3b color);
			void calculateAverage();
			void calculateMedian();
			void reset();

		}GridData;
#endif

		cv::Mat m_InputDenseMap;
		std::vector<GridVectorData> m_GridVectorArray;

#ifndef DEPRECATE_SECTION
		cv::Mat m_OriginalHistImage;
		std::vector<GridData> m_GridArray;
		std::array<int, NUMBER_OF_ANGLE> m_VectorHistogram;
#endif
		bool m_IsLockCreating;
		double m_VectorThreshold;
		int m_GridLength;
		int m_GridWidth;
		int m_GridHeight;


	};

	enum TextureType
	{
		Color,
		Gray,
		Float
	};

	static void glfwErrorCallback(int error, const char* description);

	/**
	* @fn	bool OpticalFlow_GPU::initialize(int imageWidth, int imageHeight, std::string shaderDirPath, int gridLength, int vectorThreshold);
	*
	* @brief	Initialize all needed objects and setup OpenGL environment, you need to provide some parameter to determine how the vector data will be produced.
	*
	* @param	imageWidth	   	Width of the image.
	* @param	imageHeight	   	Height of the image.
	* @param	shaderDirPath  	Path name of the shader directory.
	* @param	gridLength	   	Length of the grid.
	* @param	vectorThreshold	The vector threshold.
	*
	* @return	True if it succeeds, false if it fails.
	*/

	bool initialize(int imageWidth, int imageHeight, std::string shaderDirPath, int gridLength, int vectorThreshold);

	/**
	* @fn	void OpticalFlow_GPU::run(cv::Mat previousFrame, cv::Mat currentFrame);
	*
	* @brief	Run Optical Flow algorithm in OpenGL, it needs two input frames which are current
	* 			frame(N th frame) and previous frame(N-1 th frame).
	*
	* @param	previousFrame	The previous frame(N th frame).
	* @param	currentFrame 	The current frame(N-1 th frame).
	*/

	void run(cv::Mat previousFrame, cv::Mat currentFrame);

	/**
	* @fn	cv::Mat OpticalFlow_GPU::getVectorData();
	*
	* @brief	Get dense vector data, the number of vectors equals to the number of pixels which is usually 320 * 240.
	*
	* @return	The vector data whose type is CV_32FC3, the first channel represents horizontal vector, the second channel represents vertical vector, the third channel has no data.
	*/

	cv::Mat getVectorData();

	/**
	* @fn	cv::Mat OpticalFlow_GPU::getColorVectorMap();
	*
	* @brief	Get a vector map visualized by color space.
	*
	* @return	The color vector map.
	*/

	cv::Mat getColorVectorMap();

	/**
	* @fn	std::vector<cv::Vec2f> OpticalFlow_GPU::getProcessedVectorData();
	*
	* @brief	Get vector data which is processed by grid methodology, the number of vectors equals to the number of grids in a frame.
	*
	* @return	The processed vector data.
	*/

	std::vector<cv::Vec2f> getProcessedVectorData();

	/**
	* @fn	cv::Mat OpticalFlow_GPU::getProcessedVectorMap();
	*
	* @brief	Get a processed vector map visualized by several arrows and an arrow represent a vector in a grid.
	*
	* @return	The processed vector map.
	*/

	cv::Mat getProcessedVectorMap();

	/**
	* @fn	std::vector<cv::Vec2f> OpticalFlow_GPU::getProcessedVectorDataWithFixedLength();
	*
	* @brief	Get vector data which is processed by grid methodology and all vector have same length, so only angle information is significant, the number of vectors equals to the number of grids in a frame.
	*
	* @return	The processed vector data with fixed length.
	*/

	std::vector<cv::Vec2f> getProcessedVectorDataWithFixedLength();

	/**
	* @fn	cv::Mat OpticalFlow_GPU::getProcessedVectorMapWithFixedLength();
	*
	* @brief	Get a processed vector map visualized by several arrows and an arrow represent a vector in a grid and all vector have same length.
	*
	* @return	The processed vector map with fixed length.
	*/

	cv::Mat getProcessedVectorMapWithFixedLength();

	/**
	* @fn	cv::Mat OpticalFlow_GPU::getMaskMap();
	*
	* @brief	Get a mask map processed from optical flow result.
	*
	* @return	The mask map.
	*/

	cv::Mat getMaskMap();


private:
	void initializeGLFW();
	void initializeGLEW();
	void initializeVertexBuffer();
	void initializeUniformVariables();
	GLuint createTextures(int width, int height, TextureType type);
	GLuint createFrameBuffers(GLuint textureId);
	GLuint createProgram(std::string vertexShaderPath, std::string fragmentShaderPath);
	GLuint loadShaders(std::string vertexShaderPath, std::string fragmentShaderPath);
	void readDataFromGL();
	bool checkGLError();

	GLFWwindow* m_Window;

	VectorSystem m_VectorSystem;

	double m_VectorThreshold;

	int m_Width, m_Height;

	cv::Mat m_OpticalFlowData;
	cv::Mat m_OpticalFlowColorMap;
	cv::Mat m_OpticalFlowMask;

	GLuint m_TextureId;
	GLuint m_VisualizeTextureId;
	GLuint m_PreFrameTextureID;
	GLuint m_curFrameTextureID;

	GLuint m_FrameBuffer;
	GLuint m_VisualizeFrameBuffer;

	GLuint m_ProgramId;
	GLuint m_VisualizeProgramId;

	GLuint m_VertexArrayObject;

	GLint m_Location_CurrentFrame;
	GLint m_Location_PreviousFrame;
	GLint m_Location_TextureSize;
	GLint m_Location_Threshold;

	GLuint m_Location_ResultFrame;

	GLuint m_VertexCoordBuffer;
	GLuint m_TextureCoordBuffer;

};

class OpenGLException : public std::exception
{
public:
	OpenGLException(std::string desciption)
		:m_Description(desciption) {}

	virtual const char* what() const throw()
	{
		std::string errorInfo = "";
		errorInfo = "OpenGL occurs error.\nDescription: " + m_Description;
		return m_Description.c_str();
	}
private:
	std::string m_Description;
};



