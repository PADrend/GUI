/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "Draw.h"

#include "Fonts/AbstractFont.h"
#include "BasicColors.h"
#include "../Style/Colors.h" // \todo remove this!!!
#include "GUI_internals.h"
#include <Util/Macros.h>
#include <Util/Graphics/PixelFormat.h>
#include <iostream>

namespace GUI{

//----------------------------------------------------------------------------------
// internal

struct DrawContext{
	bool useShader;
	GLuint shaderProg,activeTextureId,nullTexture;
	GLint attr_color, attr_uv, attr_vertex;
	GLint u_color,	u_colorAttrEnabled, u_posOffset, u_screenScale, u_textureEnabled, u_useVertexColor;
	Geometry::Vec2i position,screenSize;
	DrawContext() : useShader(false),shaderProg(0),activeTextureId(0),nullTexture(0){}
};

static DrawContext ctxt;

static const char * const vs = 
R"***(#version 110
attribute vec4 attr_color;
attribute vec2 attr_vertex;
attribute vec2 attr_uv;
uniform vec2 u_posOffset;
uniform vec2 u_screenScale;
uniform vec4 u_color;
uniform int u_textureEnabled;
uniform int u_colorAttrEnabled;
varying vec2 var_uv;
varying vec4 var_color;
void main() {
	gl_Position = vec4(vec2(-1.0, 1.0) + u_screenScale * (attr_vertex + u_posOffset), -0.1, 1.0);
	var_uv = (u_textureEnabled > 0) ? attr_uv : vec2(0.0);
	var_color = (u_colorAttrEnabled > 0) ? attr_color : u_color;
}
)***";

static const char * const fs = 
R"***(#version 110
varying vec4 var_color;
varying vec2 var_uv;
uniform sampler2D sampler0;
uniform int u_textureEnabled;
void main() {
	vec4 color = var_color;
	if(u_textureEnabled > 0) {
		color *= texture2D(sampler0, var_uv);
	}
	gl_FragColor = color;
}
)***";

static void checkGLError(int line) {
	GLenum errorFlag = glGetError();
	for(int i=0;errorFlag != GL_NO_ERROR && i<10;++i){
		std::cout << "OpenGL Error:"<<errorFlag<<"(Before line:"<<line<<")\n";
		errorFlag = glGetError();
	}
}


static void drawVertices(const GLenum mode,size_t numVertices,const GLfloat * vertices, const Util::Color4ub & color){
//	checkGLError(__LINE__);
	if(ctxt.useShader){
		const Util::Color4f c2(color);
		glUniform4fv(ctxt.u_color,1,c2.data());

		glVertexAttribPointer(ctxt.attr_vertex,2,GL_FLOAT,GL_FALSE,0,vertices);
		glVertexAttribPointer(ctxt.attr_color,4,GL_UNSIGNED_BYTE,GL_FALSE,0,vertices); // dummy for AMD-cards
		glVertexAttribPointer(ctxt.attr_uv,2,GL_UNSIGNED_BYTE,GL_FALSE,0,vertices); // dummy for AMD-cards
		glDrawArrays(mode, 0, numVertices);
	}else{
		glColor4ubv(color.data());
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, vertices);
		glDrawArrays(mode, 0, numVertices);
		glDisableClientState(GL_VERTEX_ARRAY);
	}
//	checkGLError(__LINE__);
}


static void drawVertices(const GLenum mode,size_t numVertices,const GLfloat * vertices, const uint32_t * colors){
//	checkGLError(__LINE__);
	if(ctxt.useShader){	
		glUniform1i(ctxt.u_colorAttrEnabled,1);
		glVertexAttribPointer(ctxt.attr_vertex,2,GL_FLOAT,GL_FALSE,0,vertices);
		glVertexAttribPointer(ctxt.attr_color,4,GL_UNSIGNED_BYTE,GL_TRUE,0,colors);
		glVertexAttribPointer(ctxt.attr_uv,2,GL_UNSIGNED_BYTE,GL_FALSE,0,colors); // dummy for AMD-cards

		glDrawArrays(mode, 0, numVertices ) ;

		glUniform1i(ctxt.u_colorAttrEnabled,0);
	}else{
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
		glVertexPointer(2, GL_FLOAT, 0, vertices);
		glDrawArrays(mode, 0, numVertices ) ;
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
	}
//	checkGLError(__LINE__);
}

