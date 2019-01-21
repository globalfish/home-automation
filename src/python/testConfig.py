import configparser
#
# Read config file settings from motionDetect.config
#
class readConfig:

    def __init__(self, configFile = "motionDetect.conf"):

        config = configparser.ConfigParser()
        configSet = config.read(configFile)
        if len(configSet) != 1: # config file not present so set defaults
            self.cameraType = "BUILTINCAMERA"
            self.camera_port = "DEFAULT_CAM"
        else:
            self.cameraType = config['CAMERA']['type']
            self.cameraPort = config['CAMERA']['port']


conf = readConfig()
print(conf.cameraType)
print(conf.cameraPort)
