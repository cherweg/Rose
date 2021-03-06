/*
 * DanceMove.h
 *
 *  Created on: Feb 16, 2018
 *      Author: jochenalt
 */

#ifndef SRC_MOVE_H_
#define SRC_MOVE_H_

#include "basics/spatial.h"
#include <string>
#include <vector>


const double bodyHeight = 100.0;
const double headHeight = 105.0;

class Move {
public:
	enum MoveType { NO_MOVE, LISTENING, PHYSICISTS_HEAD_NICKER, TENNIS_HEAD_NICKER, WEASELS_MOVE,
		            TRAVOLTA_HEAD_NICKER, ENHANCED_TRAVOLTA_HEAD_NICKER,
					EYED_DIPPED_DIAGONAL_HEAD_SWING,
					BELLY_MOVE, BOLLYWOOD_HEAD_MOVE, SWING_DOUBLE_BOLLYWOOD_MOVE,
					BODY_WAVE, DIPPED_BODY_WAVE, SIDE_DIPPED_BODY_WAVE,
					SHIMMYS, TRIPPLE_SHIMMYS, LEANING_TRIPPLE_SHIMMYS,
					SHOULDER_MOVE,SHOULDER_CIRCLE, SHOULDER_DIP,
					TURN_AND_SHOW_BACK, TWERK, TURN_BACK,
					LAST_MOVE
					};

	static int numMoves() { return (int) LAST_MOVE-1; };

	Move() {};
	Move(const Move& p) {
		id = p.id;
		name = p.name;
		lengthInBeats = p.lengthInBeats;
	};

	Move(MoveType newId, string newName, int newLengthInBeats) {
		id = newId;
		name = newName;
		lengthInBeats = newLengthInBeats;
	};

	static void setup();

	~Move() {};
	void operator= (const Move & p) {
		id = p.id;
		name = p.name;
		lengthInBeats = p.lengthInBeats;
	}
	bool operator== (const Move & p) {
		return p.id == id;
	}

	static Move& getMove(MoveType i);

	Pose getDefaultHeadPose() {
		return Pose (Point(0,0,headHeight), Rotation(0,0,0));
	}
	string getName() { return name; };
	MoveType getMoveType() { return id; };
	int getLength() { return lengthInBeats; };

	double scaleMove(double movePercentage, double speedFactor, double phase);
	double baseCurveCos(double movePercentage);
	double baseCurveFatCos(double movePercentage);
	double baseCurveTriangle(double movePercentage);
	double baseCurveTrapezoid(double movePercentage);
	double baseCurveDip(double movePercentage);
	double baseCurveFatDip(double movePercentage);
	double baseCurveRectangle(double movePercentage);
	double baseCurveSharpRectangle(double movePercentage);


	double scaleAmbition(double value);

	static TotalBodyPose absHead (const Pose& bodyPose, const Pose& relHeadPose);
	// methods implementing dance moves
	TotalBodyPose listeningMove(double movePercentage);

	TotalBodyPose physicistsHeadNicker(double movePercentage);
	TotalBodyPose tennisHeadNicker(double movePercentage);

	TotalBodyPose travoltaHeadNicker(double movePercentage);
	TotalBodyPose enhancedTravoltaHeadNicker(double movePercentage);
	TotalBodyPose weaselsMove(double movePercentage);

	TotalBodyPose eyedDippedDiagonalSwing(double movePercentage);

	TotalBodyPose bollywoodHeadMove(double movePercentage);
	TotalBodyPose bellySwingingMove(double movePercentage);
	TotalBodyPose swingDoubleBollywoodHeadMove(double movePercentage);

	TotalBodyPose bodyWaveMove(double movePercentage);
	TotalBodyPose dipBodyWaveMove(double movePercentage);
	TotalBodyPose sidedDipBodyWaveMove(double movePercentage);

	TotalBodyPose shimmys(double movePercentage);
	TotalBodyPose trippleShimmys(double movePercentage);
	TotalBodyPose leaningTrippleShimmys(double movePercentage);

	TotalBodyPose shoulderMove(double movePercentage);
	TotalBodyPose shoulderCircle(double movePercentage);
	TotalBodyPose shoulderDipMove(double movePercentage);

	TotalBodyPose turnAndShowBack(double movePercentage);
	TotalBodyPose twerk(double movePercentage);
	TotalBodyPose turnBack(double movePercentage);

	TotalBodyPose move(double movePercentage);
	MoveType id = NO_MOVE;
	string name;
	int lengthInBeats;
	static std::vector<Move> moveLibrary;
};

#endif /* SRC_MOVE_H_ */
