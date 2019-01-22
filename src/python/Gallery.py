# Create gallery of images for face recognition demos
#

# 2019_01_21 Created from previous code

def deleteGallery(rekogClient, galleryName):

    #
    # delete previous collection, if any
    #
    try:
        rekogClient.delete_collection( CollectionId = galleryName)
    except Exception:
        print("Caught an exception when deleting... Continuing...")
        pass


def createGallery(rekogClient, s3client, galleryName, imagesBucket):

    print("Creating images gallery from bucket..."+imagesBucket)
    
    images = s3client.list_objects(Bucket=imagesBucket)['Contents']
    
    #
    # Create a new collection
    #
    rekogClient.create_collection( CollectionId = galleryName)

    for image in images:
        imageFileName = image["Key"]
        # use filename as name of face in the image, e.g. JohnDoe.png
        imageId = imageFileName.split(".")[0]
        print("Indexing..." + imageId)
        rekogClient.index_faces(
            CollectionId=galleryName,
            Image = {
                'S3Object' : {
                    'Bucket' : imagesBucket,
                    'Name' : imageFileName
                }
            },
            ExternalImageId = imageId,
            DetectionAttributes = ['DEFAULT']
        )

    print("... created gallery " + galleryName)