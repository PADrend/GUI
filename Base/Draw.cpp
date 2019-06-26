/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	Copyright (C) 2018-2019 Sascha Brandt <sascha@brandt.graphics>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "Draw.h"

#include "Fonts/AbstractFont.h"
#include "BasicColors.h"
#include "../Style/Colors.h" // \todo remove this!!!
#include <Util/Macros.h>
#include <Util/Graphics/PixelFormat.h>
#include <Util/Graphics/PixelAccessor.h>
#include <Util/References.h>

#include <Rendering/Helper.h>
#include <Rendering/RenderingContext/RenderingContext.h>
#include <Rendering/RenderingContext/RenderingParameters.h>
#include <Rendering/Shader/Shader.h>
#include <Rendering/Shader/Uniform.h>
#include <Rendering/Texture/Texture.h>
#include <Rendering/Texture/TextureUtils.h>
#include <Rendering/Mesh/Mesh.h>
#include <Rendering/Mesh/MeshDataStrategy.h>
#include <Rendering/Mesh/VertexAttributeAccessors.h>

#include <iostream>
#include <cstring>
#include <deque>

#include <GL/glew.h>

namespace GUI {
using namespace Rendering;

static const uint32_t maxVertexCount = 32768; // 1 MB

static const Uniform::UniformName UNIFORM_POS_OFFSET("u_posOffset");
static const Uniform::UniformName UNIFORM_SCREEN_SCALE("u_screenScale");

//----------------------------------------------------------------------------------
// internal

struct DrawCommand {
	uint32_t start;
	uint32_t count;
	Geometry::Vec2f offset;
	Geometry::Rect_i scissor;
	Mesh::draw_mode_t mode;
	bool blending;
	Util::Reference<Texture> texture;
	DrawCommand(uint32_t start, uint32_t count, Geometry::Vec2f offset, Geometry::Rect_i scissor, Mesh::draw_mode_t mode, bool blend, Util::Reference<Texture> texture=nullptr) :
		start(start), count(count), offset(offset), scissor(scissor), mode(mode), blending(blend), texture(texture) {}
};

struct DrawContext {
	uint32_t meshOffset = 0;
	Geometry::Vec2i position,screenSize;
	Geometry::Rect_i scissor;
	
	RenderingContext* rc;
	Util::Reference<Shader> shader;
	Util::Reference<Mesh> mesh;
	Util::Reference<TexCoordAttributeAccessor> posAcc;
	Util::Reference<ColorAttributeAccessor> colAcc;
	Util::Reference<TexCoordAttributeAccessor> uvAcc;
	Util::Reference<Texture> activeTexture;
	
	std::deque<DrawCommand> commands;
};

static DrawContext ctxt;

static const char * const vs = 
R"***(#version 130
in vec2 sg_Position;
in vec2 sg_TexCoord0;
in vec4 sg_Color;
uniform vec2 u_posOffset;
uniform vec2 u_screenScale;
out vec2 var_uv;
out vec4 var_color;
void main() {
	gl_Position = vec4(vec2(-1.0, 1.0) + u_screenScale * (sg_Position + u_posOffset), -0.1, 1.0);
	var_uv = sg_TexCoord0 * vec2(1.0,-1.0);
	var_color = sg_Color;
}
)***";

static const char * const fs = 
R"***(#version 130
in vec4 var_color;
in vec2 var_uv;
uniform sampler2D sg_texture0;
uniform bool sg_textureEnabled[8];
out vec4 fragColor;
void main() {
	vec4 color = var_color;
	if(sg_textureEnabled[0]) {
		color *= texture2D(sg_texture0, var_uv);
	}
	fragColor = color;
}
)***";

static void drawVertices(const Mesh::draw_mode_t mode, const std::vector<Geometry::Vec2>& vertices, const Util::Color4f& color, bool blending=false) {
	if(ctxt.meshOffset+vertices.size() > ctxt.mesh->getVertexCount())
		Draw::flush();
	
	for(uint32_t i=0; i<vertices.size(); ++i) {
		ctxt.posAcc->setCoordinate(ctxt.meshOffset+i, vertices[i]);
		ctxt.colAcc->setColor(ctxt.meshOffset+i, color);
		ctxt.uvAcc->setCoordinate(ctxt.meshOffset+i, {0,0});
	}
	
	ctxt.commands.emplace_back(ctxt.meshOffset, vertices.size(), ctxt.position, ctxt.scissor, mode, blending);
	ctxt.meshOffset += vertices.size();
}

