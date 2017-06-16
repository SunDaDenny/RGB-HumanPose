#include "OpticalFlow_GPU.h"

OpticalFlow_GPU::VectorSystem::GridVectorData::GridVectorData(const int gridLength)
{
	this->gridLength = gridLength;
	this->vector = cv::Vec2f(0.0, 0.0);
	this->angleOfVector = 0;
	this->isValid = false;
}

OpticalFlow_GPU::VectorSystem::GridVectorData::GridVectorData()
{
	this->gridLength = 10;	// Default length of grid
	this->vector = cv::Vec2f(0.0, 0.0);
	this->angleOfVector = 0;
	this->isValid = false;
}

void OpticalFlow_GPU::VectorSystem::GridVectorData::pushData(const cv::Vec2f vector)
{
	this->vectorArray.push_back(vector);
}

void OpticalFlow_GPU::VectorSystem::GridVectorData::computeAverage()
{
	if (this->vectorArray.size() >= gridLength * gridLength * 0.3)
	{
		this->isValid = true;
		cv::Vec2f vectorSum = cv::Vec2f(0.0, 0.0);
		std::vector<cv::Vec2f>::iterator it = this->vectorArray.begin();
		std::vector<cv::Vec2f>::iterator end = this->vectorArray.end();
		for (; it != end; it++)
		{
			vectorSum += *it;
		}
		this->vector = vectorSum / (double)this->vectorArray.size();
	}
	else
	{
		this->isValid = false;
		this->vector = cv::Vec2f(0.0, 0.0);
	}
}

void OpticalFlow_GPU::VectorSystem::GridVectorData::computeMedian()
{
	if (this->vectorArray.size() >= gridLength * gridLength * 0.3)
	{
		this->isValid = true;
		std::sort(this->vectorArray.begin(), this->vectorArray.end(), VectorCompare);
		this->vector = this->vectorArray[this->vectorArray.size() / 2];
	}
	else
	{
		this->isValid = false;
		this->vector = cv::Vec2f(0.0, 0.0);
	}
}

void OpticalFlow_GPU::VectorSystem::GridVectorData::calculateAngleOfVector()
{
	this->angleOfVector = atan2f(this->vector[1], this->vector[0]);
}

void OpticalFlow_GPU::VectorSystem::GridVectorData::reset()
{
	this->vector = cv::Vec2f(0.0, 0.0);
	this->angleOfVector = 0.0;
	this->vectorArray.clear();
}



#ifndef DEPRECATE_SECTION
void OpticalFlow_GPU::VectorSystem::GridData::pushData(const double angle, cv::Vec3b color)
{
	this->angleSum += angle;
	this->colorSum += cv::Vec3d(color);
	this->angleArray.push_back(angle);
	this->validCount += 1;
}

void OpticalFlow_GPU::VectorSystem::GridData::calculateAverage()
{
	if (this->validCount >= GRID_LENGTH * GRID_LENGTH * 0.3)
	{
		this->isValid = true;
		this->angle = this->angleSum / this->validCount;
		this->color = this->colorSum / this->validCount;
	}
	else
	{
		this->isValid = false;
	}
}
void OpticalFlow_GPU::VectorSystem::GridData::calculateMedian()
{
	if (this->validCount >= GRID_LENGTH * GRID_LENGTH * 0.3)
	{
		this->isValid = true;
		std::sort(this->angleArray.begin(), this->angleArray.end());
		this->angle = this->angleArray[this->validCount / 2];
	}
	else
	{
		this->isValid = false;
	}
}

void OpticalFlow_GPU::VectorSystem::GridData::reset()
{
	this->angleArray.clear();
	this->angleSum = 0;
	this->validCount = 0;
	this->colorSum = { 0.0, 0.0, 0.0 };
	this->isValid = false;
}
#endif


OpticalFlow_GPU::VectorSystem::VectorSystem(int gridLength, double vectorThreshold)
{
	m_VectorThreshold = vectorThreshold;
	m_GridLength = gridLength;
	m_IsLockCreating = false;
}

OpticalFlow_GPU::VectorSystem::VectorSystem()
{
	m_VectorThreshold = 0.8;	// Default threshold of length of vector
	m_GridLength = 10;			// Default length of grid
	m_IsLockCreating = false;
}

void OpticalFlow_GPU::VectorSystem::lockCreating()
{
	m_IsLockCreating = true;
}

