import os.path

from vtk import vtkClipPolyData, vtkAppendPolyData, vtkPlane, vtkStripper, \
    vtkFeatureEdges, vtkPolyDataConnectivityFilter, vtkProgrammableFilter, \
    vtkTriangleFilter, vtkCleanPolyData, vtkIntArray, vtkPoints, vtkPolyData, \
    vtkCellArray, vtkIdList, vtkPolyLine, \
    vtkXMLPolyDataWriter, vtkAlgorithm, vtkImplicitBoolean, vtkSphere, \
    vtkPolyDataNormals

from vmtk.vtkvmtk import vtkvmtkPolyDataBoundaryExtractor
from vmtk.vtkvmtk import vtkvmtkBoundaryReferenceSystems

from .Iolets import Iolet
def getpreviousstage(algo):
    """Given a vtkAlgorithm, get the previous algorithm in its pipeline.
    """
    return algo.GetInputConnection(0, 0).GetProducer()


def getpipeline(last):
    """Return a list of all the algorithms in a pipeline, given the last one.
    """
    pipe = [last]
    while True:
        try:
            prev = getpreviousstage(pipe[0])
            pipe.insert(0, prev)
        except:
            break
        continue

    return pipe


class StageWriter(object):
    """For debugging, will easily let one write polydata.
    """

    def __init__(self, d):
        if os.path.exists(d):
            assert os.path.isdir(d)
        else:
            os.mkdir(d)
            pass
        self.dir = d
        self.i = 0
        return

    def WriteOutput(self, stage, name=None):
        writer = vtkXMLPolyDataWriter()

        if isinstance(stage, vtkAlgorithm):
            # To ensure everything executes in order
            stage.Update()
            writer.SetInputConnection(stage.GetOutputPort())
        elif isinstance(stage, vtkPolyData):
            writer.SetInput(stage)
        else:
            raise ValueError(
                'Cannot cope with instances of "%s"' % type(stage))

        if name is None:
            fnPattern = '%02d.vtp'
        else:
            fnPattern = '%02d-' + name + '.vtp'
        filename = os.path.join(self.dir, fnPattern % self.i)
        self.i += 1
        writer.SetFileName(filename)
        writer.Write()
        return
    pass

class Clipper(object):
    """Clips the input STL file to the ROI and caps it.
    """

    def __init__(self, profile):
        """Create the generator for the supplied Model.Profile object.

        profile - the HemeLbSetupTool.Model.Profile object to use
        """
        # Copy the profile's _Args
        for k in profile._Args:
            setattr(self, k, getattr(profile, k))
            continue
        # Pull in the SurfaceSource too
        self.SurfaceSource = profile.StlReader

        self.ClippedSurfaceSource = self.ConstructClipPipeline()
        return

    def ConstructClipPipeline(self):
        """This constructs a VTK pipeline to clip the vtkPolyData read from
        the STL file against the Iolets. It also adds a scalar value to each
        polygon indicating which Iolet it represents, or -1 if it is just a
        wall, and computes polygon normals.

        Note that this does NOT EXECUTE the pipeline.
        """
        # seal any leaks first.
        closer = PolyDataCloser()
        closer.SetInputConnection(self.SurfaceSource.GetOutputPort())
        # Add the Iolet id -1 to all cells
        adder = IntegerAdder(Value=-1)
        adder.SetInputConnection(closer.GetOutputPort())
        # Have the name pdSource first point to the input, then loop
        # over IOlets, clipping and capping.
        pdSource = adder
        for i, iolet in enumerate(self.Iolets):
            capper = PolyDataClipCapAndLabeller(Value=i, Iolet=iolet,
                                                SeedPoint=self.SeedPoint)
            capper.SetInputConnection(pdSource.GetOutputPort())
            # Set the source of the next iteraction to the capped
            # surface producer.
            pdSource = capper
            continue

        # The following adds cell normals to the PolyData
        normer = vtkPolyDataNormals()
        normer.SetInputConnection(pdSource.GetOutputPort())
        normer.ComputeCellNormalsOn()
        normer.ComputePointNormalsOff()
        normer.SplittingOff()
        normer.ConsistencyOn()
        normer.AutoOrientNormalsOn()
        normer.NonManifoldTraversalOff()

        return normer

    pass


class IntegerAdder(vtkProgrammableFilter):
    """vtkFilter for adding an integer value to vtkPolyData's CellData. Use
    SetValue to set the value to be added, or supply the optional argument
    'Value' to the constructor.
    """

    def __init__(self, Value=-1):
        self.SetExecuteMethod(self._Execute)
        self.Value = Value
        return

    def SetValue(self, val):
        self.Value = val
        return

    def GetValue(self):
        return self.Value

    def _Execute(self, *args):
        """Run the filter.
        """
        # Get the input
        inputPD = self.GetPolyDataInput()
        values = vtkIntArray()
        values.SetNumberOfTuples(inputPD.GetNumberOfCells())
        values.FillComponent(0, self.Value)

        # Get the out PD
        out = self.GetPolyDataOutput()
        out.CopyStructure(inputPD)
        out.GetCellData().SetScalars(values)

        return

    pass


