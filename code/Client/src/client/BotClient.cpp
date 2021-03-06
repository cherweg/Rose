/*
 * BotClient.cpp
 *
 *  Created on: Mar 14, 2018
 *      Author: JochenAlt
 */

#include <client/BotClient.h>
#include <client/HttpConnection.h>
#include <ui/WindowController.h>


BotClient::BotClient() {
}

BotClient::~BotClient() {
}


BotClient& BotClient::getInstance() {
	static BotClient instance;
	return instance;
}


void BotClient::setup(string host, int port) {

	// webclient is ready
	isWebClientActive = true;

	// establish TCP/IP connect (takes a couple of seconds)
	cmdConn.setup(host, port);
	statusConn.setup(host, port);

	// msuic is detected on the server side
	musicDetected = false;

	bodyPose.null();
	headPose.null();

	// ambition is defined on the server side
	ambition = 0;

	// we do not move during startup
	move = Move::MoveType::NO_MOVE;
}


string BotClient::get(HttpConnection& conn, string requestUrl, bool& ok) {
	int httpStatus = -1;
	string httpResponse;
	conn.get(requestUrl, httpResponse, httpStatus);
	if (httpStatus != 200) {
		std::ostringstream statusStr;
		statusStr << "request " << requestUrl << " returned http code " << httpResponse;
		WindowController::getInstance().setStatus(statusStr.str());
		ok = false;
	} else {
		ok = true;
	}
	return httpResponse;
}

string BotClient::post(HttpConnection& conn, string requestUrl, const string& httpBody, bool& ok) {
	int httpStatus = -1;
	string httpResponse;
	conn.post(requestUrl, httpBody, httpResponse, httpStatus);
	if (httpStatus != 200) {
		std::ostringstream statusStr;
		statusStr << "request " << requestUrl << " returned http code " << httpResponse ;
		WindowController::getInstance().setStatus(statusStr.str());

		ok = false;
	} else {
		ok = true;
	}
	return httpResponse;

}

void BotClient::getStatus() {
	bool ok;
	string httpResponse = get(statusConn, "/status", ok);
	if (ok) {
		std::istringstream in(httpResponse);
		parseCharacter(in, '{', ok);
		parseString(in, ok); // "response"
		parseCharacter(in, ':', ok);
		parseCharacter(in, '{', ok);
		parseString(in, ok); // "body"
		parseCharacter(in, ':', ok);

		bodyPose.deserialize(in, ok);
		parseCharacter(in, ',', ok);
		parseString(in, ok); // "head"
		parseCharacter(in, ':', ok);

		headPose.deserialize(in, ok);
		parseCharacter(in, ',', ok);
		parseString(in, ok); // "ambition"
		parseCharacter(in, ':', ok);
		ambition = parseFloat(in, ok);
		parseCharacter(in, ',', ok);
		parseString(in, ok); // "move"
		parseCharacter(in, ':', ok);
		int moveTmp = parseInt(in, ok);
		move = (Move::MoveType)moveTmp;
		parseCharacter(in, ',', ok);
		parseString(in, ok); // "music"
		parseCharacter(in, ':', ok);
		musicDetected = parseBool(in, ok);
		parseCharacter(in, '}', ok);
		parseCharacter(in, ',', ok);
		parseString(in, ok); // "auto"
		parseCharacter(in, ':', ok);
		sequenceMode= parseInt(in, ok);

		parseCharacter(in, '}', ok);
		parseCharacter(in, ',', ok);
		parseString(in, ok); // "status"
		parseCharacter(in, ':', ok);
		parseBool(in, ok);

		parseCharacter(in, '}', ok);
	}
}


void BotClient::setMoveMode(Dancer::SequenceModeType moveMode) {
	std::ostringstream request;
	request << "/console/movemode?value=" << (int) moveMode;

	bool ok;
	string httpResponse = get(cmdConn, request.str(), ok);
	if (ok) {
		std::istringstream in(httpResponse);
		parseCharacter(in, '{', ok);
		parseString(in, ok); // "status="
		parseCharacter(in, ':', ok);
		parseBool(in, ok);
		parseCharacter(in, '}', ok);
	}
}

// set the current move
void BotClient::setMove(Move::MoveType move) {
	std::ostringstream request;
	request << "/console/move?value=" << (int) move;
	bool ok;
	string httpResponse = get(cmdConn, request.str(), ok);
	if (ok) {
		std::istringstream in(httpResponse);
		parseCharacter(in, '{', ok);
		parseString(in, ok); // "status="
		parseCharacter(in, ':', ok);
		parseBool(in, ok);
		parseCharacter(in, '}', ok);
	}
}

// set the current amplitude
void BotClient::setAmbition(float ambition) {
	std::ostringstream request;
	request << "/console/ambition?value=" << std::fixed << std::setprecision(3) << ambition;
	bool ok;
	string httpResponse = get(cmdConn, request.str(), ok);
	if (ok) {
		std::istringstream in(httpResponse);
		parseCharacter(in, '{', ok);
		parseString(in, ok); // "status="
		parseCharacter(in, ':', ok);
		parseBool(in, ok);
		parseCharacter(in, '}', ok);
	}
}

// play this .wav file
void BotClient::setWavFile(string name, string wavContent) {
	bool ok;
	std::ostringstream request;
	request << "/console/song?name=" <<name;

	string httpResponse = post(cmdConn, request.str(), wavContent, ok);
	if (ok) {
		std::istringstream in(httpResponse);
		parseCharacter(in, '{', ok);
		parseString(in, ok); // "status="
		parseCharacter(in, ':', ok);
		parseBool(in, ok);
		parseCharacter(in, '}', ok);
	}

}