static void drawTexturedVertices(const GLenum mode,size_t numVertices,const GLfloat * verticesAndUVs, const Util::Color4ub & color){
//	checkGLError(__LINE__);
	if(ctxt.useShader){	
		glVertexAttribPointer(ctxt.attr_vertex,2,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*4,verticesAndUVs);
		glVertexAttribPointer(ctxt.attr_uv,2,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*4,verticesAndUVs+2);
		glVertexAttribPointer(ctxt.attr_color,4,GL_UNSIGNED_BYTE,GL_FALSE,0,verticesAndUVs); // dummy for AMD-cards

		const Util::Color4f c2(color);
		glUniform4fv(ctxt.u_color,1,c2.data());

		glDrawArrays(mode, 0, numVertices);
	}else{
		glColor4ubv(color.data());
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(2, GL_FLOAT, sizeof(GLfloat)*4, verticesAndUVs);
		glTexCoordPointer(2, GL_FLOAT, sizeof(GLfloat)*4, verticesAndUVs+2 );
		glDrawArrays(mode, 0, numVertices);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
	}
//	checkGLError(__LINE__);
}


//! (internal)
static GLuint createShaderObject(const GLuint type,const char * code){
//	checkGLError(__LINE__);
	const GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &code, nullptr);
	glCompileShader(shader);
	GLint compileStatus;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
	if(compileStatus == GL_FALSE) {
		GLint infoLogLength = 0;
		checkGLError(__LINE__);
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		checkGLError(__LINE__);
		if (infoLogLength > 1) {
			int charsWritten = 0;
			auto infoLog = new char[infoLogLength];
			glGetShaderInfoLog(shader, infoLogLength, &charsWritten, infoLog);
			std::string s(infoLog, charsWritten);
//			// Skip "Everything ok" messages from AMD-drivers.
//			if(s.find("successfully")==string::npos && s.find("shader(s) linked.")==string::npos && s.find("No errors.")==string::npos) {
				WARN(std::string("Shader compile error:\n") + s + "\nShader code:\n" + code);
//			}
			delete [] infoLog;
		}			
		throw std::runtime_error("GUI: Invalid shader.");
	}
	checkGLError(__LINE__);
	return shader;
}


