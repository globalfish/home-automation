#
# demo to get webcam images from a Mac (possibly a PC?) and into AWS Rekognition
# Ideally, this will get triggered by an external event, e.g. a sensor that detects
# motion, a doorbell
#
# Author: vswamina, April 2017.
# 2017_08_06 Updated with multiple changes for sharing

# With a lot of help from the Internet
import cv2
import boto3
import json
import gallery
import threading
import RPi.GPIO as GPIO
from espeak import espeak
import time
from random import *
from picamera.array import PiRGBArray
from picamera import PiCamera

#
# if you have multiple cameras on your device then just use appropriate device
# if you have just one camera on your device then DEFAULT_CAM should work
DEFAULT_CAM = 0
REAR_CAM = 1
XTNL_CAM = 2
flywheelPin = 14 #broadcom pin to start flywheel
triggerPin = 15

#
# colors to use for bounding box and text
RED = 0, 0, 255
BLUE = 255, 0, 0
GREEN = 0, 255, 0
WHITE = 255,255,255
BLACK=10,10,10

# which camera do we use
camera_port = DEFAULT_CAM

# setup AWS Rekognition client
client = boto3.client('rekognition')

GPIO.setmode(GPIO.BCM)
GPIO.setup(flywheelPin, GPIO.OUT)
GPIO.setup(triggerPin, GPIO.OUT)
GPIO.output(flywheelPin, GPIO.HIGH)
GPIO.output(triggerPin, GPIO.HIGH)

#
# load images from S3 bucket and create a gallery of known faces
#
s3client = boto3.client("s3")

#
# In order to avoid buffering, we read the camera images
# in a separate thread and then get images as needed

class VideoStreamFromCamera:
    def __init__(self, src=camera_port):

        # setup camera to capture
        self.camera = cv2.VideoCapture(camera_port)

        #
        # setting lower capture resolution for processing speed using USB webcam
        self.camera.set(3, 320)
        self.camera.set(4, 240)
        
        (self.grabbed, self.frame) = self.camera.read()

        # default color for bounding box
        self.color = RED

        #default coordinates for bounding box
        self.boxTopLeftX = 0
        self.boxTopLeftY = 0
        self.boxBotRightX = 0
        self.boxBotRightY = 0
        self.personName = "UNKNOWN"

        # default font for name
        self.font = cv2.FONT_HERSHEY_SIMPLEX

        # which classifier to use
        self.faceCascade = cv2.CascadeClassifier('haarcascade_frontalface_default.xml')
        self.faces=[] # initialize else we get an error in self.readFaces
        self.foundFaces = False
 
        # should thread run or stop
        self.stopped = False

    def start(self):
        # start thread to read frame
        threading.Thread(target=self.update, args=()).start()
        return self

    def update(self):
        # read camera
        while True:
            if self.stopped:
                return
            (self.grabbed, self.frame) = self.camera.read()
            self.drawRect(self.boxTopLeftX, self.boxTopLeftY,
                          self.boxBotRightX, self.boxBotRightY,
                          self.color)
      
            # detect faces
            self.faces = self.faceCascade.detectMultiScale(
                self.frame,
                scaleFactor = 1.1,
                minNeighbors = 5,
                minSize = (20, 20)
                )
            # draw bounding box
            for (x,y,w,h) in self.faces:
                self.drawRect(x, y, x+w, y+h, self.color)
                cv2.putText(self.frame, self.personName, (x, y-10), self.font, 0.5, self.color, 2)
                
            if( len(self.faces) > 0):
                self.foundFaces = True
            else:
                self.foundFaces = False

                # draw help text
            cv2.putText(self.frame, "Press <S> to Save", (2, 10), self.font, 0.5, BLACK, 1)
            cv2.imshow('Rekognition', self.frame)        
            key = cv2.waitKey(1)
            if( 's' == chr(key & 255) or 'S' == chr(key & 255)):
                # save to S3
                _, imgFile = cv2.imencode(".png",self.frame)
                response = s3client.put_object(
                    ACL='private',
                    Body=imgFile.tobytes(),
                    Bucket="com.vswamina.aws.test.images",
                    Key="unknown"+str(randint(100,10000))
                    )

            cv2.imshow('Rekognition', self.frame)        
            cv2.waitKey(1)

    def drawRect(self, x1, y1, x2, y2, color):
        cv2.rectangle(self.frame, (x1, y1), (x2, y2), color, 2)
        return

    
    def read(self):
        return self.frame

    def readFaces(self):
        return self.faces
        
    def foundFacesInFrame(self):
        return self.foundFaces

    def stop(self):
        self.stopped = True
        cv2.destroyAllWindows()
        del self.camera

    def setColor(self, color):
        self.color = color
        
    def setBoundingBox(self, x1,y1,x2,y2):
        self.boxTopLeftX = x1
        self.boxTopLeftY = y1
        self.boxBotRightX = x2
        self.boxBotRightY = y2

    def setName(self, name):
        self.personName = name

