#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	_client = new ofxKhronosClient("localhost");
	_imgui.setup();
	_scaleReciever.setup(8000);
}
void ofApp::exit() {
	delete _client;
}
//--------------------------------------------------------------
void ofApp::update(){
	while (_scaleReciever.hasWaitingMessages()) {
		ofxOscMessage m;
		_scaleReciever.getNextMessage(m);
		if (m.getAddress() == "/scale") {
			_client->setScale(m.getArgAsDouble(0));
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw() {
	ofClear(0);

	double elapsed = _client->now();

	ofVec2f center;
	center.set(ofGetWidth() * 0.5, ofGetHeight() * 0.5);
	ofVec2f dir = ofVec2f(std::cosf(elapsed * 3.14 * 2.0), std::sinf(elapsed * 3.14 * 2.0));
	ofVec2f p = center + dir * 200.0f;

	ofDrawCircle(p, 30.0f);

	_imgui.begin();
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ofVec4f(0.0f, 0.2f, 0.2f, 0.5));
	ImGui::SetNextWindowPos(ofVec2f(10, 30), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ofVec2f(500, ofGetHeight() - 50), ImGuiSetCond_Once);

	ImGui::Begin("Client Panel");
	ImGui::Text("fps: %.2f", ofGetFrameRate());
	ImGui::Text("sync at least once: %s", _client->syncAtLeastOnce() ? "yes" : "no");
	ImGui::Text("time: %.2f", _client->now());
	ImGui::Text("scale: %.2f", _client->getScale());

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