//! (internal)
static bool init(){
	glewInit();
//	checkGLError(__LINE__);
	
	GLuint shaderProg = glCreateProgram();

	const GLuint vertexShader = createShaderObject(GL_VERTEX_SHADER,vs);
	glAttachShader(shaderProg, vertexShader);
	glDeleteShader(vertexShader);

	const GLuint fragmentShader = createShaderObject(GL_FRAGMENT_SHADER,fs);
	glAttachShader(shaderProg, fragmentShader);
	glDeleteShader(fragmentShader);

	glLinkProgram(shaderProg);

	GLint linkStatus;
	glGetProgramiv(shaderProg, GL_LINK_STATUS, &linkStatus);
	if(linkStatus == GL_FALSE) {
		GLint infoLogLength = 0;
		checkGLError(__LINE__);
		glGetProgramiv(shaderProg, GL_INFO_LOG_LENGTH, &infoLogLength);
		checkGLError(__LINE__);
		if (infoLogLength > 1) {
			int charsWritten = 0;
			auto infoLog = new char[infoLogLength];
			glGetProgramInfoLog(shaderProg, infoLogLength, &charsWritten, infoLog);
			std::string s(infoLog, charsWritten);
//			// Skip "Everything ok" messages from AMD-drivers.
//			if(s.find("successfully")==string::npos && s.find("shader(s) linked.")==string::npos && s.find("No errors.")==string::npos) {
				WARN(std::string("Shader could not be linked:\n") + s );
//			}
			delete [] infoLog;
		}			
		throw std::runtime_error("GUI: Invalid shader program.");
	}
	ctxt.shaderProg = shaderProg;
	
	ctxt.u_color = glGetUniformLocation(ctxt.shaderProg ,"u_color");
	ctxt.u_posOffset = glGetUniformLocation(ctxt.shaderProg ,"u_posOffset");
	ctxt.u_textureEnabled = glGetUniformLocation(ctxt.shaderProg ,"u_textureEnabled");
	ctxt.u_colorAttrEnabled = glGetUniformLocation(ctxt.shaderProg ,"u_colorAttrEnabled");
	ctxt.u_screenScale = glGetUniformLocation(ctxt.shaderProg ,"u_screenScale");
	
	ctxt.attr_color = glGetAttribLocation(ctxt.shaderProg ,"attr_color");
	ctxt.attr_vertex = glGetAttribLocation(ctxt.shaderProg ,"attr_vertex");
	ctxt.attr_uv = glGetAttribLocation(ctxt.shaderProg ,"attr_uv");
		
	ctxt.nullTexture = Draw::generateTextureId();
	if(ctxt.nullTexture){
		const uint32_t data = 0xffffffff;
		Draw::uploadTexture(ctxt.nullTexture,1,1,Util::PixelFormat::RGBA, reinterpret_cast<const uint8_t*>(&data));
	}
		
    ctxt.useShader = true;
    
	checkGLError(__LINE__);
	return true;
}
//----------------------------------------------------------------------------------
// general

//! (static)
void Draw::beginDrawing(const Geometry::Vec2i & screenSize){
	// make sure glewInit has been called at least once
	static bool initialized = false;
	if(!initialized){
		initialized = true;
		init();
	}
	checkGLError(__LINE__);
	// Push back and cache the current state of depth testing and lighting
	// and then disable them.
	glPushAttrib( GL_ALL_ATTRIB_BITS);

	glBlendEquation(GL_FUNC_ADD);

	glDisable( GL_DEPTH_TEST );
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glActiveTexture( GL_TEXTURE0 );
	glEnable(GL_SCISSOR_TEST);
	
	ctxt.screenSize = screenSize;
	resetScissor();
	
	if(ctxt.useShader){
		glUseProgram(ctxt.shaderProg);
		ctxt.position = Geometry::Vec2(0,0);
		glUniform2f(ctxt.u_posOffset,ctxt.position.x(),ctxt.position.y());
		glUniform1i(ctxt.u_textureEnabled,0);
		glUniform1i(ctxt.u_colorAttrEnabled,0);
		glUniform2f(ctxt.u_screenScale,2.0/screenSize.getWidth(),-2.0/screenSize.getHeight());
		
		glEnableVertexAttribArray(ctxt.attr_vertex);
		glEnableVertexAttribArray(ctxt.attr_color);
		glEnableVertexAttribArray(ctxt.attr_uv);
		
		// use 1x1 white texture as backup for graphic drivers that access the sampler even in disabled branches...
		glBindTexture(GL_TEXTURE_2D,ctxt.nullTexture);
		ctxt.activeTextureId = ctxt.nullTexture;
	}else{
		glDisable( GL_TEXTURE_2D );
		glDisable( GL_LIGHTING );
		// Push back the current matrices and go orthographic for text rendering.
		glMatrixMode( GL_PROJECTION );
		glPushMatrix();
		glLoadIdentity();

		glOrtho( 0, screenSize.getWidth(),screenSize.getHeight(), 0, -1, 1 );

		glMatrixMode( GL_MODELVIEW );
		glPushMatrix();
		glLoadIdentity();
	}
	checkGLError(__LINE__);

}

