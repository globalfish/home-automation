#
# Visitor alert using pi camera
#
# Author: vswamina, September 2017.
# 2017_09_10 Updated with multiple changes for sharing
# 2017_09_30 Modified to support multi platform

# set platform where this code is being run
PI = 1
WINDOWS = 2
LINUX = 3
platform = WINDOWS

# With a lot of help from the Internet
import cv2
import boto3
import json
import threading
#import RPi.GPIO as GPIO
#from espeak import espeak
import time
from random import *
import sys

#
# Type of camera you want to use
#
DLINK930 = 1 # D-Link camera model 930. URL = user:pass@ip:port/video.cgi
DLINK2312 = 2 # D-Link camera model 2312. URL = user:pass@ip/video1.mjpg 
BUILTINCAMERA = 3 # USB or built in camera on laptop
PICAMERA = 4 # Pi camera if running on Raspberry Pi

#
# if you have multiple cameras on your device then just use appropriate device
# if you have just one camera on your device then DEFAULT_CAM should work
DEFAULT_CAM = 0
REAR_CAM = 1
XTNL_CAM = 2

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

#
# load images from S3 bucket and create a gallery of known faces
#
s3client = boto3.client("s3")

#
# generic class that's called with a parameter and this then instantiates the
# correct type of implementation. The video recognition logic is in this class
class VideoCamera:

    def __init__(self, cameraType, arg1=None, arg2=None):

        self.cameraType = cameraType
        
        # setup camera based on cameraType
        if( cameraType == BUILTINCAMERA):
            if( arg1 is not None ):
                print("BUILTINCAMERA OK")
                camera_port = arg1
                # no additional imports needed, OpenCV3 can deal with cameras
                self.camera = cv2.VideoCapture(camera_port)
                self.camera.set(cv2.CAP_PROP_FRAME_WIDTH, 1920)
                self.camera.set(cv2.CAP_PROP_FRAME_HEIGHT, 1080)
                (self.grabbed, self.frame) = self.camera.read()
            else:
                print("If using built in camera, need to specify camera port")

        elif( cameraType == DLINK930):
            if( arg1 is not None and arg2 is not None):
                print("DLINK930 OK")
                cameraUrl = arg1
                authString = arg2
                streamurl = "http://" + ':'.join(authString) + '@' + cameraUrl + "/video.cgi"
                # no additional imports needed, OpenCV3 can deal with the URL
                self.camera= cv2.VideoCapture(streamurl)
                (self.grabbed, self.frame) = self.camera.read()
            else:
                print("If using IP Camera, need to specify camera IP and user:password")

        elif( cameraType == DLINK2312):
            if( arg1 is not None and arg2 is not None):
                print("DLINK2312 OK")
                cameraUrl = arg1
                authString = arg2
                streamurl = "http://" + ':'.join(authString) + '@' + cameraUrl + "/video1.mjpg"
                # no additional imports needed, OpenCV3 can deal with the URL
                self.camera= cv2.VideoCapture(streamurl)
                (self.grabbed, self.frame) = self.camera.read()
            else:
                print("If using IP Camera, need to specify camera IP and user:password")
            
        elif( cameraType == PICAMERA):
            if( arg1 is not None and arg2 is not None):
                # imports to handle picamera
                from picamera.array import PiRGBArray
                from picamera import PiCamera
                self.camera = PiCamera()
                self.camera.resolution = arg1
                self.camera.framerate = arg2
                self.rawCapture = PiRGBArray(self.camera, size=self.camera.resolution)
                time.sleep(1)
            else:
                print("If using Pi Camera, need to specify resolution (x,y) and framerate")
        else:
            print("couldn't make sense of your arguments. Cannot proceed!")
            exit()
            

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
        if( self.cameraType == PICAMERA):
            threading.Thread(target=self.updatePiCam, args=()).start()
        else:
            threading.Thread(target=self.update, args=()).start()
            
        return self

    def updatePiCam(self): # This will work only for Pi Camera
        # read camera

        for frame in self.camera.capture_continuous(self.rawCapture,
                    format="bgr", use_video_port=True):

            if self.stopped:
                return
            
            self.frame=frame.array
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

            # process each face found
            for (x,y,w,h) in self.faces:

                # draw bounding box
                self.drawRect(x, y, x+w, y+h, self.color)
                cv2.putText(self.frame, self.personName, (x, y-10), self.font, 0.5, self.color, 2)

            if( len(self.faces) > 0):
                self.foundFaces = True
            else:
                self.foundFaces = False

            cv2.waitKey(1)

    def update(self): # update for other cameras
        # read camera
        lastSaveTime = 0 # time when did the last save
        minSaveInterval = 10 # do not save files more often than this

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

    
            # process each face found
            for (x,y,w,h) in self.faces:

                # draw bounding box
                self.drawRect(x, y, x+w, y+h, self.color)
                cv2.putText(self.frame, self.personName, (x, y-10), self.font, 0.5, self.color, 2)

         
            if( len(self.faces) > 0):
                self.foundFaces = True
            else:
                self.foundFaces = False
                
            cv2.namedWindow('DoorMonitor', cv2.WINDOW_NORMAL)
            cv2.moveWindow('DoorMonitor', 10, 50)
            cv2.resizeWindow('DoorMonitor', 855,480)
            cv2.imshow('DoorMonitor', self.frame)
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