void OpticalFlow_GPU::VectorSystem::unlockCreating()
{
	m_IsLockCreating = false;
}

void OpticalFlow_GPU::VectorSystem::createVectorGrids(const cv::Mat vectorDataMap)
{
	if (!m_IsLockCreating)
	{
		m_InputDenseMap = vectorDataMap;
		m_GridWidth = vectorDataMap.cols / m_GridLength;
		m_GridHeight = vectorDataMap.rows / m_GridLength;
		cv::MatConstIterator_<cv::Vec3f> dataIt = vectorDataMap.begin<cv::Vec3f>();
		cv::MatConstIterator_<cv::Vec3f> dataEnd = vectorDataMap.end<cv::Vec3f>();
		std::vector<GridVectorData>::iterator gridIt = m_GridVectorArray.begin();
		std::vector<GridVectorData>::iterator gridEnd = m_GridVectorArray.end();

		if (m_GridVectorArray.size() != m_GridWidth * m_GridHeight)
		{
			m_GridVectorArray.clear();
			m_GridVectorArray.resize(m_GridWidth * m_GridHeight, GridVectorData(m_GridLength));
		}

		for (; gridIt != gridEnd; gridIt++)
		{
			gridIt->reset();
		}

		int x, y;
		double hue;
		cv::Vec2f vector;
		for (x = 0, y = 0; dataIt != dataEnd; dataIt++, x++)
		{
			if (x >= vectorDataMap.cols)
			{
				x = 0;
				y++;
			}
			vector = cv::Vec2f((*dataIt)[0], (*dataIt)[1]);
			if (cv::norm(vector) > m_VectorThreshold)
			{
				m_GridVectorArray[x / m_GridLength + y / m_GridLength * m_GridWidth].pushData(vector);
			}
		}

		gridIt = m_GridVectorArray.begin();
		gridEnd = m_GridVectorArray.end();
		for (; gridIt != gridEnd; gridIt++)
		{
			gridIt->computeMedian();
			gridIt->calculateAngleOfVector();	//Optional, this function can be ignored if you do not need to produce [Fixed Length Vector Grid]
		}
		lockCreating();
	}
}



void OpticalFlow_GPU::VectorSystem::drawVector(bool isFixedVectorLength /* = false */)
{
	const double PORTION_SCALE = 0.33;
	const double TAIL_RATIO = 0.33;
	const double HEAD_RATIO = 0.5;

	if (this->vectorMap.size() != m_InputDenseMap.size())
	{
		this->vectorMap = cv::Mat(m_InputDenseMap.size(), CV_8UC3, cv::Scalar(0, 0, 0));
	}
	else
	{
		this->vectorMap.setTo(cv::Scalar(0, 0, 0));
	}

	std::vector<GridVectorData>::iterator gridIt = m_GridVectorArray.begin();
	std::vector<GridVectorData>::iterator gridEnd = m_GridVectorArray.end();

	cv::Point center;
	int x, y;
	float vectorX, vectorY;

	for (x = 0, y = 0; gridIt != gridEnd; gridIt++, x++)
	{
		if (x >= m_GridWidth)
		{
			x = 0;
			y++;
		}

		center = cv::Point(x * m_GridLength + m_GridLength / 2, y * m_GridLength + m_GridLength / 2);

		if (gridIt->isValid)
		{
			vectorX = gridIt->vector[0];
			vectorY = gridIt->vector[1];
			if (isFixedVectorLength)
			{
				cv::arrowedLine(
					this->vectorMap,
					cv::Point(center.x - m_GridLength * TAIL_RATIO * cos(gridIt->angleOfVector), center.y + m_GridLength * TAIL_RATIO * sin(gridIt->angleOfVector)),
					cv::Point(center.x + m_GridLength * HEAD_RATIO * cos(gridIt->angleOfVector), center.y - m_GridLength *HEAD_RATIO * sin(gridIt->angleOfVector)),
					cv::Scalar(0, 0, 255),
					1,
					8,
					0,
					0.3);
			}
			else
			{
				cv::arrowedLine(
					this->vectorMap,
					cv::Point(center.x - vectorX * (1 - PORTION_SCALE), center.y + vectorY * (1 - PORTION_SCALE)),
					cv::Point(center.x + vectorX * PORTION_SCALE, center.y - vectorY * PORTION_SCALE),
					cv::Scalar(0, 0, 255),
					1,
					8,
					0,
					0.3);
			}
		}
	}
}


