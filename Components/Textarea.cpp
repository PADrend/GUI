/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "Textarea.h"
#include "../GUI_Manager.h"
#include "../Base/Draw.h"
#include "Scrollbar.h"
#include "ComponentPropertyIds.h"
#include "../Base/Layouters/ExtLayouter.h"

#include <Util/UI/Event.h>
#include <Util/Timer.h>
#include <Util/ObjectExtension.h>
#include <iostream>

/*
\todo
- tabs
- duplicate
- interface for search and replace
- invalid line range
- fixed size font support
- auto line break
- syntax highlighting / parser support
- row marker
- tabs-support

*/
namespace GUI {

// -----------------------------------------------------------------
//! (internal)
class SimpleTextProcessor : public TextareaTextProcessor{
public:
	/*
	class STPData{
			PROVIDES_TYPE_NAME_NV(STPData)
		public:
			std::vector<Util::Color4ub> lineMarker;
	};
	
	STPData& getData(Textarea&	ta)const{
		STPData * data = Util::getObjectExtension<STPData>(&ta);
		if(data==nullptr)
			data = Util::addObjectExtension<STPData>(&ta);
		return *data;
	}
	*/
	SimpleTextProcessor() = default;
	virtual ~SimpleTextProcessor(){}
	
	void displayText(Textarea& ta) override{
		
		auto & gui = ta.getGUI();
		const uint32_t lineHeight = ta._getLineHeight();
		const auto scrollPos = ta.getScrollPos();
		const Geometry::Rect localRect = ta.getLocalRect();

		if(ta.isSelected()) {
			// draw text selection
			if(ta.isTextSelected()) {
				const Textarea::cursor_t c1 = std::min(ta.getSelectionStart(),ta.getCursor());
				const Textarea::cursor_t c2 = std::max(ta.getSelectionStart(),ta.getCursor());
				const Geometry::Vec2 p1 = cursorToTextPos(ta,c1)-scrollPos;
				const Geometry::Vec2 p2 = cursorToTextPos(ta,c2)-scrollPos;

				if(c1.first == c2.first){ // same line
					gui.displayShape(PROPERTY_TEXTFIELD_TEXT_SELECTION_SHAPE,
											Geometry::Rect(p1.x(),p1.y(),p2.x()-p1.x(),lineHeight));
				}else{
					gui.displayShape(PROPERTY_TEXTFIELD_TEXT_SELECTION_SHAPE,
											Geometry::Rect(p1.x(),p1.y(),localRect.getWidth()-p1.x(),lineHeight));
					gui.displayShape(PROPERTY_TEXTFIELD_TEXT_SELECTION_SHAPE,
											Geometry::Rect(0,p2.y(),p2.x(),lineHeight));
					if(c2.first-c1.first>1){
						gui.displayShape(PROPERTY_TEXTFIELD_TEXT_SELECTION_SHAPE,
												Geometry::Rect(0,p1.y()+lineHeight,ta.getWidth(),p2.y()-p1.y()-lineHeight));
					}
				}
			}
			// draw cursor
			const Geometry::Vec2 c = cursorToTextPos(ta,ta.getCursor())-scrollPos;
			gui.displayShape(PROPERTY_TEXTFIELD_CURSOR_SHAPE,Geometry::Rect(c.x(),c.y(),1,lineHeight));

			// draw component selection
			Geometry::Rect rect=localRect;
			rect.moveRel(Geometry::Vec2(-1,-1));
			rect.changeSize(2,2);
			gui.displayShape(PROPERTY_SELECTION_RECT_SHAPE,rect);

		}
		const size_t firstLine = static_cast<size_t>(scrollPos.y() / lineHeight);
		const size_t endLine = std::min( firstLine+static_cast<size_t>(localRect.getHeight() / lineHeight)+1,ta.getNumberOfLines());

		//auto & data = getData(ta);
		for(size_t l = firstLine; l<endLine; ++l ){
			Draw::drawText(ta.getLine(l),cursorToTextPos(ta,std::make_pair(l,0))-scrollPos,
					ta._getActiveFont(),gui.getActiveColor(PROPERTY_TEXTFIELD_TEXT_COLOR));
//					ta._getActiveFont(),data.lineMarker[l]);
		}

		if(scrollPos.x()>0)
			gui.displayShape(PROPERTY_SCROLLABLE_MARKER_LEFT_SHAPE, localRect, 0);
		if(scrollPos.y()>0)
			gui.displayShape(PROPERTY_SCROLLABLE_MARKER_TOP_SHAPE, localRect, 0);
		if(cursorToTextPos(ta,std::make_pair(ta.getNumberOfLines(),0)).y()-scrollPos.y()>localRect.getHeight())
			gui.displayShape(PROPERTY_SCROLLABLE_MARKER_BOTTOM_SHAPE, localRect, 0);

	}
	void consolidateLines(Textarea&/*ta*/,const std::pair<size_t,size_t>&/*lines*/) override{
		/*auto & data = getData(ta);
		for(size_t l=lines.first;l<=lines.second && l<data.lineMarker.size();++l)
			data.lineMarker.at(l) = Util::Color4ub(ta.getLine(l).length()*10,0,0,255);*/
	}
	Geometry::Vec2 cursorToTextPos(const Textarea& ta,const Textarea::cursor_t &c) override{
		if(ta.getNumberOfLines()==0){
			return Geometry::Vec2(0,0);
		}
		const auto & line = ta.getLine(c.first);
		const float x = (line.empty()||c.second==0) ? 0 :
					Draw::getTextSize(line.substr(0,c.second),ta._getActiveFont()).width();

		return Geometry::Vec2(x,c.first*ta._getLineHeight());
	}
	Textarea::cursor_t textPosToCursor(const Textarea& ta,const Geometry::Vec2 &textPos)const override{
		if(ta.getNumberOfLines()==0||textPos.y()<0){
			return std::make_pair(0,0);
		}
		const size_t lineNr = std::min( static_cast<size_t>(textPos.y() / ta._getLineHeight()), ta.getNumberOfLines()-1);
		const std::string & line = ta.getLine(lineNr);
		
		for(size_t l=1;l<line.length(); ++l) {
			if( Draw::getTextSize(line.substr(0,l),ta._getActiveFont()).x() >= textPos.x())
				return std::make_pair(lineNr,l-1);
		}
		return std::make_pair(lineNr,line.length());
	}
	void onLinesInserted(Textarea& /*ta*/,size_t /*first*/,size_t /*number*/) override{
/*		auto & data = getData(ta);
		const Util::Color4ub b(0,0,0,0);
		data.lineMarker.insert(std::next(data.lineMarker.begin(),first),number,b);	*/
//		std::cout << "Lines inserted: "<<first<<" +"<<number<<"\n";
	}
	void onLineErased(Textarea& /*ta*/,size_t /*first*/,size_t /*number*/) override{
/*		auto & data = getData(ta);
		data.lineMarker.erase( std::next(data.lineMarker.begin(),first),std::next(data.lineMarker.begin(),first+number)); */
//		std::cout << "Lines erased: "<<first<<" -"<<number<<"\n";
	}
};

// -----------------------------------------------------------------

static const Util::StringIdentifier dataId_scrollPos("scroll");

//! (ctor)
Textarea::Textarea(GUI_Manager & _gui,flag_t _flags):
		Container(_gui,_flags),MouseButtonListener(),MouseMotionListener(),KeyListener(),lineHeight(15),
		selectionStart(std::make_pair(0,std::string::npos)), dataName("text"),dataChanged(false),activeTextUpdateIndex(0){
	fontReference = getGUI().getActiveFont(PROPERTY_DEFAULT_FONT);
	addMouseButtonListener(this);
	addKeyListener(this);
	setFlag(SELECTABLE,true);
	setFlag(USE_SCISSOR,true);
    
    setMouseCursorProperty(PROPERTY_MOUSECURSOR_TEXTFIELD);
    processor = new SimpleTextProcessor;
}


//! (dtor)
Textarea::~Textarea() {
	getGUI().removeMouseMotionListener(this);
	//dtor
}

//! ---|> Component
void Textarea::doDisplay(const Geometry::Rect & region) {
	getGUI().displayShape(PROPERTY_TEXTFIELD_SHAPE,getLocalRect());
	displayChildren(region,true);
	// update reference to actual font
	fontReference = getGUI().getActiveFont(PROPERTY_DEFAULT_FONT);
	lineHeight = fontReference->getLineHeight();
	processor->displayText(*this);
	
}

void Textarea::setText(const std::string & fullText) {
	invalidateRegion();
	_deleteText(range(std::make_pair(0,0),std::make_pair(getNumberOfLines()+1,0)));
	_insertText(std::make_pair(0,0),fullText);
	dataChanged = false;
	consolidate();
	moveCursor(std::make_pair(0,0));
	scrollTo(Geometry::Vec2(0,0));
}

std::string Textarea::getText()const {
	return getText(range(std::make_pair(0,0),std::make_pair(lines.size(),0)) );

}
const std::string & Textarea::getLine(uint32_t nr)const{
	static const std::string emptyString;
	return nr<lines.size() ? lines[nr] : emptyString;
}
float Textarea::getTextHeight()const{
	return processor->cursorToTextPos(*this,std::make_pair(lines.size(),0)).y();
}

void Textarea::moveCursor(const cursor_t & c,bool updateSelection){
	const cursor_t newCursor(std::min(c.first, 
									  static_cast<uint32_t>(lines.empty() ? 0 : lines.size() - 1)),
							 c.second);
	if(!updateSelection){
		selectionStart = newCursor;
	}
	invalidateRegion();
	cursor = newCursor;

	// scroll
	Geometry::Vec2 scroll = getScrollPos();
	const auto localCursorPos = processor->cursorToTextPos(*this,cursor)-scrollPos;
	if(localCursorPos.y() < 0 ){
		scroll.y( scroll.y()+localCursorPos.y());
	}else if(localCursorPos.y()+lineHeight>getHeight()){
		scroll.y(scroll.y()+(localCursorPos.y()-getHeight()+lineHeight));
	}
	if(localCursorPos.x() < 0 ){
		scroll.x( scroll.x()+localCursorPos.x());
	}else if(localCursorPos.x()+10>getWidth()){
		scroll.x(scroll.x()+(localCursorPos.x()-getWidth()+10));
	}
	scrollTo(scroll);
	
}

void Textarea::scrollTo(const Geometry::Vec2 & p){
	const Geometry::Vec2 newScroll(std::max(0.0f,p.x()),std::max(0.0f,std::min(getTextHeight()-getHeight(),p.y())));
	if(newScroll!=scrollPos){
		scrollPos = newScroll;
		if(scrollBar.isNotNull())
			scrollBar->setScrollPos( getScrollPos().y() );
		invalidateRegion();
	}
}


std::string Textarea::getText(const range_t & r)const{
	if(r.first >= r.second || lines.empty())
		return std::string();

	const size_t firstLineNr = std::min(lines.size() - 1,
										static_cast<std::size_t>(r.first.first)); // valid line
	const size_t firstLineChar = r.first.second; // may be beyond the line
	size_t lastLineNr = r.second.first;	// valid line
	size_t lastLineChar = r.second.second; // may be beyond the line
	if(lastLineNr>=lines.size()){
		lastLineNr = lines.size()-1;
		lastLineChar = std::string::npos;
	} 
	
	const std::string & firstLine = getLine(firstLineNr);

	if(firstLineNr == lastLineNr){ // only inside one line?
		return firstLineChar<firstLine.length() ? 
					firstLine.substr(firstLineChar,lastLineChar-firstLineChar) :
					std::string();
	}else{
		std::ostringstream result;
		if(firstLineChar<firstLine.length()) // first line
			result << firstLine.substr(firstLineChar);
		result << "\n";
		
		for(size_t l = firstLineNr+1; l<lastLineNr; ++l) // in between line
			result << getLine(l) << "\n"; 
		
		 // last line
		result << getLine(lastLineNr).substr(0, std::min(getLine(lastLineNr).length(),lastLineChar));
		return result.str();
	}
}
Textarea::cursor_t Textarea::_deleteText(const range_t & r){
	auto pBegin = r.first;
	const auto & pEnd = r.second;

	if(pBegin.second == std::string::npos)
		return pEnd;
	else if(pEnd.second == std::string::npos)
		return pBegin;
	else if(pBegin == pEnd)
		return pBegin;

	pBegin.second = std::min(getLine(pBegin.first).length(),pBegin.second);
	if(pBegin.first<lines.size()){
		const std::string before = getLine(pBegin.first).substr(0,pBegin.second);
		const std::string & lastLine = getLine(pEnd.first);
		const std::string after = pEnd.second<lastLine.length() ? lastLine.substr(pEnd.second) : "";

		lines.at(pBegin.first) = before + after;
		if(pEnd.first > pBegin.first){
			lines.erase(std::next(lines.begin(),pBegin.first+1),
							pEnd.first<lines.size() ? std::next(lines.begin(),pEnd.first+1) : lines.end() );
			processor->onLineErased(*this,pBegin.first,pEnd.first-pBegin.first);
		}
		markForConsolidation(pBegin.first,pBegin.first);
		dataChanged = true;
	}
	return pBegin;

}
	
	
Textarea::range_t Textarea::_insertText(const cursor_t & pos,const std::string & s){
	cursor_t endPos = pos;

	if(s.empty())
		return range(pos,endPos);
	
	invalidateRegion();
	dataChanged = true;

	if(pos.first>=lines.size()){ // pos pointing to the end?
		const size_t linesBefore = lines.size();
		lines.resize(pos.first+1); 
		processor->onLinesInserted(*this,linesBefore,lines.size()-linesBefore);
	}

	static const size_t delimiterLen = 1;
	size_t sCursor = s.find('\n');
	
	if(sCursor==std::string::npos){ // no newline? -> insert into current line
		const size_t p = std::min(getLine(pos.first).length(),pos.second);
		lines.at(pos.first).insert(p, s);
		endPos.second = p + s.length();
	}else{
		const std::string remainingLine = pos.second<getLine(pos.first).length() ? getLine(pos.first).substr(pos.second) : "";
		lines.at(pos.first) = getLine(pos.first).substr(0,pos.second) + s.substr(0,sCursor);
		sCursor += delimiterLen;
		
		std::vector<std::string> parts;
		while( true ){
			const size_t nextDelimiterPos = s.find('\n',sCursor);
			if(nextDelimiterPos == std::string::npos)
				break;
			parts.emplace_back(s.substr(sCursor,nextDelimiterPos-sCursor));
			sCursor = nextDelimiterPos+delimiterLen;
		}
		
		if(sCursor<s.length()){ // insert the remaining part
			
			parts.emplace_back(s.substr(sCursor)+remainingLine);
			endPos.second = s.length()-sCursor;
		}else{
			parts.emplace_back(remainingLine);
			endPos.second = 0;
		}
		if(!parts.empty()){
			lines.insert(std::next(lines.begin(),pos.first+1),parts.begin(),parts.end());
			processor->onLinesInserted(*this,pos.first,parts.size());
			endPos.first+=parts.size();
		}
	}
	markForConsolidation(pos.first,endPos.first);
	return range(pos,endPos);
}

//! ---|> KeyListener
bool Textarea::onKeyEvent(Component * /*component*/, const Util::UI::KeyboardEvent & keyEvent) {
	if(!keyEvent.pressed)
		return true;

	if(isLocked())
		return false;

	getGUI().enableKeyRepetition(keyEvent);
	const bool shiftPressed = getGUI().isShiftPressed();
	const bool ctrlPressed = getGUI().isCtrlPressed();
	if(keyEvent.key == Util::UI::KEY_A && ctrlPressed) { // select all
		moveCursor(std::make_pair(0,0));
		moveCursor(std::make_pair(lines.size(),std::string::npos-1),true);
		return true;
	}
	else if(keyEvent.key == Util::UI::KEY_TAB) {
		if(!shiftPressed)
			return false;
		executeTextUpdate( range(cursor, selectionStart), "\t" );
	} else if(keyEvent.key == Util::UI::KEY_RIGHT ) {
		if( !getLine(cursor.first).empty() && cursor.second<getLine(cursor.first).length() ){
			moveCursor(std::make_pair(cursor.first,cursor.second+1),shiftPressed);
		}else if(cursor.first<lines.size()-1){
			moveCursor(std::make_pair(cursor.first+1,0),shiftPressed);
		}
	} else if(keyEvent.key == Util::UI::KEY_LEFT) {
		if(cursor.second>0 && !getLine(cursor.first).empty()){
			moveCursor(std::make_pair(cursor.first, std::min(getLine(cursor.first).length(),cursor.second)-1),shiftPressed);
		}else if(cursor.first>0){
			moveCursor(std::make_pair(cursor.first-1,getLine(cursor.first-1).length()),shiftPressed);
		}
	} else if(keyEvent.key == Util::UI::KEY_UP) {
		if(cursor.first>0){
			moveCursor(std::make_pair(cursor.first-1,cursor.second),shiftPressed);
		}
	} else if(keyEvent.key == Util::UI::KEY_DOWN) {
		moveCursor(std::make_pair(cursor.first+1,cursor.second),shiftPressed);
	} else if(keyEvent.key == Util::UI::KEY_END) {
		moveCursor(std::make_pair(cursor.first,std::string::npos-1),shiftPressed);
	} else if(keyEvent.key == Util::UI::KEY_HOME) {
		moveCursor(std::make_pair(cursor.first,0),shiftPressed);
	} else if(keyEvent.key == Util::UI::KEY_PAGEUP) {
		moveCursor(std::make_pair(std::max(0,static_cast<int>(cursor.first-getHeight()/lineHeight+2)),cursor.second),shiftPressed);
	} else if(keyEvent.key == Util::UI::KEY_PAGEDOWN) {
		moveCursor(std::make_pair(cursor.first+getHeight()/lineHeight-2,cursor.second),shiftPressed);
	} else if(keyEvent.key == Util::UI::KEY_DELETE) {
		cursor_t pos2;
		if(isTextSelected()) {
			pos2 = selectionStart;
		} else if(cursor.second+1<getLine(cursor.first).length()){
			pos2 = std::make_pair(cursor.first,cursor.second+1);
		}else{
			pos2 = std::make_pair(cursor.first+1,0);
		}
		executeTextUpdate(range(cursor, pos2), "");
	} else if(keyEvent.key == Util::UI::KEY_BACKSPACE) {
		cursor_t pos2;
		if(isTextSelected()) {
			pos2 = selectionStart;
		} else if(cursor.second>0){
			pos2 = std::make_pair(cursor.first,cursor.second-1);
		} else if(cursor.first>0){
			pos2 = std::make_pair(cursor.first-1,std::string::npos-1);
		} else {
			return true;
		}
		executeTextUpdate(range(cursor, pos2), "");
	} else if(keyEvent.key == Util::UI::KEY_ESCAPE) {
		unselect();
	} else if(keyEvent.key == Util::UI::KEY_REDO || (keyEvent.key == Util::UI::KEY_Z && ctrlPressed && shiftPressed)) {
		redoTextUpdate();
	} else if(keyEvent.key == Util::UI::KEY_UNDO || (keyEvent.key == Util::UI::KEY_Z && ctrlPressed && !shiftPressed)) { 
		undoTextUpdate();
	} else if(keyEvent.key == Util::UI::KEY_RETURN) { 
		executeTextUpdate(range(cursor, selectionStart), "\n");
	} else if(keyEvent.key == Util::UI::KEY_C && ctrlPressed) {
		if(isTextSelected()) {
			getGUI().copyStringToClipboard(getText(range(selectionStart,cursor)));
		}
	} else if(keyEvent.key == Util::UI::KEY_X && ctrlPressed) {
		if(isTextSelected()) {
			getGUI().copyStringToClipboard(getText(range(selectionStart,cursor)));
			executeTextUpdate(range(cursor, selectionStart), "");
		}
	} else if(keyEvent.key == Util::UI::KEY_V && ctrlPressed) {
		executeTextUpdate(range(cursor, selectionStart), getGUI().getStringFromClipboard());
//	}
//	// does not work... No idea why...
////     else if(ke->getChar() == 'z' &&ke->getModifier()&KMOD_CTRL  ) {
////        text=backupText;
////        moveCursor(0);
////    }
	} else if(keyEvent.str[0] >= 32) {
		
		if(!commands.empty() && commands.back().extendable && commands.back().getInsertionCursor()==cursor){
			commands.back().extend(*this,std::string(keyEvent.str, 1));
		}else {
			executeTextUpdate( range(cursor, selectionStart), std::string(keyEvent.str, 1));
			commands.back().extendable = true;
		}
	}
	consolidate();
	return true;
}

//! ---|> MouseButtonListener
listenerResult_t Textarea::onMouseButton(Component * /*component*/, const Util::UI::ButtonEvent & buttonEvent){
	if(buttonEvent.pressed && !isLocked()) {
		select();
		const Geometry::Vec2 localPos = Geometry::Vec2(buttonEvent.x, buttonEvent.y) - getAbsPosition();
		if(buttonEvent.button == Util::UI::MOUSE_BUTTON_LEFT) {
			moveCursor(processor->textPosToCursor(*this,localPos+scrollPos), getGUI().isShiftPressed());
			getGUI().addMouseMotionListener(this);
		}else if(buttonEvent.button == Util::UI::MOUSE_BUTTON_MIDDLE) {
			getGUI().addMouseMotionListener(this);
		}else if(buttonEvent.button == Util::UI::MOUSE_WHEEL_UP ) { // scroll
			scrollTo( getScrollPos()-Geometry::Vec2(0,3.0f*lineHeight) );
		}else if(buttonEvent.button == Util::UI::MOUSE_WHEEL_DOWN) { // scroll
			scrollTo( getScrollPos()+Geometry::Vec2(0,3.0f*lineHeight) );
		}else{
			return LISTENER_EVENT_NOT_CONSUMED;
		}
		return LISTENER_EVENT_CONSUMED;
	}
	return LISTENER_EVENT_NOT_CONSUMED;

}
//
//! ---|> MouseMotionListener
listenerResult_t Textarea::onMouseMove(Component * /*component*/, const Util::UI::MotionEvent & motionEvent){
	if((motionEvent.buttonMask & Util::UI::MASK_MOUSE_BUTTON_LEFT) && !isLocked()) {
		const Geometry::Vec2 localPos = Geometry::Vec2(motionEvent.x, motionEvent.y) - getAbsPosition();
		moveCursor(processor->textPosToCursor(*this,localPos+scrollPos), true);
		return LISTENER_EVENT_CONSUMED;
	}else if((motionEvent.buttonMask & Util::UI::MASK_MOUSE_BUTTON_MIDDLE)) {
		const Geometry::Vec2 delta(motionEvent.deltaX, motionEvent.deltaY);
		scrollTo( getScrollPos() - delta*2.0 );
		return LISTENER_EVENT_CONSUMED;
	}
	return LISTENER_EVENT_NOT_CONSUMED_AND_REMOVE_LISTENER;
}

//! ---|> Component
bool Textarea::onSelect() {
	return true;
}

//! ---|> Component
bool Textarea::onUnselect() {
	getGUI().removeMouseMotionListener(this);
	if(dataChanged) { 
		dataChanged = false;
		getGUI().componentDataChanged(this,dataName);
	}

	return true;
}
//! ---|> DataChangeListener
void Textarea::handleDataChange(Component *, const Util::StringIdentifier & actionName) {
	if(scrollBar.isNotNull() && actionName==dataId_scrollPos){
		scrollTo( Geometry::Vec2(getScrollPos().x(),scrollBar->getScrollPos() ));
	}
}



//-------------------------------------------------------------------------------------------
// TextUpdate

void Textarea::TextUpdate::execute(Textarea& ta){
	oldText = ta.getText(r1);
	Textarea::cursor_t position = ta._deleteText(r1);
	r2 = ta._insertText(position,newText);
	ta.moveCursor(r2.second);
}
void Textarea::TextUpdate::undo(Textarea& ta){
	Textarea::cursor_t position = ta._deleteText(r2);
	auto range = ta._insertText(position,oldText);
	ta.moveCursor(range.second);
}
void Textarea::TextUpdate::extend(Textarea& ta,const std::string & s){
	undo(ta);
	newText+=s;
	execute(ta);
}

void Textarea::consolidate(){
	if(linesToConsolidate.first>linesToConsolidate.second)
		return;
	processor->consolidateLines(*this,linesToConsolidate); // update syntax highlighting, line breaks, etc.
	linesToConsolidate.first = lines.size()+1;
	linesToConsolidate.second = 0;
	
	updateScrollPos();
}
	
void Textarea::updateScrollPos(){
	const int maxScrollPos = getTextHeight()-getHeight();
//	std::cout << "MaxScrollPos"<<maxScrollPos<<"\n";
	if(maxScrollPos>0){
		if(scrollBar.isNull()){
			scrollBar = new Scrollbar(getGUI(), dataId_scrollPos, Scrollbar::VERTICAL);
			scrollBar->addDataChangeListener(this);
			scrollBar->setExtLayout( 	ExtLayouter::POS_X_ABS|ExtLayouter::REFERENCE_X_RIGHT|ExtLayouter::ALIGN_X_RIGHT|
						ExtLayouter::POS_Y_ABS|ExtLayouter::REFERENCE_Y_TOP|ExtLayouter::ALIGN_Y_TOP |
						ExtLayouter::WIDTH_ABS|ExtLayouter::HEIGHT_ABS,
						Geometry::Vec2(1,2),Geometry::Vec2(getGUI().getGlobalValue(PROPERTY_SCROLLBAR_WIDTH),-4));
			_addChild(scrollBar.get());
		}
		scrollBar->setMaxScrollPos(maxScrollPos);
		scrollBar->setScrollPos( getScrollPos().y() );
	}else{
		 if(scrollBar.isNotNull()){
			getGUI().markForRemoval(scrollBar.get());
			scrollBar = nullptr;
		}
		if(getScrollPos().y()>0){
			scrollTo( Geometry::Vec2(getScrollPos().x(),0 ));
		}
	
	}
}

void Textarea::executeTextUpdate(Textarea::range_t _r1,const std::string & text){
	if(activeTextUpdateIndex<commands.size())
		commands.resize(activeTextUpdateIndex);
	if(!commands.empty())
		commands.back().extendable = false;
	commands.emplace_back(_r1,text);
	activeTextUpdateIndex = commands.size();
	commands.back().execute(*this);
}
void Textarea::redoTextUpdate(){
	if(activeTextUpdateIndex<commands.size()){
		commands.at(activeTextUpdateIndex).execute(*this);
		commands.at(activeTextUpdateIndex).extendable = false;
		++activeTextUpdateIndex;
	}
}
void Textarea::undoTextUpdate(){
	if(activeTextUpdateIndex>0){
		--activeTextUpdateIndex;
		commands.at(activeTextUpdateIndex).undo(*this);
		commands.at(activeTextUpdateIndex).extendable = false;
	}
}

}