class PiVideoStream:
    def __init__(self, resolution=(320,240), framerate=32):
        self.camera = PiCamera()
        self.camera.resolution = resolution
        self.camera.framerate = framerate
        self.rawCapture = PiRGBArray(self.camera, size=resolution)
        self.stream = self.camera.capture_continuous(
        self.rawCapture, format="bgr", use_video_port=True)

        # default color for bounding box
        self.color = RED

        #default coordinates for bounding box
        self.boxTopLeftX = 0
        self.boxTopLeftY = 0
        self.boxBotRightX = 0
        self.boxBotRightY = 0
        self.personName = "UNKNOWN"

        # default font for name
        self.font = cv2.FONT_HERSHEY_SIMPLEX

        # which classifier to use
        self.faceCascade = cv2.CascadeClassifier('haarcascade_frontalface_default.xml')
        self.faces=[] # initialize else we get an error in self.readFaces
        self.foundFaces = False
 
        # should thread run or stop
        self.stopped = False

        self.frame= None

    def start(self):
        threading.Thread(target=self.update, args=()).start()
        return self

    def update(self):
        for f in self.stream:

            self.frame=f.array
            self.rawCapture.truncate(0)

            self.drawRect(self.boxTopLeftX, self.boxTopLeftY,
                self.boxBotRightX, self.boxBotRightY,
                self.color)
      
            # detect faces
            self.faces = self.faceCascade.detectMultiScale(
                self.frame,
                scaleFactor = 1.1,
                minNeighbors = 5,
                minSize = (20, 20)
                )
            # draw bounding box
            for (x,y,w,h) in self.faces:
                self.drawRect(x, y, x+w, y+h, self.color)
                cv2.putText(self.frame, self.personName, (x, y-10), self.font, 0.5, self.color, 2)
                
            if( len(self.faces) > 0):
                self.foundFaces = True
            else:
                self.foundFaces = False

            # draw help text
            cv2.putText(self.frame, "Press <S> to Save", (2, 10), self.font, 0.5, BLACK, 1)
            cv2.imshow('Rekognition', self.frame)        
            key = cv2.waitKey(1)
            if( 's' == chr(key & 255) or 'S' == chr(key & 255)):
                # save to S3
                _, imgFile = cv2.imencode(".png",self.frame)
                response = s3client.put_object(
                    ACL='private',
                    Body=imgFile.tobytes(),
                    Bucket="com.vswamina.aws.test.images",
                    Key="unknown"+str(randint(100,10000))
                    )

            cv2.imshow('Rekognition', self.frame)        
            cv2.waitKey(1)

            if self.stopped:
                self.stream.close()
                self.rawCapture.close()
                self.camera.close()
                return

            
    def drawRect(self, x1, y1, x2, y2, color):
        cv2.rectangle(self.frame, (x1, y1), (x2, y2), color, 2)
        return

    
    def read(self):
        return self.frame

    def readFaces(self):
        return self.faces
        
    def foundFacesInFrame(self):
        return self.foundFaces

    def setColor(self, color):
        self.color = color
        
    def setBoundingBox(self, x1,y1,x2,y2):
        self.boxTopLeftX = x1
        self.boxTopLeftY = y1
        self.boxBotRightX = x2
        self.boxBotRightY = y2

    def setName(self, name):
        self.personName = name

    def stop(self):
        self.stopped = True
        
