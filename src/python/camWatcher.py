#
# Visitor alert using pi camera
#
# Author: vswamina, November 2017.
# 2017_11_26 created from home_security.py

# set platform where this code is being run
PI = 1
WINDOWS = 2
LINUX = 3
platform = LINUX

# With a lot of help from the Internet
import cv2
import json
import threading
import time
from random import *
import sys
import imutils
import paho.mqtt.client as mqtt

#
# Type of camera you want to use
#
DLINK930 = 1 # D-Link camera model 930. URL = user:pass@ip:port/video.cgi
DLINK2312 = 2 # D-Link camera model 2312. URL = user:pass@ip/video1.mjpg 
BUILTINCAMERA = 3 # USB or built in camera on laptop
PICAMERA = 4 # Pi camera if running on Raspberry Pi
# which camera do we use
camera_port = BUILTINCAMERA

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



#
# generic class that's called with a parameter and this then instantiates the
# correct type of implementation. The video recognition logic is in this class
class VideoCamera:

    def __init__(self, cameraType, arg1=None, arg2=None):

        self.cameraType = cameraType
        
        # setup camera based on cameraType
        if( cameraType == BUILTINCAMERA):
            if( arg1 is not None ):
                print("Connecting to BUILTIN...")
                camera_port = arg1
                # no additional imports needed, OpenCV3 can deal with cameras
                self.camera = cv2.VideoCapture(camera_port)
                self.camera.set(cv2.CAP_PROP_FRAME_WIDTH, 1920)
                self.camera.set(cv2.CAP_PROP_FRAME_HEIGHT, 1080)
                (self.grabbed, self.frame) = self.camera.read()
                print("...connected to BUILTIN")
            else:
                print("If using built in camera, need to specify camera port")

        elif( cameraType == DLINK930):
            if( arg1 is not None and arg2 is not None):
                print("Connecting to DLINK930...", end="", flush=True)
                cameraUrl = arg1
                authString = arg2
                streamurl = "http://" + ':'.join(authString) + '@' + cameraUrl + "/video.cgi"
                print("URL: ", streamurl)
                # no additional imports needed, OpenCV3 can deal with the URL
                self.camera= cv2.VideoCapture(streamurl)
                (self.grabbed, self.frame) = self.camera.read()
                print("...connected to DLINK930 at URL",streamurl)
            else:
                print("If using IP Camera, need to specify camera IP and user:password")

        elif( cameraType == DLINK2312):
            if( arg1 is not None and arg2 is not None):
                print("Connecting to DLINK2312...", end="", flush=True)
                cameraUrl = arg1
                authString = arg2
                streamurl = "http://" + ':'.join(authString) + '@' + cameraUrl + "/video1.mjpg"
                print("URL: ", streamurl)
                # no additional imports needed, OpenCV3 can deal with the URL
                self.camera= cv2.VideoCapture(streamurl)
                (self.grabbed, self.frame) = self.camera.read()
                print("...connected to DLINK2312 at URL",streamurl)
            else:
                print("If using IP Camera, need to specify camera IP and user:password")
            
        elif( cameraType == PICAMERA):
            if( arg1 is not None and arg2 is not None):
                print("Connectin to PICAMERA...", end="", flush=True)
                # imports to handle picamera
                from picamera.array import PiRGBArray
                from picamera import PiCamera
                self.camera = PiCamera()
                self.camera.resolution = arg1
                self.camera.framerate = arg2
                self.rawCapture = PiRGBArray(self.camera, size=self.camera.resolution)
                time.sleep(1)
                print("...connected to PICAMERA.", flush=True)
            else:
                print("If using Pi Camera, need to specify resolution (x,y) and framerate")
        else:
            print("couldn't make sense of your arguments. Cannot proceed!")
            exit()
            
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
  
            
    def update(self): # update for other cameras
        # read camera
        while True:
            if self.stopped:
                return

            (self.grabbed, self.frame) = self.camera.read()
                
    def drawRect(self, x1, y1, x2, y2, color):
        cv2.rectangle(self.frame, (x1, y1), (x2, y2), color, 2)
        return

    
    def read(self):
        return self.frame

    def stop(self):
        self.stopped = True
        cv2.destroyAllWindows()
        del self.camera


def IsBoundingBoxInFrame(frameSize, box, borderThreshold=50):

    (x1,y1,x2, y2) = box
    height,width,_ = frameSize
    if( x1 > borderThreshold and x2 < width-borderThreshold and
        y1 > borderThreshold and  y1 < height-borderThreshold):
        return True
    else:
        return False


#
#
#  MAIN Code starts here
#
#
if( len(sys.argv) < 2 ):
    print("Usage: ",sys.argv[0], " cameraType [user password ipaddr location]")
    print("     cameraType = BUILTIN, PI, DLINK2312, DLINK930")
    print("     If cameraType is DLINK2312 or DLINK930 then username, password and IP address to be specified")
    quit()

if( len(sys.argv) == 3):
    if( sys.argv[1] == "PI"):
        vs = VideoCamera(PICAMERA, (400, 300), 30)
    elif( sys.argv[1] == "BUILTIN"):
        vs = VideoCamera(BUILTINCAMERA, DEFAULT_CAM)
    else:
        print("Usage: ",sys.argv[0], " cameraType [user password ipaddr location]")
        print("     cameraType = BUILTIN, PI, DLINK2312, DLINK930")
        print("     If cameraType is DLINK2312 or DLINK930 then username, password and IP address to be specified")
        quit()
        