static void drawVertices(const Mesh::draw_mode_t mode, const std::vector<Geometry::Vec2>& vertices, const std::vector<Util::Color4f>& colors, bool blending=false) {
	if(ctxt.meshOffset+vertices.size() > ctxt.mesh->getVertexCount())
		Draw::flush();
		
	for(uint32_t i=0; i<vertices.size(); ++i) {
		ctxt.posAcc->setCoordinate(ctxt.meshOffset+i, vertices[i]);
		ctxt.colAcc->setColor(ctxt.meshOffset+i, colors[i]);
		ctxt.uvAcc->setCoordinate(ctxt.meshOffset+i, {0,0});
	}
	
	ctxt.commands.emplace_back(ctxt.meshOffset, vertices.size(), ctxt.position, ctxt.scissor, mode, blending);
	ctxt.meshOffset += vertices.size();
}

static void drawTexturedVertices(const Mesh::draw_mode_t mode, const std::vector<Geometry::Vec2>& vertices, const std::vector<Geometry::Vec2>& uvs, const Util::Color4f& color, bool blending=false) {
	assert(vertices.size() == uvs.size());
	if(ctxt.meshOffset+vertices.size() > ctxt.mesh->getVertexCount())
		Draw::flush();
	
	for(uint32_t i=0; i<vertices.size(); ++i) {
		ctxt.posAcc->setCoordinate(ctxt.meshOffset+i, vertices[i]);
		ctxt.colAcc->setColor(ctxt.meshOffset+i, color);
		ctxt.uvAcc->setCoordinate(ctxt.meshOffset+i, uvs[i]);
	}
	
	ctxt.commands.emplace_back(ctxt.meshOffset, vertices.size(), ctxt.position, ctxt.scissor, mode, blending, ctxt.activeTexture);
	ctxt.meshOffset += vertices.size();
}

//! (internal)
static bool init() {	
	ctxt.shader = Shader::createShader(vs, fs);	
	if(!ctxt.shader->init())
		throw std::runtime_error("GUI: Invalid shader program.");
		
	ctxt.meshOffset = 0;
	VertexDescription vd;
	vd.appendPosition2D();
	vd.appendTexCoord();
	vd.appendColorRGBAFloat();
	ctxt.mesh = new Mesh(vd, maxVertexCount, 0);
	ctxt.mesh->setUseIndexData(false);
	ctxt.mesh->setDataStrategy(SimpleMeshDataStrategy::getDynamicVertexStrategy());
	ctxt.posAcc = TexCoordAttributeAccessor::create(ctxt.mesh->openVertexData(), VertexAttributeIds::POSITION);
	ctxt.colAcc = ColorAttributeAccessor::create(ctxt.mesh->openVertexData(), VertexAttributeIds::COLOR);
	ctxt.uvAcc = TexCoordAttributeAccessor::create(ctxt.mesh->openVertexData(), VertexAttributeIds::TEXCOORD0);
	
	GET_GL_ERROR();
	return true;
}
//----------------------------------------------------------------------------------
// general

//! (static)
void Draw::beginDrawing(Rendering::RenderingContext& rc, const Geometry::Vec2i & screenSize) {
	ctxt.rc = &rc;
	
	static bool initialized = false;
	if(!initialized) {
		initialized = true;
		init();
	}
	
	ctxt.position = Geometry::Vec2(0,0);
	ctxt.activeTexture = nullptr;
	ctxt.screenSize = screenSize;
	ctxt.meshOffset = 0;
		
	rc.pushAndSetDepthBuffer(DepthBufferParameters(false, false, Comparison::ALWAYS));
	rc.pushAndSetPolygonMode(PolygonModeParameters(PolygonModeParameters::FILL));
	rc.pushAndSetCullFace(CullFaceParameters(CullFaceParameters::CULL_BACK));
	rc.pushAndSetTexture(0, nullptr);
	rc.pushAndSetBlending(BlendingParameters());
	rc.pushScissor();
	resetScissor();	
	rc.pushAndSetShader(ctxt.shader.get());
	ctxt.shader->setUniform(rc, Uniform(UNIFORM_SCREEN_SCALE, Geometry::Vec2(2.0/screenSize.getWidth(),-2.0/screenSize.getHeight())));
		
	GET_GL_ERROR();
}

