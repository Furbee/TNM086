#include <osg/Version>

#include <osgViewer/Viewer>
#include <osgUtil/Optimizer>


int main(int argc, char *argv[]){
  
  osg::ref_ptr<osg::Group> root = new osg::Group;

#if 1
  /// Line ---

  osg::Vec3 line_p0 (-1, 0, 0);
  osg::Vec3 line_p1 ( 1, 0, 0);
  
  osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array();
  vertices->push_back(line_p0);
  vertices->push_back(line_p1);
  
  osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
  colors->push_back(osg::Vec4(0.9f,0.2f,0.3f,1.0f));

  osg::ref_ptr<osg::Geometry> linesGeom = new osg::Geometry();
  linesGeom->setVertexArray(vertices);
  linesGeom->setColorArray(colors, osg::Array::BIND_OVERALL);
  
  linesGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,2));
  
  osg::ref_ptr<osg::Geode> lineGeode = new osg::Geode();
  lineGeode->addDrawable(linesGeom);
  lineGeode->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
  
  root->addChild(lineGeode);
  
  /// ---
#endif

  
  // Add your stuff to the root node here...
  
  
  // Optimizes the scene-graph
  //osgUtil::Optimizer optimizer;
  //optimizer.optimize(root);
  
  // Set up the viewer and add the scene-graph root
  osgViewer::Viewer viewer;
  viewer.setSceneData(root);

  osg::ref_ptr<osg::Camera> camera = new osg::Camera;
  camera->setProjectionMatrixAsPerspective(60.0, 1.0, 0.1, 100.0);
  camera->setViewMatrixAsLookAt (osg::Vec3d(0.0, 0.0, 2.0),
                                 osg::Vec3d(0.0, 0.0, 0.0),
                                 osg::Vec3d(0.0, 1.0, 0.0));
  camera->getOrCreateStateSet()->setGlobalDefaults();
  viewer.setCamera(camera);
  
  return viewer.run();
}