//! (static)
void Draw::endDrawing(){
	checkGLError(__LINE__);
	if(ctxt.useShader){	
		glUseProgram(0);
		glDisableVertexAttribArray(ctxt.attr_vertex);
		glDisableVertexAttribArray(ctxt.attr_color);
		glDisableVertexAttribArray(ctxt.attr_uv);
	}else{
		glMatrixMode( GL_PROJECTION );
		glPopMatrix();

		glMatrixMode( GL_MODELVIEW );
		glPopMatrix();
	}
	glBindTexture(GL_TEXTURE_2D,0);
	ctxt.activeTextureId = 0;
	glPopAttrib();
	checkGLError(__LINE__);
}

//! (static)
void Draw::moveCursor(const Geometry::Vec2i & pos){
	if(ctxt.useShader){
		ctxt.position += pos;
		glUniform2f(ctxt.u_posOffset,ctxt.position.x(),ctxt.position.y());
	}else{
		glTranslatef(static_cast<int>(pos.getX()),static_cast<int>(pos.getY()),0);
	}
}

//! (static)
Geometry::Rect_i Draw::queryViewport(){
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport );
	return Geometry::Rect_i(viewport[0],viewport[1],viewport[2],viewport[3]);
}

//! (static)
void Draw::setScissor(const Geometry::Rect_i & rect){
	glScissor(rect.getX(), ctxt.screenSize.getHeight()-rect.getY()-rect.getHeight(), rect.getWidth(), rect.getHeight());
}

//! (static)
void Draw::resetScissor(){
	glScissor(0,0,ctxt.screenSize.getWidth(),ctxt.screenSize.getHeight());
}

//! (static)
void Draw::clearScreen(const Util::Color4ub & color){
	glClearColor(color.getR(), color.getG(), color.getB(), color.getA());
	glClear(GL_COLOR_BUFFER_BIT);
}

//----------------------------------------------------------------------------------
// text
//! (static)
void Draw::drawText(const std::string & text, const Geometry::Vec2 pos,AbstractFont * font, const Util::Color4ub & c){
	if(font==nullptr || text.empty())
		return;
	font->enable();
	font->renderText( pos, text,c);
	font->disable();
}

//! (static)
void Draw::drawText(const std::string & text,const Geometry::Rect & rect,AbstractFont * font,const Util::Color4ub & c,unsigned int style){
	if(font==nullptr || text.empty())
		return;
	font->enable();
	const Geometry::Vec2 size=font->getRenderedTextSize( text );

	Geometry::Vec2 pos=rect.getPosition();
	if ( style&TEXT_ALIGN_RIGHT){
		pos+=Geometry::Vec2( rect.getWidth()-size.getWidth(),0);
	}else if ( style&TEXT_ALIGN_CENTER){
		pos+=Geometry::Vec2( (rect.getWidth()-size.getWidth())*0.5,0);
	}
	if ( style&TEXT_ALIGN_MIDDLE){
		pos+=Geometry::Vec2( 0, (rect.getHeight()-size.getHeight())*0.5);
	}
	font->renderText( pos, text,c);
	font->disable();
}

//! (static)
float Draw::getTextWidth(const std::string & text, AbstractFont * font){
	return font == nullptr ? 0 : font->getRenderedTextSize( text ).getWidth();
}

//! (static)
Geometry::Vec2 Draw::getTextSize(const std::string & text, AbstractFont * font){
	return font == nullptr ? Geometry::Vec2() : font->getRenderedTextSize( text );
}

//----------------------------------------------------------------------------------
// draw