//! (static)
void Draw::endDrawing() {
	auto& rc = *ctxt.rc;
	
	flush();
		
	rc.popDepthBuffer();
	rc.popPolygonMode();
	rc.popCullFace();
	rc.popTexture(0);
	rc.popBlending();
	rc.popScissor();
	rc.popShader();
	
	ctxt.rc = nullptr;
	GET_GL_ERROR();
}

//! (static)
Rendering::RenderingContext& Draw::getRenderingContext() {
	return *ctxt.rc;
}

//! (static)
void Draw::flush() {
	BlendingParameters blending(BlendingParameters::SRC_ALPHA, BlendingParameters::ONE_MINUS_SRC_ALPHA);
	ctxt.mesh->openVertexData().markAsChanged();
	for(const auto& cmd : ctxt.commands) {
		ctxt.shader->setUniform(*ctxt.rc, {UNIFORM_POS_OFFSET, cmd.offset});
		ctxt.rc->setScissor(ScissorParameters(cmd.scissor));
		if(cmd.blending)
			blending.enable();
		else
			blending.disable();
		ctxt.rc->setBlending(blending);
		ctxt.rc->setTexture(0, cmd.texture.get());
		ctxt.mesh->setDrawMode(cmd.mode);
		ctxt.rc->displayMesh(ctxt.mesh.get(), cmd.start, cmd.count);
	}
	ctxt.commands.clear();
	ctxt.rc->finish();
	ctxt.meshOffset = 0;
}

//! (static)
void Draw::moveCursor(const Geometry::Vec2i & pos) {
	ctxt.position += pos;
}

//! (static)
void Draw::setScissor(const Geometry::Rect_i & rect) {
	ctxt.scissor = {rect.getX(), ctxt.screenSize.getHeight()-rect.getY()-rect.getHeight(), rect.getWidth(), rect.getHeight()};
}

//! (static)
void Draw::resetScissor() {
	ctxt.scissor = {0,0,ctxt.screenSize.getWidth(),ctxt.screenSize.getHeight()};
}

//! (static)
void Draw::clearScreen(const Util::Color4ub & color) {
	flush();
	ctxt.rc->clearScreen(color);
}

//----------------------------------------------------------------------------------
// text
//! (static)
void Draw::drawText(const std::string & text, const Geometry::Vec2 pos,AbstractFont * font, const Util::Color4ub & c) {
	if(font==nullptr || text.empty())
		return;
	font->enable();
	font->renderText( pos, text,c);
	font->disable();
}

//! (static)
void Draw::drawText(const std::string & text,const Geometry::Rect & rect,AbstractFont * font,const Util::Color4ub & c,unsigned int style) {
	if(font==nullptr || text.empty())
		return;
	font->enable();
	const Geometry::Vec2 size=font->getRenderedTextSize( text );

	Geometry::Vec2 pos=rect.getPosition();
	if ( style&TEXT_ALIGN_RIGHT) {
		pos+=Geometry::Vec2( rect.getWidth()-size.getWidth(),0);
	}else if ( style&TEXT_ALIGN_CENTER) {
		pos+=Geometry::Vec2( (rect.getWidth()-size.getWidth())*0.5,0);
	}
	if ( style&TEXT_ALIGN_MIDDLE) {
		pos+=Geometry::Vec2( 0, (rect.getHeight()-size.getHeight())*0.5);
	}
	font->renderText( pos, text,c);
	font->disable();
}

//! (static)
float Draw::getTextWidth(const std::string & text, AbstractFont * font) {
	return font == nullptr ? 0 : font->getRenderedTextSize( text ).getWidth();
}

//! (static)
Geometry::Vec2 Draw::getTextSize(const std::string & text, AbstractFont * font) {
	return font == nullptr ? Geometry::Vec2() : font->getRenderedTextSize( text );
}

//----------------------------------------------------------------------------------
// draw

