#include "Ocean.h"

//string mPath, string tPath, string sPath, glm::vec3 position, glm::vec3 direction, glm::vec3 up
Ocean::Ocean() : GameObject("Models\\ocean.obj", "Textures\\error.tga", "Shaders\\Ocean Shaders\\ocean.glsl", glm::vec3(0,0,0), glm::vec3(0,0,-1), glm::vec3(0,1,0))
{
	windDirection = glm::normalize(glm::vec2(1,1));
	amplitude = 5.0f; // 2.0f;
	windSpeed = 600.0f;
	lambda = 1000.0f;
	gridSize = 512.0f;//2000.0f;
	oceanTime = 0;
	FFTbool = true;
	quad = getModel("Models\\quad.obj");

	//save seed
	oceanSeed = rand();
	grandSeed(oceanSeed);
	gaussianNumbersTexture = Texture::loadComplexGaussianNumbersTexture(textureSize, textureSize);

	butterflyLookupTexture = Texture::loadButterflyLookupTexture(textureSize, numPasses);
	
	FFTTestTexture = Texture::loadFFTTest(textureSize, textureSize);
	iFFTTestTexture = Texture::loadiFFTTest(textureSize, textureSize);

	h0Buffer = new FrameBuffer(textureSize, textureSize);
	h0Buffer->addDepthBuffer();
	h0Buffer->addColorFloatTexture();

	htBuffer = new FrameBuffer(textureSize, textureSize);
	htBuffer->addDepthBuffer();
	htBuffer->addColorFloatTexture();
		
	FFTPingPongBuffer = new FrameBuffer(textureSize, textureSize);
	FFTPingPongBuffer->addDepthBuffer();
	FFTPingPongBuffer->addColorFloatTexture();
	FFTPingPongBuffer->addColorFloatTexture();

	dataBuffer = new FrameBuffer(textureSize, textureSize);
	dataBuffer->addDepthBuffer();
	dataBuffer->addColorFloatTexture(); //heigth(2)
	dataBuffer->addColorFloatTexture(); //choppy vectors
	dataBuffer->addColorFloatTexture(); //heights - boat depths(r)

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	h0Shader = getShader("Shaders\\Ocean Shaders\\h0.glsl");
	htShader = getShader("Shaders\\Ocean Shaders\\ht.glsl");
	choppyShader = getShader("Shaders\\Ocean Shaders\\choppy.glsl");
	FFTxShader = getShader("Shaders\\Ocean Shaders\\FFTx.glsl");
	FFTyShader = getShader("Shaders\\Ocean Shaders\\FFTy.glsl");
	scaleShader = getShader("Shaders\\Ocean Shaders\\scale.glsl");
	finalHeightShader = getShader("Shaders\\Ocean Shaders\\finalOceanHeight.glsl");
	quadShader = getShader("Shaders\\quad.glsl");

	updateH0Texture();
}

Ocean::~Ocean()
{
	delete gaussianNumbersTexture;
	delete butterflyLookupTexture;
	delete FFTTestTexture;
	delete iFFTTestTexture;
	delete h0Buffer;
	delete htBuffer;
	delete FFTPingPongBuffer;
	delete dataBuffer;
}

int Ocean::update(long elapsedTime)
{
	oceanTime += elapsedTime;
	kConst = (float)(gridSize*2.0*M_PI / lambda);
	//windDirection = glm::vec2(cos(oceanTime*0.0001f), sin(oceanTime*0.0001f));
	updateH0Texture();
	updateHtTexture();
	updateHeights();
	updateChoppyVectors();
	return GameObject::update(elapsedTime);
}