if( len(sys.argv) == 6):
    if( sys.argv[1] == "DLINK2312"):
        vs = VideoCamera(DLINK2312, sys.argv[4], (sys.argv[2], sys.argv[3]))
    elif( sys.argv[1] == "DLINK930"):
        vs = VideoCamera(DLINK930, sys.argv[4], (sys.argv[2], sys.argv[3])) 
    else:
        print("Usage: ",sys.argv[0], " cameraType [user password ipaddr location]")
        print("     cameraType = BUILTIN, PI, DLINK2312, DLINK930")
        print("     If cameraType is DLINK2312 or DLINK930 then username, password and IP address to be specified")
        quit()
    
if(vs == None):
    print("Error creating videostream. Exiting.")
    quit()

location = sys.argv[5]
vs.start()
time.sleep(1)

ipAddress = "192.168.86.74"
port = 1883
timeout = 60
topic = "Sensors"

mqttClient = mqtt.Client()
mqttClient.connect(ipAddress, port, timeout)

mqttClient.loop_start()

#
# Some confis
minSaveInterval = 10 # do not save files more often than this many seconds
##
## From a no-motion state, check frames every frameGrabInterval seconds
## Every time we detect motion, we increase the publish interval by 2 seconds
## Every time we do not detect motion we decrease interval by 6 seconds
## (but we never go lower than min...Interval or higher than max...Inteval
## so that we can auto-throttle the motion alerts
minPublishInterval = 2 # do not publish more frequently than this
maxPublishInterval = 180 # do not publish less frequently than this
publishInterval = minPublishInterval
frameGrabInterval = 1 #read frame from camera every this many seconds
motionResetInterval = 15 # reset background every so many seconds for motion detection

# to detect motion we shall look over previous frame
prevFrame = None
lastSaveTime = 0 # when did we save the last image
lastGrabTime = 0 # when did we get the last frame
lastMotionBaseTime = 0
lastPublishTime = 0 # When did we last publish an alert

try: 
    while True:

        if( time.time() - lastGrabTime < frameGrabInterval):
            continue
        
        lastGrabTime = time.time()
        
        # grab a frame from camera
        camera_capture = vs.read()

        # I did not find a frame. This is not good. 
        if( camera_capture is None):
            quit()
            
        # resize, grayscale and smooth
        currFrame = imutils.resize(camera_capture, width=500)
        text = "No Motion(" + str(publishInterval) + ")"
        gray = cv2.cvtColor(currFrame, cv2.COLOR_BGR2GRAY)
        gray = cv2.GaussianBlur(gray, (21,21), 0)

        # if this is the first frame set it up as base, and skip rest of loop
        if prevFrame is None:
            prevFrame = gray
            lastMotionBaseTime = time.time()
            continue
        # if we are ready to grab next frame (based on time interval) do so
        elif( time.time() - lastMotionBaseTime > maxPublishInterval):
            prevFrame = gray
            lastMotionBaseTime = time.time()

        # check for regions where motion may be there
        frameDelta = cv2.absdiff(prevFrame, gray)
        thresh = cv2.threshold(frameDelta, 25, 255, cv2.THRESH_BINARY)[1]
        thresh = cv2.dilate(thresh, None, iterations=2)
        (_,cnts,_) = cv2.findContours(thresh.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

        motionTrigger = False

        # evaluate contour regions to see if they are large enough
        for c in cnts:
            if cv2.contourArea(c) < 3000:
                continue

            # if at least one region is large enough then we have motion
            (x,y,w,h) = cv2.boundingRect(c)
            cv2.rectangle(currFrame, (x,y), (x+w, y+h), (0, 255,0), 2)
            text = "Motion detected (" + str(publishInterval) + ")"
            motionTrigger = True

        # Now that we have motion, let's create a data object
        if( motionTrigger ):

            if( time.time() - lastPublishTime > publishInterval):

                lastPublishTime = time.time()
                #print("Found motion")
                sensorData = {}
                sensorData['time'] = round(lastPublishTime * 1000)
                sensorData['sensorId'] = location
                sensorData['sensorType']='motion'
                sensorData['sensorValue']='true'

                mqttClient.publish(topic, json.dumps(sensorData))
                cv2.imwrite(
                    '/run/user/1000/gvfs/afp-volume:host=WDMyCloud.local,user=anonymous,volume=Public/'+
                    str(lastPublishTime) +
                    '.png',
                    currFrame)
                        
                # increase delay to next publish
                publishInterval = publishInterval + 2
                if( publishInterval > maxPublishInterval):
                    publishInterval = maxPublishInterval
        else:
            publishInterval = publishInterval - 6
            if( publishInterval <= minPublishInterval):
                publishInterval = minPublishInterval
                
                
        cv2.putText(currFrame, text, (10,20),cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 2)     
        cv2.imshow("Motionsensor", currFrame)
        #cv2.imshow("thresh", thresh)
        #cv2.imshow("delta", frameDelta)
        cv2.waitKey(1)


except (KeyboardInterrupt): # expect to be here when keyboard interrupt
    vs.stop()
    
