#
# Face recognition demo
#

import cv2
from CameraReader import VideoCamera
import boto3
import time
import Gallery

# define colors
YELLOW = 255,255,0
RED = 0, 0, 255
BLUE = 255, 0, 0
GREEN = 0, 255, 0
WHITE = 255,255,255
BLACK=10,10,10

def drawRect(frame,x1, y1, x2, y2, color):
    cv2.rectangle(frame, (x1, y1), (x2, y2), color, 2)
    return

def IsBoundingBoxInFrame(frameSize, box, borderThreshold=50):

    (x1,y1,x2, y2) = box
    height,width,_ = frameSize
    if( x1 > borderThreshold and x2 < width-borderThreshold and
        y1 > borderThreshold and  y1 < height-borderThreshold):
        return True
    else:
        return False

#
# SETUP CAMERA
#
# see CameraReader.py for setting up this call
vs = VideoCamera(2, "192.168.86.131", ("admin", "f1102cosmos"))
vs.start()  # start the camera
time.sleep(5) # wait for seconds for camera to stabilize
faceIdentified = False

#
# setup AWS S3 client (to retrieve images that are stored there)
#
s3client = boto3.client("s3")

# setup AWS Rekognition client
#
rekogClient = boto3.client('rekognition')

#
# setup gallery of faces
#
galleryName = "Gallery"
imagesBucket = "com-dminc-vswaminathan-demo"
Gallery.deleteGallery(rekogClient, galleryName)
Gallery.createGallery(rekogClient, s3client, galleryName, imagesBucket)
#
# setup face detector classifier
#
# this is the classifier that detects faces from image
faceCascade = cv2.CascadeClassifier('haarcascade_frontalface_default.xml')
# store faces retrieved by classifier
faces=[]
foundFacesInFrame = False
identifiedFaceInFrame = False

try: 
    while True:
        
        # say cheese; get a frame from camera
        rawFrame = vs.read()
        frameDims = rawFrame.shape

        # process frame if needed
        processedFrame = rawFrame

        # detect faces
        faces = faceCascade.detectMultiScale(
            processedFrame,
            scaleFactor = 1.1,
            minNeighbors = 5,
            minSize = (20,20)
        )
        foundFacesInFrame = (len(faces) > 0)

        if( not foundFacesInFrame ):
            identifiedFaceInFrame = False
        else:  # OpenCV has found faces in image

            largestFaceArea = 0
            largestFaceBox = (0,0,0,0)    
           
            # process each face
            for face in faces:

                # draw bounding box (for visual feedback only)
                (x, y, w, h) = face
                drawRect(rawFrame,x,y,x+w,y+h,YELLOW)

                # pick largest face
                if( w*h > largestFaceArea):
                    largestFaceBox = (x,y,w,h)
                    largestFaceArea = w*h
                    
                # check if face moved out of frame 
                faceInFrame = IsBoundingBoxInFrame(frameDims, face)

                # if face moved out of frame then mark as new face
                if( not faceInFrame ):
                    identifiedFaceInFrame = False
                
                # if we have not identified the face
                if( not identifiedFaceInFrame ):

                    # encode the image into a known format for Rekognition to process
                    _, rekogInputFrame = cv2.imencode(".png",processedFrame)
                
                    # While OpenCV may already have detected a face above, we need Rekognition to
                    # also detect the face, else we have issues.
                    rekogFaces = rekogClient.detect_faces(
                        Image={
                            'Bytes': rekogInputFrame.tobytes(),
                            },
                        Attributes=['DEFAULT',]
                    )

                    # has Rekognition found faces?
                    numFaces = len(rekogFaces["FaceDetails"])
                
                    if( numFaces > 0): # Yes, Rekognition found faces
                        response3 = rekogClient.search_faces_by_image(
                            CollectionId='Gallery',
                            Image={
                                'Bytes': rekogInputFrame.tobytes(),
                            },
                        )

                        # Can Rekognition match faces to known people?
                        matches = len(response3['FaceMatches'])
                    
                        if matches > 0: # looks like Rekognition found a match
                            person = response3['FaceMatches'][0]['Face']['ExternalImageId']
                            print("found " + person)
                            identifiedFaceInFrame = True
                        else:
                            identifiedFaceInFrame = False 
            # if( largestFaceArea > 200 and
            #     time.time() - lastSaveTime > minSaveInterval):

            #     print("Saving file with area "+str(largestFaceArea))
            #     lastSaveTime = time.time()
            #     # store saved face in S3 for 72 hours

            #     #faceImage = self.frame[y:y+h, x:x+h]
            #     _,faceImageFile = cv2.imencode(".png",camera_capture)
            #     timeStr = time.strftime("%Y%m%d-%H%M%S")
            #     response = s3client.put_object(
            #             Body=faceImageFile.tobytes(),
            #             Bucket="com.vswamina.aws.doorimages",
            #             Key=timeStr+"-front.png") 

            if (identifiedFaceInFrame):
                vs.setColor(GREEN)

            if( not faceIdentified):
                vs.setColor(RED)

except():
    print("caught exception")
    vs.stop()