//! (static)
void Draw::drawCross(const Geometry::Rect & r,const Util::Color4ub & c,float lineWidth/*=4.0*/){
	if (c == Colors::NO_COLOR)
		return;

	const float f = lineWidth*0.4;
	const GLfloat vertices[] = {
		r.getMinX()+f,r.getMinY()+0,		r.getMinX()+0,r.getMinY()+f,		r.getMaxX()-f,r.getMaxY(),
		r.getMaxX()-f,r.getMaxY(),			r.getMaxX(),r.getMaxY()-f,			r.getMinX()+f,r.getMinY()+0,
		r.getMaxX(),r.getMinY()+f,			r.getMaxX()-f,r.getMinY()+0,		r.getMinX()+0,r.getMaxY()-f,
		r.getMinX()+0,r.getMaxY()-f,		r.getMinX()+f,r.getMaxY(),			r.getMaxX(),r.getMinY()+f,
	};

	drawVertices(GL_TRIANGLES, sizeof(vertices) / (sizeof(GLfloat)*2), vertices, c);
}

//! (static)
void Draw::draw3DRect(const Geometry::Rect & r,bool down,const Util::Color4ub & bgColor1,const Util::Color4ub & bgColor2){
	glEnable(GL_BLEND);
	if (bgColor1 != Colors::NO_COLOR){
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

		const uint32_t c1 = (down?bgColor2:bgColor1).getAsUInt();
		const uint32_t c2 = (down?bgColor1:bgColor2).getAsUInt();
		const GLfloat vertices[] = {
			r.getMaxX(),r.getMinY(),		r.getMinX(),r.getMinY(),		r.getMinX(),r.getMaxY(),		r.getMaxX()   ,r.getMaxY()
		};
		const uint32_t colors[] = {
			c1,								c1,								c2,								c2
		};
		drawVertices(GL_TRIANGLE_FAN, sizeof(vertices) / (sizeof(GLfloat)*2), vertices, colors);
	}

	const Util::Color4ub & c1 = down ? Colors::BRIGHT_COLOR : Colors::DARK_COLOR;
	const Util::Color4ub & c2 = down ? Colors::DARK_COLOR   : Colors::BRIGHT_COLOR;

	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	const GLfloat vertices[] = {
		r.getMinX(),r.getMaxY(),	r.getMaxX(),r.getMaxY(),	r.getMaxX(),r.getMaxY(),	r.getMaxX(),r.getMinY(),
		r.getMaxX(),r.getMinY(),	r.getMinX(),r.getMinY(),	r.getMinX(),r.getMinY(),	r.getMinX(),r.getMaxY()
	};
	const uint32_t colors[] = {
		c1.getAsUInt(),		c1.getAsUInt(),		c1.getAsUInt(),		c1.getAsUInt(),
		c2.getAsUInt(),		c2.getAsUInt(),		c2.getAsUInt(),		c2.getAsUInt()
	};
	drawVertices(GL_LINES, sizeof(vertices) / (sizeof(GLfloat)*2), vertices, colors);
	glDisable(GL_BLEND);
}

//! (static)
void Draw::drawFilledRect(const Geometry::Rect & r,const Util::Color4ub & bgColor,bool blend){
	if (bgColor.isTransparent())
		return;

	if (blend) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	}

	const GLfloat vertices[] = {	r.getMinX(), r.getMinY(),	r.getMinX(), r.getMaxY(),	r.getMaxX(), r.getMaxY(),	r.getMaxX(), r.getMinY()	};
	drawVertices(GL_TRIANGLE_FAN, sizeof(vertices) / (sizeof(GLfloat)*2), vertices, bgColor);

	if(blend)
		glDisable(GL_BLEND);
}

//! (static)
void Draw::drawFilledRect(const Geometry::Rect & r,const Util::Color4ub & bgColorTL, const Util::Color4ub & bgColorBL,
									const Util::Color4ub & bgColorBR, const Util::Color4ub & bgColorTR, bool blend){
	if (blend) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	}

	const GLfloat vertices[] = {	
			r.getMinX(), r.getMinY(),		r.getMinX(), r.getMaxY(),		r.getMaxX(), r.getMaxY(),		r.getMaxX(), r.getMinY()	};
	const uint32_t colors[] = {
			bgColorTL.getAsUInt(),			bgColorBL.getAsUInt(),			bgColorBR.getAsUInt(),			bgColorTR.getAsUInt()
		};
	drawVertices(GL_TRIANGLE_FAN, sizeof(vertices) / (sizeof(GLfloat)*2), vertices, colors);

	if(blend)
		glDisable(GL_BLEND);
}
									
