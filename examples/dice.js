//
//  dice.js
//  examples
//
//  Created by Philip Rosedale on February 2, 2015
//  Copyright 2015 High Fidelity, Inc.
//
//  Press the dice button to throw some dice from the center of the screen. 
//  Change NUMBER_OF_DICE to change the number thrown (Yahtzee, anyone?) 
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

var isDice = false; 
var NUMBER_OF_DICE = 2; 
var dice = [];
var DIE_SIZE = 0.20;

var madeSound = true;   //  Set false at start of throw to look for collision

HIFI_PUBLIC_BUCKET = "http://s3.amazonaws.com/hifi-public/";

var rollSound = SoundCache.getSound(HIFI_PUBLIC_BUCKET + "sounds/dice/diceRoll.wav");

var screenSize = Controller.getViewportDimensions();
var offButton = Overlays.addOverlay("image", {
                    x: screenSize.x - 48,
                    y: 96,
                    width: 32,
                    height: 32,
                    imageURL: HIFI_PUBLIC_BUCKET + "images/close.png",
                    color: { red: 255, green: 255, blue: 255},
                    alpha: 1
                });
var diceButton = Overlays.addOverlay("image", {
                    x: screenSize.x - 48,
                    y: 130,
                    width: 32,
                    height: 32,
                    imageURL: HIFI_PUBLIC_BUCKET + "images/die.png",
                    color: { red: 255, green: 255, blue: 255},
                    alpha: 1
                });

var GRAVITY = -3.5;
var LIFETIME = 300;
function shootDice(position, velocity) {
    for (var i = 0; i < NUMBER_OF_DICE; i++) {
        dice.push(Entities.addEntity(
        { type: "Model",
          modelURL: HIFI_PUBLIC_BUCKET + "models/props/Dice/goldDie.fbx",
          position: position,  
          velocity: velocity, 
          rotation: Quat.fromPitchYawRollDegrees(Math.random() * 360, Math.random() * 360, Math.random() * 360),
          // NOTE: angularVelocity is in radians/sec
          var maxAngularSpeed = Math.PI;
          angularVelocity: { x: Math.random() * maxAngularSpeed, y: Math.random() * maxAngularSpeed, z: Math.random() * maxAngularSpeed },
          lifetime: LIFETIME,
          gravity: {  x: 0, y: GRAVITY, z: 0 },
          shapeType: "box",
          collisionsWillMove: true
      }));
      position = Vec3.sum(position, Vec3.multiply(DIE_SIZE, Vec3.normalize(Quat.getRight(Camera.getOrientation()))));
    }
}

function deleteDice() {
    while(dice.length > 0) {
        Entities.deleteEntity(dice.pop()); 
    }
}

function entityCollisionWithEntity(entity1, entity2, collision) {
    if (!madeSound) {
        // Is it one of our dice? 
        for (var i = 0; i < dice.length; i++) {
            if (!dice[i].isKnownID) {
                dice[i] = Entities.identifyEntity(dice[i]);
            }
            if ((entity1.id == dice[i].id) || (entity2.id == dice[i].id)) {
                madeSound = true;
                Audio.playSound(rollSound, { position: collision.contactPoint });
            }
        }
        
    }
}

function mousePressEvent(event) {
    var clickedText = false;
    var clickedOverlay = Overlays.getOverlayAtPoint({x: event.x, y: event.y});
    if (clickedOverlay == offButton) {
        deleteDice();
    } else if (clickedOverlay == diceButton) {
        var HOW_HARD = 2.0;
        var position = Vec3.sum(Camera.getPosition(), Quat.getFront(Camera.getOrientation()));
        var velocity = Vec3.multiply(HOW_HARD, Quat.getFront(Camera.getOrientation()));
        shootDice(position, velocity); 
        madeSound = false; 
    } 
}

function scriptEnding() {
    deleteDice();
    Overlays.deleteOverlay(offButton);
    Overlays.deleteOverlay(diceButton);
   
}

Entities.entityCollisionWithEntity.connect(entityCollisionWithEntity);
Controller.mousePressEvent.connect(mousePressEvent);
Script.scriptEnding.connect(scriptEnding);
