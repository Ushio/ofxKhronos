#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	_imgui.setup();
	_server = new ofxKhronosServer();
	_scaleSender.setup("localhost", 8000);
}
void ofApp::exit() {
	delete _server;
}
//--------------------------------------------------------------
void ofApp::update() {

}

//--------------------------------------------------------------
void ofApp::draw() {
	ofClear(0);
	double elapsed = _server->now();

	ofVec2f center;
	center.set(ofGetWidth() * 0.5, ofGetHeight() * 0.5);
	ofVec2f dir = ofVec2f(std::cosf(elapsed * 3.14 * 2.0), std::sinf(elapsed * 3.14 * 2.0));
	ofVec2f p = center + dir * 200.0f;

	ofDrawCircle(p, 30.0f);

	_imgui.begin();
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ofVec4f(0.0f, 0.2f, 0.2f, 1.0f));
	ImGui::SetNextWindowPos(ofVec2f(10, 30), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ofVec2f(500, ofGetHeight() - 50), ImGuiSetCond_Once);

	ImGui::Begin("Server Panel");
	ImGui::Text("fps: %.2f", ofGetFrameRate());
	ImGui::Text("time: %.2f", _server->now());

	auto sendScale = [=]() {
		ofxOscMessage m;
		m.setAddress("/scale");
		m.addDoubleArg(_server->getScale());
		_scaleSender.sendMessage(m);
	};
	if (ImGui::SliderFloat("timescale", &_timescale, -1.0f, 2.0f)) {
		_server->setScale(_timescale);
		sendScale();
	}
	if (ofGetFrameNum() % 60 == 0) {
		sendScale();
	}

	ImGui::End();
	ImGui::PopStyleColor();

	_imgui.end();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