void OpticalFlow_GPU::VectorSystem::produceVectorData(bool isFixedVectorLength /*= false*/)
{
	std::vector<GridVectorData>::iterator gridIt = m_GridVectorArray.begin();
	std::vector<GridVectorData>::iterator gridEnd = m_GridVectorArray.end();

	this->vectorData.clear();
	for (; gridIt != gridEnd; gridIt++)
	{
		if (isFixedVectorLength)
		{
			this->vectorData.push_back(cv::Vec2f(cos(gridIt->angleOfVector), sin(gridIt->angleOfVector)));
		}
		else
		{
			this->vectorData.push_back(gridIt->vector);

		}
	}
}

#ifndef DEPRECATE_SECTION
void OpticalFlow_GPU::VectorSystem::calculateHistogram()
{
	const int HALF_Of_KERNEL_BORDER = 1;
	const int ANGLE_TOLERANCE = 10;
	m_VectorHistogram.fill(0);
	//=========================Using index(slower in computation) to implement a 2D convolution scan in calculating vector histogram in order to add the space factor=========================
	int coordX, coordY;
	int centerAngle;
	for (int y = 0; y < m_GridHeight; y++)
	{
		for (int x = 0; x < m_GridWidth; x++)
		{
			if (m_GridArray[x + y * m_GridWidth].isValid)
			{
				for (int ky = -HALF_Of_KERNEL_BORDER; ky <= HALF_Of_KERNEL_BORDER; ky++)
				{
					for (int kx = -HALF_Of_KERNEL_BORDER; kx <= HALF_Of_KERNEL_BORDER; kx++)
					{
						coordX = x + kx;
						coordY = y + ky;

						if (coordX >= 0 && coordX < m_GridWidth && coordY >= 0 && coordY < m_GridHeight)
						{
							centerAngle = m_GridArray[x + y * m_GridWidth].angle;
							if (abs(m_GridArray[coordX + coordY * m_GridWidth].angle - centerAngle) <= ANGLE_TOLERANCE)
							{
								m_VectorHistogram[(int)((centerAngle - 0.0001) * NUMBER_OF_ANGLE / 360.0)] += 2;
							}
							else
							{
								m_VectorHistogram[(int)((centerAngle - 0.0001) * NUMBER_OF_ANGLE / 360.0)]--;
							}
						}



					}
				}
			}
		}
	}



	//=========================Using index(slower in computation) to implement a 2D convolution scan in calculating vector histogram in order to add the space factor=========================



	//=========================Using iterator to implement a simple scan in calculating vector histogram=========================
	//std::vector<GridData>::iterator gridIt = m_GridArray.begin();
	//std::vector<GridData>::iterator gridEnd = m_GridArray.end();
	//int x, y;
	//for (x = 0, y = 0; gridIt != gridEnd; gridIt++, x++)
	//{
	//	if (gridIt->isValid)
	//	{
	//		m_VectorHistogram[(int)((gridIt->angle - 0.0001) * NUMBER_OF_ANGLE / 360.0)]++;
	//	}
	//}
	//=========================Using iterator to implement a simple scan in calculating vector histogram=========================
}

void OpticalFlow_GPU::VectorSystem::drawHistogram()
{
	const double HEIGHT_SCALE = 0.3;
	int maxHeight = m_GridWidth * m_GridHeight * HEIGHT_SCALE;
	if (m_OriginalHistImage.size() != cv::Size(NUMBER_OF_ANGLE, maxHeight))
	{
		m_OriginalHistImage = cv::Mat(maxHeight, NUMBER_OF_ANGLE, CV_8UC3, cv::Scalar(0, 0, 0));
	}
	else
	{
		m_OriginalHistImage.setTo(cv::Scalar(0, 0, 0));
	}
	std::array<int, NUMBER_OF_ANGLE>::iterator histIt = m_VectorHistogram.begin();
	std::array<int, NUMBER_OF_ANGLE>::iterator histEnd = m_VectorHistogram.end();
	int x;
	for (x = 0; histIt != histEnd; x++, histIt++)
	{
		cv::line(m_OriginalHistImage, cv::Point(x, 0), cv::Point(x, *histIt), cv::Scalar(0, 0, 255));
	}
	cv::resize(m_OriginalHistImage, this->histImage, m_OriginalHistImage.size() * 3);
}

