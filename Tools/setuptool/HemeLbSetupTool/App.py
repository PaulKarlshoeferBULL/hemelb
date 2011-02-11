import wx

from HemeLbSetupTool.Model.Profile import Profile
from HemeLbSetupTool.Model.Pipeline import Pipeline

from HemeLbSetupTool.Controller.ProfileController import ProfileController
from HemeLbSetupTool.Controller.PipelineController import PipelineController

from HemeLbSetupTool.View.MainWindow import MainWindow

class SetupTool(wx.App):
    def __init__(self, args={}, profile=None, **kwargs):
        self.cmdLineArgs = args
        self.cmdLineProfileFile = profile
        
        wx.App.__init__(self, **kwargs)
        return
    
    def OnInit(self):
        # Model
        if self.cmdLineProfileFile is None:
            # No profile
            self.profile = Profile(**self.cmdLineArgs)
        else:
            # Load the profile
            self.profile = Profile.NewFromFile(self.cmdLineProfileFile)
            # override any keys that have been set on cmdline.
            for k, val in self.cmdLineArgs.iteritems():
                if val is not None:
                    setattr(self.profile, k, val)
                    pass
                continue
            pass
        
        self.pipeline = Pipeline()
        
        # Controller
        self.controller = ProfileController(self.profile)
        self.controller.Pipeline = PipelineController(self.pipeline, self.controller)
        
        # View
        self.view = MainWindow(self.controller)

        # If we set the StlFile on the command line, we need to
        # trigger the event now everything's been setup.
        if 'StlFile' in self.cmdLineArgs and \
               self.cmdLineArgs['StlFile'] is not None:
            self.profile.DidChangeValueForKey('StlFile')
            pass

        self.SetTopWindow(self.view)
        return True
    
    pass