int Ocean::render(long totalElapsedtime)
{
	Shader * temp = activeShader;
	
	shader->activate();
	
	glActiveTexture(GL_TEXTURE0);
	dataBuffer->getTexture(0)->bindTexture();
	shader->setUniformTexture("heightSampler", 0);

	glActiveTexture(GL_TEXTURE1);
	dataBuffer->getTexture(1)->bindTexture();
	shader->setUniformTexture("choppySampler", 1);

	glActiveTexture(GL_TEXTURE2);
	dataBuffer->getTexture(2)->bindTexture();
	shader->setUniformTexture("finalHeightSampler", 2);

	/*glActiveTexture(GL_TEXTURE1);
	dataBuffer->getTexture(1)->bindTexture();
	shader->setUniformTexture("normalSampler", 1);*/

	glm::vec3 camPos = activeCamera->getPos();
	worldMatrix = glm::translate(glm::vec3(camPos.x, 0, camPos.z));
	//worldMatrix = glm::translate(glm::vec3(0, 0, 0));
	shader->setUniformMatrixf4("projViewMatrix", *projMatrix * *viewMatrix);
	shader->setUniformMatrixf4("worldMatrix", worldMatrix);

	shader->setUniformf3("cameraPosition", camPos.x, camPos.y, camPos.z);
	shader->setUniformf1("enableLighting", 1.0f);
	shader->setUniformf2("windDirection", windDirection.x, windDirection.y);
	shader->setUniformf1("textureSize", textureSize);
	activeShader->setUniformf1("boatTextureSize", boatTextureSize);

	model->render();
		
	if (temp != NULL) temp->activate();
	//return GameObject::render(totalElapsedtime);
	return 0;
}

void Ocean::turnWindLeft(long elapsedTime)
{
	glm::vec3 temp = glm::rotate(glm::vec3(windDirection.x, 0, windDirection.y), -elapsedTime*0.01f, glm::vec3(0, 1, 0));
	windDirection = glm::vec2(temp.x, temp.z);
}

void Ocean::turnWindRight(long elapsedTime)
{
	glm::vec3 temp = glm::rotate(glm::vec3(windDirection.x, 0, windDirection.y), elapsedTime*0.01f, glm::vec3(0, 1, 0));
	windDirection = glm::vec2(temp.x, temp.z);
}

float Ocean::getHeight(float x, float z)
{
	//if (x < 0.0f || x > 511.0f || z < 0.0f || z > 511.0f) return 0.0f;
	
	x = mod(x, 512.0f);
	z = mod(z, 512.0f);

	FrameBuffer::setActiveFrameBuffer(dataBuffer, GL_READ_FRAMEBUFFER);
	glReadBuffer(GL_COLOR_ATTACHMENT0);

	GLfloat height;
	glReadPixels((int)(x), (int)(z), 1, 1, GL_RED, GL_FLOAT, &height);
   		
	return (float)height;
}

void Ocean::updateH0Texture()
{
	FrameBuffer::setActiveFrameBuffer(h0Buffer);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glDepthFunc(GL_ALWAYS);
	glDisable(GL_BLEND);

	Shader * prevShader = activeShader;
	h0Shader->activate();

	glActiveTexture(GL_TEXTURE0);
	gaussianNumbersTexture->bindTexture();
	h0Shader->setUniformTexture("gaussianSampler", 0);
	h0Shader->setUniformf2("windDirection", windDirection.x, windDirection.y);
	h0Shader->setUniformf1("amplitude", amplitude);
	h0Shader->setUniformf1("kConst", kConst);
	h0Shader->setUniformf1("windSpeed", windSpeed);

	quad->render();
	
	if (prevShader != NULL) prevShader->activate();
}

void Ocean::updateHtTexture()
{
	FrameBuffer::setActiveFrameBuffer(htBuffer);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glDepthFunc(GL_ALWAYS);
	glDisable(GL_BLEND);

	Shader * prevShader = activeShader;
	htShader->activate();
	
	glActiveTexture(GL_TEXTURE0);
	h0Buffer->getTexture(0)->bindTexture();
	htShader->setUniformTexture("h0Sampler", 0);
	htShader->setUniformf1("kConst", kConst);
	htShader->setUniformf1("time", (float)oceanTime);

	quad->render();
	
	if (prevShader != NULL) prevShader->activate();
}

void Ocean::updateHeights()
{
	Shader * prevShader = activeShader;

	FrameBuffer::setActiveFrameBuffer(FFTPingPongBuffer);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glDepthFunc(GL_ALWAYS);
	glDisable(GL_BLEND);
	quadShader->activate();
	glActiveTexture(GL_TEXTURE0);
	htBuffer->getTexture(0)->bindTexture();
	quadShader->setUniformTexture("textureSampler", 0);
	quadShader->setUniformf1("width", 2.0f);
	quadShader->setUniformf1("height", 2.0f);
	quadShader->setUniformf2("position", 0.0f, 0.0f);
	quad->render();

	FFT();
	heightScalePass();
	
	if (prevShader != NULL) prevShader->activate();
}