//! (static)
void Draw::drawCross(const Geometry::Rect & r,const Util::Color4ub & c,float lineWidth/*=4.0*/) {
	if (c == Colors::NO_COLOR)
		return;

	const float f = lineWidth*0.4;
	const std::vector<Geometry::Vec2> vertices = {
		{r.getMinX()+f,r.getMinY()+0},
		{r.getMinX()+0,r.getMinY()+f},
		{r.getMaxX()-f,r.getMaxY()},
		{r.getMaxX()-f,r.getMaxY()},
		{r.getMaxX(),r.getMaxY()-f},
		{r.getMinX()+f,r.getMinY()+0},
		{r.getMaxX(),r.getMinY()+f},
		{r.getMaxX()-f,r.getMinY()+0},
		{r.getMinX()+0,r.getMaxY()-f},
		{r.getMinX()+0,r.getMaxY()-f},
		{r.getMinX()+f,r.getMaxY()},
		{r.getMaxX(),r.getMinY()+f},
	};

	drawVertices(Mesh::DRAW_TRIANGLES, vertices, c);
}

//! (static)
void Draw::draw3DRect(const Geometry::Rect & r,bool down,const Util::Color4ub & bgColor1,const Util::Color4ub & bgColor2) {
	if (bgColor1 != Colors::NO_COLOR) {
		const auto c1 = down?bgColor2:bgColor1;
		const auto c2 = down?bgColor1:bgColor2;
		const std::vector<Geometry::Vec2> vertices = {
			{r.getMaxX(),r.getMinY()}, {r.getMinX(),r.getMinY()}, {r.getMinX(),r.getMaxY()},
			{r.getMinX(),r.getMaxY()}, {r.getMaxX(),r.getMaxY()}, {r.getMaxX(),r.getMinY()}
		};
		const std::vector<Util::Color4f> colors = {
			c1, c1, c2, c2, c2, c1
		};
		drawVertices(Mesh::DRAW_TRIANGLES, vertices, colors, true);
	}

	const Util::Color4ub & c1 = down ? Colors::BRIGHT_COLOR : Colors::DARK_COLOR;
	const Util::Color4ub & c2 = down ? Colors::DARK_COLOR   : Colors::BRIGHT_COLOR;
	
	// assure sharp lines
	const Geometry::Rect_i r2(r);
	Geometry::Rect r3(r2);
	r3.moveRel(0.5f,0.5f);
	
	const std::vector<Geometry::Vec2> vertices = {
		{r3.getMinX(),r3.getMaxY()}, {r3.getMaxX(),r3.getMaxY()}, {r3.getMaxX(),r3.getMaxY()}, {r3.getMaxX(),r3.getMinY()},
		{r3.getMaxX(),r3.getMinY()}, {r3.getMinX(),r3.getMinY()}, {r3.getMinX(),r3.getMinY()}, {r3.getMinX(),r3.getMaxY()}
	};
	const std::vector<Util::Color4f> colors = {
		c1, c1, c1, c1,
		c2, c2, c2, c2
	};
	drawVertices(Mesh::DRAW_LINES, vertices, colors, true);
}

//! (static)
void Draw::drawFilledRect(const Geometry::Rect & r,const Util::Color4ub & bgColor,bool blend) {
	if (bgColor.isTransparent())
		return;
	
	const std::vector<Geometry::Vec2> vertices = {
		{r.getMaxX(),r.getMinY()}, {r.getMinX(),r.getMinY()}, {r.getMinX(),r.getMaxY()},
		{r.getMinX(),r.getMaxY()}, {r.getMaxX(),r.getMaxY()}, {r.getMaxX(),r.getMinY()}
	};
	drawVertices(Mesh::DRAW_TRIANGLES, vertices, bgColor, blend);
}

//! (static)
void Draw::drawFilledRect(const Geometry::Rect & r,const Util::Color4ub & bgColorTL, const Util::Color4ub & bgColorBL,
									const Util::Color4ub & bgColorBR, const Util::Color4ub & bgColorTR, bool blend) {
	const std::vector<Geometry::Vec2> vertices = {
		{r.getMaxX(),r.getMinY()}, {r.getMinX(),r.getMinY()}, {r.getMinX(),r.getMaxY()},
		{r.getMinX(),r.getMaxY()}, {r.getMaxX(),r.getMaxY()}, {r.getMaxX(),r.getMinY()}
	};
	const std::vector<Util::Color4f> colors = {
		bgColorTR, bgColorTL, bgColorBL,
		bgColorBL, bgColorBR, bgColorTR,
	};
	drawVertices(Mesh::DRAW_TRIANGLES, vertices, colors, blend);
}
									