def IsBoundingBoxInFrame(frameSize, box, borderThreshold=50):

    (x1,y1,x2, y2) = box
    height,width,_ = frameSize
    if( x1 > borderThreshold and x2 < width-borderThreshold and
        y1 > borderThreshold and  y1 < height-borderThreshold):
        return True
    else:
        return False
    
if( len(sys.argv) < 2 ):
    print("Usage: ",sys.argv[0], " cameraType [user password ipaddr]")
    print("     cameraType = BUILTIN, PI, DLINK2312, DLINK930")
    print("     If cameraType is DLINK2312 or DLINK930 then username, password and IP address to be specified")
    quit()

if( len(sys.argv) == 2):
    if( sys.argv[1] == "PI"):
        vs = VideoCamera(PICAMERA, (400, 300), 30)
    elif( sys.argv[1] == "BUILTIN"):
        vs = VideoCamera(BUILTINCAMERA, DEFAULT_CAM)
    else:
        print("Usage: ",sys.argv[0], " cameraType [user password ipaddr]")
        print("     cameraType = BUILTIN, PI, DLINK2312, DLINK930")
        print("     If cameraType is DLINK2312 or DLINK930 then username, password and IP address to be specified")
        quit()
        
if( len(sys.argv) == 5):
    if( sys.argv[1] == "DLINK2312"):
        vs = VideoCamera(DLINK2312, sys.argv[4], (sys.argv[2], sys.argv[3]))
    elif( sys.argv[1] == "DLINK930"):
        vs = VideoCamera(DLINK930, sys.argv[4], (sys.argv[2], sys.argv[3])) 
    else:
        print("Usage: ",sys.argv[0], " cameraType [user password ipaddr]")
        print("     cameraType = BUILTIN, PI, DLINK2312, DLINK930")
        print("     If cameraType is DLINK2312 or DLINK930 then username, password and IP address to be specified")
        quit()
    
if(vs == None):
    print("Error creating videostream. Exiting.")
    quit()

vs.start()

faceIdentified = False
time.sleep(5)
lastSaveTime = 0 # time when did the last save
minSaveInterval = 10 # do not save files more often than this
try: 
    while True:
        
        # say cheese
        camera_capture = vs.read()
        frameDims = camera_capture.shape

        if( not vs.foundFacesInFrame() ):

            faceIdentified = False

        else:

            # get the faces
            faces = vs.readFaces()

            largestFaceArea = 0
            largestFaceBox = (0,0,0,0)    
            # OpenCV has found faces in image
            for face in faces:

                (x, y, w, h) = face
                
                # pick largest face
                if( w*h > largestFaceArea):
                    largestFaceBox = (x,y,w,h)
                    largestFaceArea = w*h
                    
                # check if face moved out of frame 
                faceInFrame = IsBoundingBoxInFrame(frameDims, face)

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
                            faceIdentified = True
                        else:
                            faceIdentified = False

                                 
            if( largestFaceArea > 200 and
                time.time() - lastSaveTime > minSaveInterval):

                print("Saving file with area "+str(largestFaceArea))
                lastSaveTime = time.time()
                # store saved face in S3 for 72 hours

                #faceImage = self.frame[y:y+h, x:x+h]
                _,faceImageFile = cv2.imencode(".png",camera_capture)
                timeStr = time.strftime("%Y%m%d-%H%M%S")
                response = s3client.put_object(
                        Body=faceImageFile.tobytes(),
                        Bucket="com.vswamina.aws.doorimages",
                        Key=timeStr+"-front.png")
                
            if ( faceIdentified):
                vs.setColor(GREEN)

            if( not faceIdentified):
                vs.setColor(RED)
                vs.setName("UNKNOWN")
                
except (KeyboardInterrupt): # expect to be here when keyboard interrupt
    vs.stop()
    
