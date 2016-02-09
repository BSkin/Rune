#ifndef OCEAN_H
#define OCEAN_H

#include "GameObject.h"
#include "Camera.h"
#include "FrameBuffer.h"
#include <math.h>

#define activeShader Shader::getActiveShader()
#define activeCamera Camera::getActiveCamera()

#define textureSize 512
#define numPasses (int)(logf(float(textureSize))/logf(2.0f))

class Ocean : public GameObject
{
public:
	Ocean();
	virtual ~Ocean();

	virtual int update(long elapsedTime);
	virtual int render(long totalElapsedTime);
	void updateFinalOceanHeights(Texture * boatHeightTexture, float boatTextureSize);

	float getHeight(float x, float z);
	Texture * getTestText() { return dataBuffer->getTexture(0); } // dataBuffer->getTexture(0);
	void turnWindRight(long elapsedTime);
	void turnWindLeft(long elapsedTime);
	void modAmplitude(float x) { amplitude += x; }
	void modWindSpeed(float x) { windSpeed += x; }
	void modLambda(float x) { lambda += x; }
	void modGridSize(float x) { gridSize += x; }
private:
	void updateH0Texture();
	void updateHtTexture();
	void updateHeights();
	void updateChoppyVectors();
	void FFT();
	void FFTpass(int pass);
	void heightScalePass();
	void choppyScalePass();

	Model * quad;
	Texture * gaussianNumbersTexture, * butterflyLookupTexture;
	Texture * FFTTestTexture, * iFFTTestTexture;
	FrameBuffer * h0Buffer;
	FrameBuffer * htBuffer;
	FrameBuffer * FFTPingPongBuffer; //FFTaBuffer, * FFTbBuffer;
	FrameBuffer * dataBuffer;
	bool FFTbool;
	Shader * h0Shader, * htShader, * choppyShader;
	Shader * FFTxShader, * FFTyShader;
	Shader * scaleShader, * finalHeightShader;
	Shader * quadShader;

	glm::vec2 windDirection;
	float amplitude, lambda, windSpeed, gridSize;
	float boatTextureSize;
	int oceanSeed;
	long oceanTime;
	float kConst;

	//#define A 1.0f
	//#define V 600.0f //wind speed
	//#define L V*V/9.81f
	//#define l L / 1000.0f
	#define L windSpeed*windSpeed/9.8f
	#define l L / 1000.0f
};

#endif