//! (static)
void Draw::drawLineRect(const Geometry::Rect & r,const Util::Color4ub & lineColor,bool blend) {
	if (lineColor.isTransparent())
		return;
	
	Geometry::Rect_i ri(r);
	const std::vector<Geometry::Vec2> vertices = {
		{ri.getMinX()+0.5f,ri.getMinY()+0.5f}, 
		{ri.getMinX()+0.5f,ri.getMaxY()+0.5f}, 
		{ri.getMaxX()+0.5f,ri.getMaxY()+0.5f},
		{ri.getMaxX()+0.5f,ri.getMinY()+0.5f}
	};
	drawVertices(Mesh::DRAW_LINE_LOOP, vertices, lineColor, blend);
}

//! (static)
void Draw::drawTab(const Geometry::Rect & r,const Util::Color4ub & lineColor, const Util::Color4ub & bgColor1,const Util::Color4ub & bgColor2) {
	if (bgColor1 != Colors::NO_COLOR && bgColor2 != Colors::NO_COLOR) {
		
		const std::vector<Geometry::Vec2> vertices = {
			{r.getMinX(),r.getMaxY()}, {r.getMaxX(),r.getMaxY()}, {r.getMaxX(),r.getMinY()+3},
			{r.getMinX(),r.getMaxY()}, {r.getMaxX(),r.getMaxY()+3}, {r.getMaxX()-3,r.getMinY()},
			{r.getMinX(),r.getMaxY()}, {r.getMaxX()-3,r.getMinY()}, {r.getMinX()+3,r.getMinY()},
			{r.getMinX(),r.getMaxY()}, {r.getMinX()+3,r.getMinY()}, {r.getMinX(),r.getMinY()+3}, 
		};
		const std::vector<Util::Color4f> colors = {
			bgColor2, bgColor2, bgColor1,
			bgColor2, bgColor1, bgColor1,
			bgColor2, bgColor1, bgColor1,
			bgColor2, bgColor1, bgColor1,
		};
		drawVertices(Mesh::DRAW_TRIANGLES, vertices, colors, true);
	}
	
	if (lineColor != Colors::NO_COLOR) {
		Geometry::Rect_i ri(r);
		const std::vector<Geometry::Vec2> vertices = {
			{ri.getMaxX()+0.5f, ri.getMaxY()+0.5f},
			{ri.getMaxX()+0.5f, ri.getMinY()+3.5f},
			{ri.getMaxX()-2.5f, ri.getMinY()+0.5f},
			{ri.getMinX()+3.5f, ri.getMinY()+0.5f},
			{ri.getMinX()+0.5f, ri.getMinY()+3.5f},
			{ri.getMinX()+0.5f, ri.getMaxY()+0.5f}
		};
		drawVertices(Mesh::DRAW_LINE_LOOP, vertices, lineColor, true);
	}
}

//! (static)
void Draw::dropShadow(const Geometry::Rect & r) {
	const Util::Color4ub c1(0,0,0,60);
	const Util::Color4ub c2(0,0,0,0);
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
	
	const std::vector<Geometry::Vec2> vertices = {
		/*C*/{r.getMinX()+s,r.getMaxY()  }, /*A*/{r.getMinX()   ,r.getMaxY()   }, /*B*/{r.getMinX()+s1,r.getMaxY()+s2},
		/*C*/{r.getMinX()+s,r.getMaxY()  }, /*B*/{r.getMinX()+s1,r.getMaxY()+s2}, /*D*/{r.getMinX()+s ,r.getMaxY()+s },
		/*C*/{r.getMinX()+s,r.getMaxY()  }, /*D*/{r.getMinX()+s ,r.getMaxY()+s }, /*E*/{r.getMaxX()   ,r.getMaxY()   },
		/*E*/{r.getMaxX()  ,r.getMaxY()  }, /*D*/{r.getMinX()+s ,r.getMaxY()+s }, /*G*/{r.getMaxX()   ,r.getMaxY()+s },
		/*E*/{r.getMaxX()  ,r.getMaxY()  }, /*G*/{r.getMaxX()   ,r.getMaxY()+s }, /*H*/{r.getMaxX()+s2,r.getMaxY()+s2},
		/*E*/{r.getMaxX()  ,r.getMaxY()  }, /*H*/{r.getMaxX()+s2,r.getMaxY()+s2}, /*F*/{r.getMaxX()+s ,r.getMaxY()   },
		/*E*/{r.getMaxX()  ,r.getMaxY()  }, /*F*/{r.getMaxX()+s ,r.getMaxY()   }, /*K*/{r.getMaxX()   ,r.getMinY()+s },
		/*K*/{r.getMaxX()  ,r.getMinY()+s}, /*F*/{r.getMaxX()+s ,r.getMaxY()   }, /*L*/{r.getMaxX()+s ,r.getMinY()+s },
		/*K*/{r.getMaxX()  ,r.getMinY()+s}, /*L*/{r.getMaxX()+s ,r.getMinY()+s }, /*J*/{r.getMaxX()+s2,r.getMinY()+s1},
		/*K*/{r.getMaxX()  ,r.getMinY()+s}, /*J*/{r.getMaxX()+s2,r.getMinY()+s1}, /*I*/{r.getMaxX()   ,r.getMinY()   },
	};
	const std::vector<Util::Color4f> colors = {
		c1,c2,c2, c1,c2,c2, c1,c2,c1, c1,c2,c2, c1,c2,c2,
		c1,c2,c2, c1,c2,c1, c1,c2,c2, c1,c2,c2, c1,c2,c2,
	};
	
	drawVertices(Mesh::DRAW_TRIANGLES, vertices, colors, true);
}