void OpticalFlow_GPU::VectorSystem::filterVector()
{
	const int HALF_Of_KERNEL_BORDER = 2;
	const int ANGLE_TOLERANCE = 20;
	const int VALID_THRESHOLD = (2 * HALF_Of_KERNEL_BORDER + 1) * (2 * HALF_Of_KERNEL_BORDER + 1) * 0.5;
	int coordX, coordY;
	int centerAngle;
	int count;
	for (int y = 0; y < m_GridHeight; y++)
	{
		for (int x = 0; x < m_GridWidth; x++)
		{
			if (m_GridArray[x + y * m_GridWidth].isValid)
			{
				count = 0;
				for (int ky = -HALF_Of_KERNEL_BORDER; ky <= HALF_Of_KERNEL_BORDER; ky++)
				{
					for (int kx = -HALF_Of_KERNEL_BORDER; kx <= HALF_Of_KERNEL_BORDER; kx++)
					{
						coordX = x + kx;
						coordY = y + ky;

						if (coordX >= 0 && coordX < m_GridWidth && coordY >= 0 && coordY < m_GridHeight)
						{
							centerAngle = m_GridArray[x + y * m_GridWidth].angle;
							if (abs(m_GridArray[coordX + coordY * m_GridWidth].angle - centerAngle) <= ANGLE_TOLERANCE * sqrt(kx * kx + ky * ky))
							{
								count++;
							}
						}
					}
				}
				if (count < VALID_THRESHOLD)
				{
					m_GridArray[x + y * m_GridWidth].isValid = false;
				}
			}
		}
	}
}
#endif


const GLfloat s_VertexCoordData[] = {
	-1.0f, -1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
	1.0f, 1.0f, 0.0f,
	-1.0f, -1.0f, 0.0f,
	1.0f, 1.0f, 0.0f,
	-1.0f, 1.0f, 0.0f,
};

const GLfloat s_TextureCoordData[] = {
	0.0f, 0.0f,
	1.0f, 0.0f,
	1.0f, 1.0f,
	0.0f, 0.0f,
	1.0f, 1.0f,
	0.0f, 1.0f,
};

void OpticalFlow_GPU::glfwErrorCallback(int error, const char* description)
{
	cout << "GLFW Error: " << description << endl;
	glfwTerminate();
	system("pause");
}

bool OpticalFlow_GPU::initialize(int imageWidth, int imageHeight, std::string shaderDirPath, int gridLength, int vectorThreshold)
{
	try
	{
		initializeGLFW();
		initializeGLEW();
	}
	catch (const OpenGLException& e)
	{
		cout << e.what() << endl;
		return false;
	}
	catch (const std::exception& e)
	{
		cout << e.what() << endl;
		return false;
	}

	m_Width = imageWidth;
	m_Height = imageHeight;
	m_PreFrameTextureID = createTextures(m_Width, m_Height, Color);
	m_curFrameTextureID = createTextures(m_Width, m_Height, Color);
	m_TextureId = createTextures(m_Width, m_Height, Float);
	m_VisualizeTextureId = createTextures(m_Width, m_Height, Color);
	m_FrameBuffer = createFrameBuffers(m_TextureId);
	m_VisualizeFrameBuffer = createFrameBuffers(m_VisualizeTextureId);
	m_ProgramId = createProgram(shaderDirPath + "\\optical_flow.vs", shaderDirPath + "\\optical_flow.fs");
	m_VisualizeProgramId = createProgram(shaderDirPath + "\\visualize_optical_flow.vs", shaderDirPath + "\\visualize_optical_flow.fs");
	initializeVertexBuffer();
	initializeUniformVariables();
	m_OpticalFlowData = cv::Mat(m_Height, m_Width, CV_32FC3);
	m_OpticalFlowColorMap = cv::Mat(m_Height, m_Width, CV_8UC3);
	m_OpticalFlowMask = cv::Mat(m_Height, m_Width, CV_8UC1);
	m_VectorThreshold = vectorThreshold;
	m_VectorSystem = VectorSystem(gridLength, vectorThreshold);

	glBindVertexArray(m_VertexArrayObject);
	return true;
}

