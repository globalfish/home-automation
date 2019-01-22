import configparser

debug = True

#
class ConfigReader:
    
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
        self.cameraPort = camera.get('port', '0')
        self.cameraUrl = camera.get('url', '127.0.0.1')
        self.cameraUser = camera.get('user', 'user')
        self.cameraPassword = camera.get('password', 'password')
        self.cameraResolution = camera.get('resolution', '(1280x720)')

        aws = config['AWS']
        self.awsFacesBucket = aws.get('facesBucket', 'com-dminc-vswaminathan-demo')
        self.awsGallery = aws.get('galleryName', 'Gallery')
        self.awsOutputBucket = aws.get('outputBucket', 'com-dminc-vswaminathan-demo-output')

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
            print("motionResetInterval: " + str(float(self.motionResetInterval)))
            print("facesBucket: " + self.awsFacesBucket)
            print("galleryName: " + self.awsGallery)
            print("outputBucket: " + self.awsOutputBucket) 
            print(" ===================\n")