//! (static)
void Draw::dropShadow(const Geometry::Rect & r1,const Geometry::Rect & r2, const Util::Color4ub c1) {
	const Util::Color4ub c2(0,0,0,0);
	/*
		r2_xy----------r2_Xy
		|                  |
		|   r1_xy--r1_Xy   |
		|   |          |   |
		|   |          |   |
		|   r1_xY--r1_XY   |
		|                  |
		r2_xY----------r2_XY
	
	*/
	const float r1_x = std::max( r1.getMinX(),r2.getMinX() );
	const float r1_X = std::min( r1.getMaxX(),r2.getMaxX() );
	const float r1_y = std::max( r1.getMinY(),r2.getMinY() );
	const float r1_Y = std::min( r1.getMaxY(),r2.getMaxY() );
	
	const std::vector<Geometry::Vec2> vertices = {
		/*r1_xy,r2_Xy,r2_xy*/ {r1_x,r1_y}, {r2.getMaxX(),r2.getMinY()}, {r2.getMinX(),r2.getMinY()},
		/*r1_xy,r1_Xy,r2_Xy*/ {r1_x,r1_y}, {r1_X,r1_y},                 {r2.getMaxX(),r2.getMinY()},
		/*r1_xy,r2_xy,r2_xY*/ {r1_x,r1_y}, {r2.getMinX(),r2.getMinY()}, {r2.getMinX(),r2.getMaxY()},
		/*r1_xy,r2_xY,r1_xY*/ {r1_x,r1_y}, {r2.getMinX(),r2.getMaxY()}, {r1_x,r1_Y},

		/*r1_XY,r2_XY,r2_Xy*/ {r1_X,r1_Y}, {r2.getMaxX(),r2.getMaxY()}, {r2.getMaxX(),r2.getMinY()},
		/*r1_XY,r2_Xy,r1_Xy*/ {r1_X,r1_Y}, {r2.getMaxX(),r2.getMinY()}, {r1_X,r1_y},
		/*r1_XY,r1_xY,r2_xY*/ {r1_X,r1_Y}, {r1_x,r1_Y},                 {r2.getMinX(),r2.getMaxY()},
		/*r1_XY,r2_xY,r2_XY*/ {r1_X,r1_Y}, {r2.getMinX(),r2.getMaxY()}, {r2.getMaxX(),r2.getMaxY()}
	};
	
	const std::vector<Util::Color4f> colors = {
		c1,c2,c2, c1,c1,c2, c1,c2,c2, c1,c2,c1,
		c1,c2,c2, c1,c2,c1, c1,c1,c2, c1,c2,c2
	};
	
	drawVertices(Mesh::DRAW_TRIANGLES, vertices, colors, true);
}