void OpticalFlow_GPU::run(cv::Mat previousFrame, cv::Mat currentFrame)
{

	assert((!previousFrame.empty() && !currentFrame.empty()) && "Detected empty input.");
	assert((previousFrame.cols == m_Width && previousFrame.rows == m_Height) && "The size of input frame must be same with what you set in initialization.");
	assert((currentFrame.cols == m_Width && currentFrame.rows == m_Height) && "The size of input frame must be same with what you set in initialization.");

	glBindTexture(GL_TEXTURE_2D, m_PreFrameTextureID);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, previousFrame.cols, previousFrame.rows, GL_BGR, GL_UNSIGNED_BYTE, previousFrame.ptr());
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, m_curFrameTextureID);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, currentFrame.cols, currentFrame.rows, GL_BGR, GL_UNSIGNED_BYTE, currentFrame.ptr());
	glBindTexture(GL_TEXTURE_2D, 0);

	assert((checkGLError()) && "Occur error when creating input texture.");

	glUseProgram(m_ProgramId);
	glViewport(0, 0, m_Width, m_Height);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_PreFrameTextureID);
	glUniform1i(m_Location_PreviousFrame, 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_curFrameTextureID);
	glUniform1i(m_Location_CurrentFrame, 0);
	glUniform2f(m_Location_TextureSize, m_Width, m_Height);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glUseProgram(m_VisualizeProgramId);
	glViewport(0, 0, m_Width, m_Height);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER, m_VisualizeFrameBuffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_TextureId);
	glUniform1i(m_Location_ResultFrame, 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	readDataFromGL();

	m_VectorSystem.unlockCreating();
}

cv::Mat OpticalFlow_GPU::getVectorData()
{
	return m_OpticalFlowData.clone();
}

cv::Mat OpticalFlow_GPU::getColorVectorMap()
{
	return m_OpticalFlowColorMap.clone();
}

std::vector<cv::Vec2f> OpticalFlow_GPU::getProcessedVectorData()
{
	m_VectorSystem.createVectorGrids(m_OpticalFlowData);
	m_VectorSystem.produceVectorData(false);

	return m_VectorSystem.vectorData;
}

cv::Mat OpticalFlow_GPU::getProcessedVectorMap()
{

	m_VectorSystem.createVectorGrids(m_OpticalFlowData);
	m_VectorSystem.drawVector(false);

	return m_VectorSystem.vectorMap.clone();
}

std::vector<cv::Vec2f> OpticalFlow_GPU::getProcessedVectorDataWithFixedLength()
{
	m_VectorSystem.createVectorGrids(m_OpticalFlowData);
	m_VectorSystem.produceVectorData(true);

	return m_VectorSystem.vectorData;
}

cv::Mat OpticalFlow_GPU::getProcessedVectorMapWithFixedLength()
{
	m_VectorSystem.createVectorGrids(m_OpticalFlowData);
	m_VectorSystem.drawVector(true);

	return m_VectorSystem.vectorMap.clone();
}

cv::Mat OpticalFlow_GPU::getMaskMap()
{
	assert((m_OpticalFlowData.size() == m_OpticalFlowMask.size()) && "The size of data vector image and mask image must be same.");

	cv::MatIterator_<cv::Vec3f> vectorIt = m_OpticalFlowData.begin<cv::Vec3f>();
	cv::MatIterator_<cv::Vec3f> vectorEnd = m_OpticalFlowData.end<cv::Vec3f>();
	cv::MatIterator_<uchar> maskIt = m_OpticalFlowMask.begin<uchar>();

	cv::Vec2f vector;
	for (; vectorIt != vectorEnd; vectorIt++, maskIt++)
	{
		vector = cv::Vec2f((*vectorIt)[0], (*vectorIt)[1]);	// Reassigning the vector is to make it clear in 2D vector calculation
		if (cv::norm(vector) > m_VectorThreshold)
		{
			*maskIt = 255;
		}
		else
		{
			*maskIt = 0;
		}
	}

	return m_OpticalFlowMask.clone();
}

void OpticalFlow_GPU::initializeGLFW()
{
	// Set error callback function, in case a GLFW function fails, an error is reported to the GLFW error callback
	glfwSetErrorCallback(glfwErrorCallback);

	// Initialize GLFW
	if (!glfwInit())
	{
		throw OpenGLException("Fail to initialize GLFW");
	}

	// Set the OpenGL version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);

	// Make the window in this thread invisible
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);


	// Create a window which associates with GL context 
	m_Window = glfwCreateWindow(320, 240, "Optical Flow", NULL, NULL);

	// Check whether the GLFW window success to create or not
	if (!m_Window)
	{
		// Terminate the GLFW
		glfwTerminate();

		throw OpenGLException("Fail to initialize GLFW window(OpenGL context)");
	}

	// Set the specific context as current context
	glfwMakeContextCurrent(m_Window);
}