//! (static)
void Draw::drawLineRect(const Geometry::Rect & r,const Util::Color4ub & lineColor,bool blend){
	if (lineColor.isTransparent())
		return;

	if (blend) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	}

	const GLfloat vertices[] = {	r.getMinX(), r.getMinY(),	r.getMinX(), r.getMaxY(),	r.getMaxX(), r.getMaxY(),	r.getMaxX(), r.getMinY()	};
	drawVertices(GL_LINE_LOOP, sizeof(vertices) / (sizeof(GLfloat)*2), vertices, lineColor);

	if(blend)
		glDisable(GL_BLEND);
}

//! (static)
void Draw::drawTab(const Geometry::Rect & r,const Util::Color4ub & lineColor, const Util::Color4ub & bgColor1,const Util::Color4ub & bgColor2){
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	if (bgColor1 != Colors::NO_COLOR && bgColor2 != Colors::NO_COLOR){

		const GLfloat vertices[] = {
			r.getMinX(),r.getMaxY(),		r.getMaxX(),r.getMaxY(),		r.getMaxX(),r.getMinY()+3,
			r.getMaxX()-3,r.getMinY(),		r.getMinX()+3,r.getMinY(),		r.getMinX(),r.getMinY()+3
		};
		const uint32_t c1 = bgColor1.getAsUInt();
		const uint32_t c2 = bgColor2.getAsUInt();
		const uint32_t colors[] = {	c2,c2,c1,c1,c1,c1	};
		drawVertices(GL_TRIANGLE_FAN, sizeof(vertices) / (sizeof(GLfloat)*2), vertices, colors);
	}
	if (lineColor != Colors::NO_COLOR){
		const GLfloat vertices[] = {
			r.getMaxX(),r.getMaxY(),		r.getMaxX(),r.getMinY()+3,		r.getMaxX()-3,r.getMinY(),
			r.getMinX()+3,r.getMinY(),		r.getMinX(),r.getMinY()+3,		r.getMinX(),r.getMaxY()
		};
		drawVertices(GL_LINE_STRIP, sizeof(vertices) / (sizeof(GLfloat)*2), vertices, lineColor);
	}
	glDisable(GL_BLEND);
}

