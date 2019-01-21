#
# Visitor alert using pi camera
#
# Author: vswamina, September 2017.

# 2018_07_16 Version to detect motion
# 2017_09_30 Modified to support multi platform
# 2017_09_10 Updated with multiple changes for sharing

# With a lot of help from the Internet
import configparser
import cv2
import json
import threading
import time
from random import *
import sys
import imutils
import paho.mqtt.client as mqtt

#
# colors to use for bounding box and text
RED = 0, 0, 255
BLUE = 255, 0, 0
GREEN = 0, 255, 0
WHITE = 255,255,255
BLACK=10,10,10

debug = True

#
class readConfig:
    
    def __init__(self, configFile = "motionDetect.conf"):

        if(debug):
            print("<DEBUG> Reading config from " + configFile)

        # extract name of this config/instance from name of config file
        self.configName = configFile.split(".")[0]

        config = configparser.ConfigParser()
        configSet = config.read(configFile)

        platform = config['PLATFORM']
        self.platform = platform.get('type', 'WINDOWS')

        camera = config['CAMERA']
        self.cameraType = camera.get('type', 'BUILTINCAMERA')
        self.cameraPort = camera.get('port', 'DEFAULT_CAM')
        self.cameraUrl = camera.get('url', '127.0.0.1')
        self.cameraUser = camera.get('user', 'user')
        self.cameraPassword = camera.get('password', 'password')
        self.cameraResolution = camera.get('resolution', '(1280x720)')

        mqtt = config['MQTT']
        self.mqttIp = mqtt.get('ip', '127.0.0.1')
        self.mqttPort = mqtt.get('port', '1883')
        self.mqttTopic = mqtt.get('topic', 'debug')

        motion = config['MOTION']
        self.frameGrabInterval = motion.get('frameGrabInterval', 1)
        self.motionResetInterval = motion.get('motionResetInterval', 5)
                                              
        if (debug):
            print("\n === CONFIG INFO ===")
            print("platform:   " + self.platform)
            print("cameraType: " + self.cameraType)
            print("cameraPort: " + self.cameraPort)
            print("cameraUrl:  " + self.cameraUrl)
            print("cameraUser: " + self.cameraUser)
            print("cameraPassword: " + self.cameraPassword)
            print("cameraResolution: " + self.cameraResolution)
            print("mqttIp:   " + self.mqttIp)
            print("mqttPort: " + self.mqttPort)
            print("mqttTopic: " + self.mqttTopic)
            print("frameGrabInterval:  " + str(float(self.frameGrabInterval)))
            print("motionResetInterval:" + str(float(self.motionResetInterval))) 
            print(" ===================\n")