void OpticalFlow_GPU::initializeGLEW()
{
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err == GLEW_OK)
	{
		cout << "Driver supports OpenGL." << endl;
	}
	else
	{
		std::stringstream sstream;
		sstream << "Error Message: " << glewGetErrorString(err);

		throw OpenGLException("Fail to initialize GLEW, " + sstream.str());
	}
	assert((checkGLError()) && "Occur error when initializing GLEW");
}

void OpticalFlow_GPU::initializeVertexBuffer()
{
	// Setup vertex coordinate buffer
	glGenBuffers(1, &m_VertexCoordBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_VertexCoordBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(s_VertexCoordData), s_VertexCoordData, GL_STATIC_DRAW);

	// Setup texture coordinate buffer
	glGenBuffers(1, &m_TextureCoordBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_TextureCoordBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(s_TextureCoordData), s_TextureCoordData, GL_STATIC_DRAW);

	// Setup vertex array object which can save all states switches and the ways about how to use data saved in GPU buffer
	glGenVertexArrays(1, &m_VertexArrayObject);
	glBindVertexArray(m_VertexArrayObject);

	glBindBuffer(GL_ARRAY_BUFFER, m_VertexCoordBuffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,					// Attribute, need to match the layout in shader      
		3,                  // Size              
		GL_FLOAT,           // Type              
		GL_FALSE,           // Whether to normalize or not              
		0,                  // Stride              
		(GLubyte*)NULL      // Array buffer offset              
	);

	glBindBuffer(GL_ARRAY_BUFFER, m_TextureCoordBuffer);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,					// Attribute, need to match the layout in shader      
		2,                  // Size              
		GL_FLOAT,           // Type              
		GL_FALSE,           // Whether to normalize or not              
		0,                  // Stride              
		(GLubyte*)NULL      // Array buffer offset              
	);

	assert((checkGLError()) && "Occur error when initializing vertex buffer and vertex array object.");

}

void OpticalFlow_GPU::initializeUniformVariables()
{
	m_Location_TextureSize = glGetUniformLocation(m_ProgramId, "textureSize");
	m_Location_CurrentFrame = glGetUniformLocation(m_ProgramId, "currentFrame");
	m_Location_PreviousFrame = glGetUniformLocation(m_ProgramId, "previousFrame");
	m_Location_Threshold = glGetUniformLocation(m_ProgramId, "threshold");

	m_Location_ResultFrame = glGetUniformLocation(m_VisualizeProgramId, "resultFrame");
}