void Ocean::updateChoppyVectors()
{
	Shader * prevShader = activeShader;

	FrameBuffer::setActiveFrameBuffer(FFTPingPongBuffer);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glDepthFunc(GL_ALWAYS);
	glDisable(GL_BLEND);
	choppyShader->activate();
	glActiveTexture(GL_TEXTURE0);
	htBuffer->getTexture(0)->bindTexture();
	choppyShader->setUniformTexture("textureSampler", 0);
	quad->render();

	FFT();
	choppyScalePass();

	if (prevShader != NULL) prevShader->activate();
}

void Ocean::FFT()
{
	FFTbool = true;
	FFTxShader->activate();
	for (int pass = 0; pass < numPasses; pass++)
		FFTpass(pass);

	FFTyShader->activate();
	for (int pass = 0; pass < numPasses; pass++)
		FFTpass(pass);
}

void Ocean::FFTpass(int pass)
{
	if (FFTbool)	glDrawBuffer(GL_COLOR_ATTACHMENT1);
	else			glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glDepthFunc(GL_ALWAYS);
	glDisable(GL_BLEND);

	activeShader->setUniformf1("butterflyPass", (float)(float(pass)+0.5)/float(numPasses));

	glActiveTexture(GL_TEXTURE0);
	butterflyLookupTexture->bindTexture();
	glActiveTexture(GL_TEXTURE1);
	if (FFTbool)	FFTPingPongBuffer->getTexture(0)->bindTexture();
	else			FFTPingPongBuffer->getTexture(1)->bindTexture();

	activeShader->setUniformTexture("butterflySampler", 0);
	activeShader->setUniformTexture("FFTSampler", 1);

	quad->render();
	FFTbool = !FFTbool;
}

void Ocean::heightScalePass()
{
	scaleShader->activate();

	FrameBuffer::setActiveFrameBuffer(dataBuffer);
	glDrawBuffer(GL_COLOR_ATTACHMENT0); //First texture attachment
	glDepthFunc(GL_ALWAYS);
	glDisable(GL_BLEND);
	
	glActiveTexture(GL_TEXTURE0);
	if (FFTbool)	FFTPingPongBuffer->getTexture(0)->bindTexture();
	else			FFTPingPongBuffer->getTexture(1)->bindTexture();
	activeShader->setUniformTexture("FFTSampler", 0);
	activeShader->setUniformf1("scaleFactor", (float)(100.0 / float(textureSize*textureSize)));
	activeShader->setUniformf1("textureSize", (float)textureSize);

	quad->render();
}

void Ocean::choppyScalePass()
{
	scaleShader->activate();

	FrameBuffer::setActiveFrameBuffer(dataBuffer);
	glDrawBuffer(GL_COLOR_ATTACHMENT1); //Second texture attachment
	glDepthFunc(GL_ALWAYS);
	glDisable(GL_BLEND);

	glActiveTexture(GL_TEXTURE0);
	if (FFTbool)	FFTPingPongBuffer->getTexture(0)->bindTexture();
	else			FFTPingPongBuffer->getTexture(1)->bindTexture();
	activeShader->setUniformTexture("FFTSampler", 0);
	activeShader->setUniformf1("scaleFactor", (float)(1.0 / float(textureSize*textureSize)));
	activeShader->setUniformf1("textureSize", (float)textureSize);

	quad->render();
}

void Ocean::updateFinalOceanHeights(Texture * boatHeightTexture, float boatTextureSize)
{
	if (boatHeightTexture == NULL) return;

	Shader * temp = activeShader;
	
	FrameBuffer::setActiveFrameBuffer(dataBuffer);
	glDrawBuffer(GL_COLOR_ATTACHMENT2); //Final heights stored in 3rd texture attachment

	finalHeightShader->activate();

	glActiveTexture(GL_TEXTURE0);
	dataBuffer->getTexture(0)->bindTexture();
	activeShader->setUniformTexture("oceanHeightSampler", 0);

	glActiveTexture(GL_TEXTURE1);
	boatHeightTexture->bindTexture();
	activeShader->setUniformTexture("boatHeightSampler", 1);

	glm::vec3 camPos = activeCamera->getPos();
	activeShader->setUniformf3("cameraPosition", camPos.x, camPos.y, camPos.z);
	activeShader->setUniformf1("gridSize", gridSize);
	activeShader->setUniformf1("boatTextureSize", boatTextureSize);
	this->boatTextureSize = boatTextureSize;

	quad->render();

	if (temp != NULL) temp->activate();
}