//! (static)
void Draw::drawTexturedTriangles(const std::vector<float> & posAndUV, const Util::Color4ub & c, bool blend/* = true*/) {
	std::vector<Geometry::Vec2> vertices;
	std::vector<Geometry::Vec2> uvs;
	vertices.reserve(posAndUV.size()/4);
	uvs.reserve(posAndUV.size()/4);
	for(uint32_t i=0; i<posAndUV.size(); i+=4) {
		vertices.emplace_back(posAndUV[i], posAndUV[i+1]);
		uvs.emplace_back(posAndUV[i+2], posAndUV[i+3]);
	}	
	drawTexturedVertices(Mesh::DRAW_TRIANGLES, vertices, uvs, c, blend);
}


//! (static)
void Draw::drawTexturedRect(const Geometry::Rect_i & screenRect,const Geometry::Rect & uvRect,const Util::Color4ub & c,bool blend/*=true*/) {
	Geometry::Rect r(screenRect);
	Geometry::Rect uv = uvRect;
	const std::vector<Geometry::Vec2> vertices = {
		{r.getMaxX(),r.getMinY()}, {r.getMinX(),r.getMinY()}, {r.getMinX(),r.getMaxY()},
		{r.getMinX(),r.getMaxY()}, {r.getMaxX(),r.getMaxY()}, {r.getMaxX(),r.getMinY()}
	};	
	const std::vector<Geometry::Vec2> uvs = {
		{uv.getMaxX(),uv.getMinY()}, {uv.getMinX(),uv.getMinY()}, {uv.getMinX(),uv.getMaxY()},
		{uv.getMinX(),uv.getMaxY()}, {uv.getMaxX(),uv.getMaxY()}, {uv.getMaxX(),uv.getMinY()}
	};
	drawTexturedVertices(Mesh::DRAW_TRIANGLES, vertices, uvs, c, blend);
}

//! (static)
void Draw::drawLine(const std::vector<float> & vertices,const std::vector<uint32_t> & colors,const float lineWidth/*=1.0*/,bool lineSmooth/*=false*/) {
	std::vector<Geometry::Vec2> vertices2;
	std::vector<Util::Color4f> colors2;
	vertices2.reserve(vertices.size()/2);
	colors2.reserve(colors.size());
	for(uint32_t i=0; i<vertices.size(); i+=2)
		vertices2.emplace_back(vertices[i], vertices[i+1]);
	for(uint32_t c : colors)
		colors2.emplace_back(Util::Color4ub(c));		
	drawVertices(Mesh::DRAW_LINE_STRIP, vertices2, colors2, true);
}

//! (static)
void Draw::drawLines(const std::vector<float> & vertices,const std::vector<uint32_t> & colors,const float lineWidth/*=1.0*/) {
	std::vector<Geometry::Vec2> vertices2;
	std::vector<Util::Color4f> colors2;
	vertices2.reserve(vertices.size()/2);
	colors2.reserve(colors.size());
	for(uint32_t i=0; i<vertices.size(); i+=2)
		vertices2.emplace_back(vertices[i], vertices[i+1]);
	for(uint32_t c : colors)
		colors2.emplace_back(Util::Color4ub(c));		
	drawVertices(Mesh::DRAW_LINES, vertices2, colors2, true);
}


//! @p vertices:  { x0,y0, x1,y1, x2,y2, ... } @p color {c0, c1, c2, ...}
void Draw::drawTriangleFan(const std::vector<float> & vertices,const std::vector<uint32_t> & colors) {
	std::vector<Geometry::Vec2> vertices2;
	std::vector<Util::Color4f> colors2;
	vertices2.reserve(vertices.size()/2);
	colors2.reserve(colors.size());
	for(uint32_t i=4; i<vertices.size(); i+=2) {
		vertices2.emplace_back(vertices[0], vertices[1]);
		vertices2.emplace_back(vertices[i-2], vertices[i-1]);
		vertices2.emplace_back(vertices[i], vertices[i+1]);
	}
	for(uint32_t i=2; i<colors.size(); ++i) {
		colors2.emplace_back(Util::Color4ub(colors[0]));
		colors2.emplace_back(Util::Color4ub(colors[i-1]));
		colors2.emplace_back(Util::Color4ub(colors[i]));
	}	
	drawVertices(Mesh::DRAW_TRIANGLES, vertices2, colors2, true);
}

//----------------------------------------------------------------------------------
// texture

void Draw::disableTexture() {
	ctxt.activeTexture = nullptr;
}
	
void Draw::enableTexture(Rendering::Texture* texture) {
	ctxt.activeTexture = texture;
}

//----------------------------------------------------------------------------------


}