//! (static)
void Draw::dropShadow(const Geometry::Rect & r){
	glEnable(GL_BLEND);
	glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	const uint32_t c1 = Util::Color4ub(0,0,0,60).getAsUInt();
	const uint32_t c2 = Util::Color4ub(0,0,0,0).getAsUInt();
	const float s=5.0;
	const float s1=s*0.25;
	const float s2=s*0.75;

	/*
					|I J
					|K L 
					|
			________|
			A C      E F
			B D      G H
	*/

	const GLfloat vertices[] = {
		/*C*/r.getMinX()+s,r.getMaxY(),		/*A*/r.getMinX(),r.getMaxY(),		/*B*/r.getMinX()+s1,r.getMaxY()+s2,
		/*C*/r.getMinX()+s,r.getMaxY(),		/*B*/r.getMinX()+s1,r.getMaxY()+s2,	/*D*/r.getMinX()+s,r.getMaxY()+s,		
		/*C*/r.getMinX()+s,r.getMaxY(),		/*D*/r.getMinX()+s,r.getMaxY()+s,	/*E*/r.getMaxX(),r.getMaxY(),
		/*E*/r.getMaxX(),r.getMaxY(),		/*D*/r.getMinX()+s,r.getMaxY()+s,	/*G*/r.getMaxX(),r.getMaxY()+s,	
		/*E*/r.getMaxX(),r.getMaxY(),		/*G*/r.getMaxX(),r.getMaxY()+s,		/*H*/r.getMaxX()+s2,r.getMaxY()+s2,	
		/*E*/r.getMaxX(),r.getMaxY(),		/*H*/r.getMaxX()+s2,r.getMaxY()+s2,	/*F*/r.getMaxX()+s,r.getMaxY(),
		/*E*/r.getMaxX(),r.getMaxY(),		/*F*/r.getMaxX()+s,r.getMaxY(),		/*K*/r.getMaxX(),r.getMinY()+s,
		/*K*/r.getMaxX(),r.getMinY()+s,		/*F*/r.getMaxX()+s,r.getMaxY(),		/*L*/r.getMaxX()+s,r.getMinY()+s,	
		/*K*/r.getMaxX(),r.getMinY()+s,		/*L*/r.getMaxX()+s,r.getMinY()+s,	/*J*/r.getMaxX()+s2,r.getMinY()+s1,		
		/*K*/r.getMaxX(),r.getMinY()+s,		/*J*/r.getMaxX()+s2,r.getMinY()+s1,	/*I*/r.getMaxX(),r.getMinY(),		
	};
	const uint32_t colors[] = {
		c1,	c2,	c2,		c1,	c2,	c2,		c1,	c2,	c1,		c1,	c2,	c2,		c1,	c2,	c2,		
		c1,	c2,	c2,		c1,	c2,	c1,		c1,	c2,	c2,		c1,	c2,	c2,		c1,	c2,	c2,
	};

	drawVertices(GL_TRIANGLES, sizeof(vertices) / (sizeof(GLfloat)*2), vertices, colors);
	glDisable(GL_BLEND);
}


//! (static)
void Draw::drawTexturedTriangles(const std::vector<float> & posAndUV, const Util::Color4ub & c, bool blend/* = true*/){
	if (blend) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	}
	drawTexturedVertices(GL_TRIANGLES, posAndUV.size() / (4), posAndUV.data() , c);
	if (blend)
		glDisable(GL_BLEND);
}


//! (static)
void Draw::drawTexturedRect(const Geometry::Rect_i & screenRect,const Geometry::Rect & uvRect,const Util::Color4ub & c,bool blend/*=true*/){
	if (blend) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	}
	const GLfloat vertices[] = {
		static_cast<float>(screenRect.getMinX()),static_cast<float>(screenRect.getMaxY()),	static_cast<float>(uvRect.getMinX()),static_cast<float>(uvRect.getMaxY()),
		static_cast<float>(screenRect.getMaxX()),static_cast<float>(screenRect.getMaxY()),	static_cast<float>(uvRect.getMaxX()),static_cast<float>(uvRect.getMaxY()),
		static_cast<float>(screenRect.getMaxX()),static_cast<float>(screenRect.getMinY()),	static_cast<float>(uvRect.getMaxX()),static_cast<float>(uvRect.getMinY()),
		static_cast<float>(screenRect.getMinX()),static_cast<float>(screenRect.getMinY()),	static_cast<float>(uvRect.getMinX()),static_cast<float>(uvRect.getMinY())
	};
	drawTexturedVertices(GL_TRIANGLE_FAN, sizeof(vertices) / (sizeof(GLfloat)*4), vertices, c);

	if (blend)
		glDisable(GL_BLEND);
}

//! (static)
void Draw::drawLine(const std::vector<float> & vertices,const std::vector<uint32_t> & colors,const float lineWidth/*=1.0*/,bool lineSmooth/*=false*/){
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glPushAttrib(GL_LINE_BIT);
	glLineWidth(lineWidth);
	if(lineSmooth){
		glEnable(GL_LINE_SMOOTH);
		glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
	}

// assert colors.size() == vertices.size()
	drawVertices(GL_LINE_STRIP, vertices.size() / 2, vertices.data(), colors.data());
	if(lineSmooth){
		glDisable(GL_LINE_SMOOTH);
//		glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
	}

	glPopAttrib();
	glDisable(GL_BLEND);
}