#
# generic class that's called with a parameter and this then instantiates the
# correct type of implementation. The video recognition logic is in this class
class VideoCamera:

    def __init__(self, cameraType, arg1=None, arg2=None):

        self.cameraType = cameraType
        
        # setup camera based on cameraType
        if( cameraType == "BUILTINCAMERA"):
            if( arg1 is not None ):
                print("BUILTINCAMERA OK")
                camera_port = arg1
                # no additional imports needed, OpenCV3 can deal with cameras
                if( camera_port == "DEFAULT_CAM"):
                    self.camera = cv2.VideoCapture(0)
                else:
                    self.camera = cv2.VideoCapture(1)
                self.camera.set(cv2.CAP_PROP_FRAME_WIDTH, 1920)
                self.camera.set(cv2.CAP_PROP_FRAME_HEIGHT, 1080)
                (self.grabbed, self.frame) = self.camera.read()
            else:
                print("If using built in camera, need to specify camera port")

        elif( cameraType == "DLINK930"):
            if( arg1 is not None and arg2 is not None):
                print("DLINK930 OK")
                cameraUrl = arg1
                authString = arg2
                streamurl = "http://" + ':'.join(authString) + '@' + cameraUrl + "/video.cgi"
                if (debug):
                    print("URL: ", streamurl)
                # no additional imports needed, OpenCV3 can deal with the URL
                self.camera= cv2.VideoCapture(streamurl)
                (self.grabbed, self.frame) = self.camera.read()
            else:
                print("If using IP Camera, need to specify camera IP and user:password")

        elif( cameraType == "DLINK2312"):
            if( arg1 is not None and arg2 is not None):
                print("DLINK2312 OK")
                cameraUrl = arg1
                authString = arg2
                streamurl = "http://" + ':'.join(authString) + '@' + cameraUrl + "/video1.mjpg"
                #streamurl = "rtsp://" + ':'.join(authString) + '@' + cameraUrl + ":554/live1.sdp"
                if (debug):
                    print("URL: ", streamurl)
                # no additional imports needed, OpenCV3 can deal with the URL
                self.camera= cv2.VideoCapture(streamurl)
                (self.grabbed, self.frame) = self.camera.read()
            else:
                print("If using IP Camera, need to specify camera IP and user:password")
            
        elif( cameraType == "PICAMERA"):
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
            
        # should thread run or stop
        self.stopped = False

    def start(self):
        # start thread to read frame
        if( self.cameraType == "PICAMERA"):
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

    def detectMotion(self, frame1, frame2, frameO):

        motionDetected = False
        # Check for differences between current and previous frame to see if motion occurred
        frameDelta = cv2.absdiff(frame1, frame2)
        thresh = cv2.threshold(frameDelta, 25, 255, cv2.THRESH_BINARY)[1]
        thresh = cv2.dilate(thresh, None, iterations=2)
        #cv2.imshow("thresh", thresh)
        #cv2.imshow("delta", frameDelta)
        # get contours
        (_,cnts,_) = cv2.findContours(thresh.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

        # check if contours are large in area
        for c in cnts:
            if cv2.contourArea(c) < 5000:
                continue

            (x,y,w,h) = cv2.boundingRect(c)
            cv2.rectangle(frame2, (x,y), (x+w, y+h), (0, 255,0), 2)
            motionDetected = True

        return motionDetected

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

#
# read config from file name
if( len(sys.argv) == 2):

    conf = readConfig(sys.argv[1])
    
if( conf.cameraType == "PICAMERA"):
    vs = VideoCamera(conf.cameraType, (400, 300), 30)

if( conf.cameraType == "BUILTINCAMERA"):
    vs = VideoCamera(conf.cameraType, conf.cameraPort)

if( conf.cameraType == "DLINK2312" or conf.cameraType == "DLINK930"):
    vs = VideoCamera(conf.cameraType, conf.cameraUrl, (conf.cameraUser, conf.cameraPassword))

try:
    vs
except NameError:
    print("*ERROR*: Could not create videostream for camera <"+conf.cameraType+">. Stopping...")
    vs.stop()
    quit()

vs.start()
time.sleep(1)

# to detect motion we shall look at previous frame
prevFrame = None
prevMotionDetected = False
motionDetectedValue = False

lastGrabTime = 0 # when did we get the last frame
lastMotionBaseTime = 0 # when did we reset the motion base

mqttClient = mqtt.Client()
mqttClient.connect(conf.mqttIp, int(conf.mqttPort), 60)
mqttClient.loop_start()

try: 
    while True:

        # check if we can read a frame from camera
        okToGrabFrame=True
        if( time.time() - lastGrabTime < float(conf.frameGrabInterval)):
            okToGrabFrame = False

        # if we can read a frame...         
        if( okToGrabFrame):

            # reset frame grab timer
            lastGrabTime = time.time()

            # say cheese
            camera_capture = vs.read()

            # if we did not get an image for any reason
            if( camera_capture is None):
                print("*ERROR*: did not get image frame. Stopping...")
                vs.stop()
                quit()

            # do some processing on frame we just read
            currFrameOrig = imutils.resize(camera_capture, width=500)
            currFrameProc = cv2.cvtColor(currFrameOrig, cv2.COLOR_BGR2GRAY)
            currFrameProc = cv2.GaussianBlur(currFrameProc, (21,21), 0)

            # is this the first frame or did we read one earlier
            if prevFrame is None:
                prevFrame = currFrameProc
                lastMotionBaseTime = time.time()
                continue
            # update the base/background image since that may change (e.g. shadows)
            elif( time.time() - lastMotionBaseTime > float(conf.motionResetInterval)):
                prevFrame = currFrameProc
                lastMotionBaseTime = time.time()

            # use processed frames to detect motion
            # sending in original frame only to draw contours
            # if we already have detected motion don't call this again
            
            motionDetected = vs.detectMotion(prevFrame, currFrameProc, currFrameOrig)
              
            if( prevMotionDetected != motionDetected):
                if( motionDetected ):
                    sensorData = {}
                    sensorData['time'] = round(time.time())
                    sensorData['atime'] = time.ctime()
                    sensorData['sensorId'] = conf.configName
                    sensorData['sensorType']='motion'
                    sensorData['sensorValue']=motionDetected
                    mqttClient.publish(conf.mqttTopic+"/"+conf.configName+"/event", json.dumps(sensorData))
                    #mqttClient.loop()
                prevMotionDetected = motionDetected

            cv2.imshow(conf.configName, currFrameOrig)
            key = cv2.waitKey(1)
            if (key == "q"):
                vs.stop()

except (KeyboardInterrupt): # expect to be here when keyboard interrupt
    print("* Program Interrupted. Stopping...*")
    vs.stop()
    