class PolyDataClipCapAndLabeller(vtkProgrammableFilter):
    """vtkFilter for clipping and capping a vtkPolyData surface, and labeling
    the cap with an integer cell data value.
    """
    def __init__(self, Value=None, Iolet=None, SeedPoint=None):
        self.SetExecuteMethod(self._Execute)
        self.Value = Value
        self.Iolet = Iolet
        self.SeedPoint = (SeedPoint.x, SeedPoint.y, SeedPoint.z)
        return

    def SetValue(self, val):
        self.Value = val
        return

    def GetValue(self):
        return self.Value

    def SetIolet(self, val):
        self.Iolet = val
        return

    def GetIolet(self):
        return self.Iolet

    def _Clip(self, pd):
        # The plane implicit function will be >0 for all the points in the positive side
        # of the plane (i.e. x s.t. n.(x-o)>0, where n is the plane normal and o is the
        # plane origin).
        plane = vtkPlane()
        plane.SetOrigin(
            self.Iolet.Centre.x, self.Iolet.Centre.y, self.Iolet.Centre.z)
        plane.SetNormal(
            self.Iolet.Normal.x, self.Iolet.Normal.y, self.Iolet.Normal.z)

        # The sphere implicit function will be >0 for all the points outside
        # the sphere.
        sphere = vtkSphere()
        sphere.SetCenter(
            self.Iolet.Centre.x, self.Iolet.Centre.y, self.Iolet.Centre.z)
        sphere.SetRadius(self.Iolet.Radius)

        # The VTK_INTERSECTION operator takes the maximum value of all the registered
        # implicit functions. This will result in the function evaluating to >0 for all
        # the points outside the sphere plus those inside the sphere in the positive
        # side of the plane.
        clippingFunction = vtkImplicitBoolean()
        clippingFunction.AddFunction(plane)
        clippingFunction.AddFunction(sphere)
        clippingFunction.SetOperationTypeToIntersection()

        clipper = vtkClipPolyData()
        clipper.SetInput(pd)
        clipper.SetClipFunction(clippingFunction)

        # Filter to get part closest to seed point
        connectedRegionGetter = vtkPolyDataConnectivityFilter()
        connectedRegionGetter.SetExtractionModeToClosestPointRegion()
        connectedRegionGetter.SetClosestPoint(*self.SeedPoint)
        connectedRegionGetter.SetInputConnection(clipper.GetOutputPort())
        connectedRegionGetter.Update()
        return connectedRegionGetter.GetOutput()

    def _AddValue(self, pd):
        adder = IntegerAdder(Value=self.Value)
        adder.SetInput(pd)
        adder.Update()
        return adder.GetOutput()

    def _Execute(self, *args):
        assert isinstance(self.Value, int)
        assert isinstance(self.Iolet, Iolet)
        inputPD = self.GetPolyDataInput()
        clipped = self._Clip(inputPD)

        clipped.BuildLinks(0)

        newPoints = vtkPoints()
        newPoints.DeepCopy(clipped.GetPoints())

        newPolys = vtkCellArray()
        newPolys.DeepCopy(clipped.GetPolys())

        newData = vtkIntArray()
        newData.DeepCopy(clipped.GetCellData().GetScalars())

        boundaryExtractor = vtkvmtkPolyDataBoundaryExtractor()
        boundaryExtractor.SetInput(clipped)
        boundaryExtractor.Update()
        boundaries = boundaryExtractor.GetOutput()
        boundariesPointIdMap = boundaries.GetPointData().GetScalars()
        for i in xrange(boundaries.GetNumberOfCells()):
            boundary = vtkPolyLine.SafeDownCast(boundaries.GetCell(i))

            barycentre = [0., 0., 0.]
            vtkvmtkBoundaryReferenceSystems.ComputeBoundaryBarycenter(
                boundary.GetPoints(),
                barycentre)

            barycentreId = newPoints.InsertNextPoint(barycentre)

            numberOfBoundaryPoints = boundary.GetNumberOfPoints()
            trianglePoints = vtkIdList()
            trianglePoints.SetNumberOfIds(3)

            for j in xrange(numberOfBoundaryPoints):
                trianglePoints.SetId(0,
                                     boundariesPointIdMap.GetValue(boundary.GetPointId(j)))
                trianglePoints.SetId(1, barycentreId)
                trianglePoints.SetId(2,
                                     boundariesPointIdMap.GetValue(boundary.GetPointId((j + 1) % numberOfBoundaryPoints))
                                     )
                newPolys.InsertNextCell(trianglePoints)
                newData.InsertNextValue(self.Value)
                continue

            continue

        output = self.GetPolyDataOutput()
        output.SetPoints(newPoints)
        output.SetPolys(newPolys)
        output.GetCellData().SetScalars(newData)

        return

    pass


class PolyDataCloser(vtkProgrammableFilter):
    """Given an input vtkPolyData object, close any holes in the surface.
    """

    def __init__(self):
        self.SetExecuteMethod(self._Execute)

    def _Execute(self, *args):
        inputPD = self.GetPolyDataInput()

        # Gets any edges of the mesh
        edger = vtkFeatureEdges()
        edger.BoundaryEdgesOn()
        edger.FeatureEdgesOff()
        edger.NonManifoldEdgesOff()
        edger.ManifoldEdgesOff()
        edger.SetInput(inputPD)

        # Converts the edges to a polyline
        stripper = vtkStripper()
        stripper.SetInputConnection(edger.GetOutputPort())
        stripper.Update()

        # Change the polylines into polygons
        boundaryPoly = vtkPolyData()
        boundaryPoly.SetPoints(stripper.GetOutput().GetPoints())
        boundaryPoly.SetPolys(stripper.GetOutput().GetLines())

        # Triangulate
        tri = vtkTriangleFilter()
        tri.SetInput(boundaryPoly)
        tri.Update()

        # Join to the input
        merger = vtkAppendPolyData()
        merger.AddInput(inputPD)
        merger.AddInput(tri.GetOutput())

        # Clean up by merging duplicate points
        cleaner = vtkCleanPolyData()
        cleaner.SetInputConnection(merger.GetOutputPort())
        cleaner.Update()

        # Set output
        output = self.GetPolyDataOutput()
        output.ShallowCopy(cleaner.GetOutput())
        return
    pass