#
# Thread for voice prompts. Run in separate thread to avoid blocking main thread
# and also to prevent irritating repetition and chatter
class VoicePrompts:
    def __init__(self, threshold=2):

        timeThreshold = 2 # 2 seconds between prompts
        espeak.synth("Voice system initialized")
        #
        # We store the previous phrase to avoid repeating the same phrase
        # If new phrase is the same as previous phrase do nothing
        self.phrase = None
        self.oldPhrase = None
        
        self.stopped = False
        self.threshold=threshold
        time.sleep(threshold)
        
    def start(self):
        threading.Thread(target=self.speak, args=()).start()
        return self

    def speak(self):
        while True:

            if( self.stopped):
                return
            
            if( not self.phrase == None):
                if( not self.phrase == self.oldPhrase):
                    espeak.synth(self.phrase)
                    self.oldPhrase = self.phrase
                
                # sleep thread for duration to allow gap between voice prompts
                time.sleep(self.threshold)


    def setPhrase(self, phrase):
        self.phrase = phrase

    def stop(self):
        self.stopped = True
    

def IsBoundingBoxInFrame(frameSize, box, borderThreshold=50):

    (x1,y1,x2, y2) = box
    height,width,_ = frameSize
    if( x1 > borderThreshold and x2 < width-borderThreshold and
        y1 > borderThreshold and  y1 < height-borderThreshold):
        return True
    else:
        return False
    
#vs = VideoStreamFromCamera().start()
vs = PiVideoStream().start()
vp = VoicePrompts().start()

faceIdentified = False

try: 
    while True:
        
        # say cheese
        camera_capture = vs.read()
        frameDims = camera_capture.shape

        if( not vs.foundFacesInFrame() ):

            faceIdentified = False
            GPIO.output(flywheelPin, GPIO.HIGH)
            #continue

        else:

            # get the faces
            faces = vs.readFaces()
            
            # OpenCV has found faces in image
            for i in range(len(faces)):

                # check if face moved out of frame 
                faceInFrame = IsBoundingBoxInFrame(frameDims, faces[i])

                # if face moved out of frame then mark as new face
                if( not faceInFrame ):
                    faceIdentified = False
                
                # if we have not identified the face
                if( not faceIdentified ):
                    # encode the image into a known format for Rekognition to process
                    _, image = cv2.imencode(".png",camera_capture)
                
                    # While OpenCV may already have detected a face above, we need Rekognition to
                    # also detect the face, else we have issues.
                    response1 = client.detect_faces(
                        Image={
                            'Bytes': image.tobytes(),
                            },
                        Attributes=['DEFAULT',]
                    )

                    # has Rekognition found faces?
                    numFaces = len(response1["FaceDetails"])
                
                    if( numFaces > 0): # Yes, Rekognition found faces
                        response3 = client.search_faces_by_image(
                            CollectionId='Gallery',
                            Image={
                                'Bytes': image.tobytes(),
                            },
                        )
                        # Can Rekognition match faces to known people?
                        matches = len(response3['FaceMatches'])
                    
                        if matches > 0: # looks like Rekognition found a match
                            person = response3['FaceMatches'][0]['Face']['ExternalImageId']
                            vs.setName(person)
                            vp.setPhrase("Hello " + person)
                            faceIdentified = True
                        else:
                            vp.setPhrase("Intruder alert!")
                            faceIdentified = False
 
            if ( faceIdentified):
                vs.setColor(GREEN)
                GPIO.output(triggerPin, GPIO.HIGH)
                GPIO.output(flywheelPin, GPIO.HIGH)

            if( not faceIdentified):
                vs.setColor(RED)
                #vp.setPhrase("Intruder alert!")
                GPIO.output(flywheelPin, GPIO.LOW)
                GPIO.output(triggerPin, GPIO.LOW)
                time.sleep(0.1)
                GPIO.output(triggerPin, GPIO.HIGH)
                vs.setName("UNKNOWN")
                
except (KeyboardInterrupt): # expect to be here when keyboard interrupt
    GPIO.cleanup()
    vs.stop()
    vp.stop()
    