//! (static)
void Draw::drawLines(const std::vector<float> & vertices,const std::vector<uint32_t> & colors,const float lineWidth/*=1.0*/){
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glPushAttrib(GL_LINE_BIT);
	glLineWidth(lineWidth);
// assert colors.size() == vertices.size()
	drawVertices(GL_LINES, vertices.size() / 2, vertices.data(), colors.data());

	glPopAttrib();
	glDisable(GL_BLEND);
}


//! @p vertices:  { x0,y0, x1,y1, x2,y2, ... } @p color {c0, c1, c2, ...}
void Draw::drawTriangleFan(const std::vector<float> & vertices,const std::vector<uint32_t> & colors){
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	drawVertices(GL_TRIANGLE_FAN, vertices.size() / 2, vertices.data(), colors.data());
	// assert colors.size() == vertices.size()
	glDisable(GL_BLEND);
}

//----------------------------------------------------------------------------------
// texture

void Draw::disableTexture(){
	if(ctxt.useShader){
		if(ctxt.activeTextureId != ctxt.nullTexture){
			glBindTexture(GL_TEXTURE_2D,ctxt.nullTexture);
			ctxt.activeTextureId = ctxt.nullTexture;
		}
		glUniform1i(ctxt.u_textureEnabled,0);
	}else{
		glDisable(GL_TEXTURE_2D);
	}
}

void Draw::destroyTexture(uint32_t textureId) {
	GLuint glId = static_cast<GLuint>(textureId);
	glDeleteTextures(1,&glId);
}
	
void Draw::enableTexture(uint32_t textureId) {
	if(ctxt.useShader){
		if(ctxt.activeTextureId != textureId){
			glBindTexture(GL_TEXTURE_2D,textureId);
			ctxt.activeTextureId = textureId;
		}
		glUniform1i(ctxt.u_textureEnabled,1);
	}else{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,textureId);
		ctxt.activeTextureId = textureId;
	}
}

uint32_t Draw::generateTextureId(){
	GLuint glId;
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glGenTextures(1,&glId);
	if(glId != 0){
		glBindTexture(GL_TEXTURE_2D,glId);

		// ... parameter
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_R, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D,ctxt.activeTextureId);
	}
	return static_cast<uint32_t>(glId);
}

void Draw::uploadTexture(uint32_t textureId,uint32_t width,uint32_t height,const Util::PixelFormat & pixelFormat, const uint8_t * data){

	GLint glInternalFormat = pixelFormat.getBytesPerPixel();
	GLint glFormat;
	if(pixelFormat==Util::PixelFormat::RGBA){
		glFormat = GL_RGBA;
	}else if(pixelFormat==Util::PixelFormat::BGRA){
		glFormat = GL_BGRA;
	}else if(pixelFormat==Util::PixelFormat::RGB){
		glFormat = GL_RGB;
	}else if(pixelFormat==Util::PixelFormat::BGR){
		glFormat = GL_BGR;
	}else if(pixelFormat==Util::PixelFormat::MONO){
		glFormat = GL_RED;
	}else{
		throw std::invalid_argument("Draw::uploadTexture: Bitmap has unimplemented color format.");
	}
	GLenum glDataType;
	switch( pixelFormat.getBytesPerComponent() ){
		case 1:
			glDataType = GL_UNSIGNED_BYTE;
			break;
		case 4:
			glDataType = GL_FLOAT;
			break;
		default: {
			throw std::invalid_argument("Draw::uploadTexture: Bitmap has invalid data format.");
		}
	}

	glBindTexture(GL_TEXTURE_2D,textureId);
	glTexImage2D(GL_TEXTURE_2D,0, glInternalFormat,	width,height, /*border*/0, glFormat, glDataType, data);
	glBindTexture(GL_TEXTURE_2D,ctxt.activeTextureId);
}

//----------------------------------------------------------------------------------


}