GLuint OpticalFlow_GPU::createTextures(int width, int height, TextureType type)
{

	assert((width > 0 || height > 0) && "The width or height should be greater than zero.");

	GLuint textureId;

	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	switch (type)
	{
	case OpticalFlow_GPU::Color:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		break;
	case OpticalFlow_GPU::Gray:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
		break;
	case OpticalFlow_GPU::Float:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
		break;
	default:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		break;
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	assert((checkGLError()) && "Occur error when creating texture.");

	return textureId;
}

GLuint OpticalFlow_GPU::createFrameBuffers(GLuint textureId)
{
	assert((textureId > 0) && "Texture ID should be greater than zero, value of zero means an empty texture object.");

	GLuint frameBuffer;
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	assert((status == GL_FRAMEBUFFER_COMPLETE) && "Fail to bind texture to frame buffer, check the setting of frame buffer.");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	assert((checkGLError()) && "Occur error when creating frame buffer.");

	return frameBuffer;
}

GLuint OpticalFlow_GPU::createProgram(std::string vertexShaderPath, std::string fragmentShaderPath)
{
	assert((vertexShaderPath != "" && fragmentShaderPath != "") && "The path of shader files should not be empty.");

	GLuint programId;
	programId = loadShaders(vertexShaderPath, fragmentShaderPath);
	return programId;
}

GLuint OpticalFlow_GPU::loadShaders(std::string vertexShaderPath, std::string fragmentShaderPath)
{
	std::string shaderCode;
	std::ifstream codeStream;
	GLint compileResult, lengthOfResult;
	std::string paths[2];
	paths[0] = vertexShaderPath;
	paths[1] = fragmentShaderPath;
	GLuint shaderIDs[2];
	shaderIDs[0] = glCreateShader(GL_VERTEX_SHADER);
	shaderIDs[1] = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint programID = glCreateProgram();
	for (int i = 0; i < 2; i++)
	{

		shaderCode = "";
		codeStream.open(paths[i], std::ios::in);


		if (codeStream.is_open() == true)
		{
			std::string tempStr = "";
			while (getline(codeStream, tempStr))
				shaderCode += tempStr + "\n";
			codeStream.close();
		}
		else
		{
			throw std::invalid_argument("Invalid path of shader files, the program cannot find shader files.");
		}
		const char* tempStrPointer = shaderCode.c_str();
		glShaderSource(shaderIDs[i], 1, &tempStrPointer, NULL);
		glCompileShader(shaderIDs[i]);
		glGetShaderiv(shaderIDs[i], GL_COMPILE_STATUS, &compileResult);
		glGetShaderiv(shaderIDs[i], GL_INFO_LOG_LENGTH, &lengthOfResult);
		if (lengthOfResult > 1)
		{
			char* logInfo = new char[lengthOfResult + 1];
			glGetShaderInfoLog(shaderIDs[i], lengthOfResult, NULL, logInfo);
			std::stringstream sstream;
			sstream << "Shader from: " << paths[i] << endl << "Shader Info : " << logInfo << endl;
			cout << sstream.str() << endl;
			delete[] logInfo;
			return 0;
		}
		else
		{
			std::stringstream sstream;
			sstream << "Shader from: " << paths[i] << endl << "Shader Info : Successful" << endl << endl << endl;
			cout << sstream.str() << endl;
		}
		glAttachShader(programID, shaderIDs[i]);
	}
	glLinkProgram(programID);
	for (int i = 0; i < 2; i++)
	{
		glDetachShader(programID, shaderIDs[i]);
		glDeleteShader(shaderIDs[i]);
	}
	return programID;
}

void OpticalFlow_GPU::readDataFromGL()
{
	glPixelStorei(GL_PACK_ROW_LENGTH, m_Width);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffer);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(0, 0, m_Width, m_Height, GL_RGB, GL_FLOAT, m_OpticalFlowData.ptr<cv::Vec3f>());
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glPixelStorei(GL_PACK_ROW_LENGTH, 0);


	glPixelStorei(GL_PACK_ROW_LENGTH, m_Width);
	glBindFramebuffer(GL_FRAMEBUFFER, m_VisualizeFrameBuffer);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(0, 0, m_Width, m_Height, GL_BGR, GL_UNSIGNED_BYTE, m_OpticalFlowColorMap.ptr());
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glPixelStorei(GL_PACK_ROW_LENGTH, 0);

	assert((checkGLError()) && "Occur error in reading data from OpenGL.");
}

bool OpticalFlow_GPU::checkGLError()
{
	GLenum errorCode = glGetError();
	if (errorCode == GL_NO_ERROR)
	{
		return true;
	}
	else
	{
		switch (errorCode)
		{
		case GL_INVALID_ENUM:
			cout << "Invalid enumeration. An unacceptable value is specified for an enumerated argument. The offending command is ignored and has no other side effect than to set the error flag." << endl;
			break;
		case GL_INVALID_VALUE:
			cout << "Invalid value. A numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag." << endl;
			break;
		case GL_INVALID_OPERATION:
			cout << "Invalid operation. The specified operation is not allowed in the current state. The offending command is ignored and has no other side effect than to set the error flag." << endl;
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			cout << "Invalid framebuffer operation. The framebuffer object is not complete. The offending command is ignored and has no other side effect than to set the error flag." << endl;
			break;
		case GL_OUT_OF_MEMORY:
			cout << "Out of memory. There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded." << endl;
			break;
		case GL_STACK_UNDERFLOW:
			cout << "Stack underflow. An attempt has been made to perform an operation that would cause an internal stack to underflow." << endl;
			break;
		case GL_STACK_OVERFLOW:
			cout << "Stack overflow. An attempt has been made to perform an operation that would cause an internal stack to overflow." << endl;
			break;
		default:
			break;
		}
		return false;
	}

}
