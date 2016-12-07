#include <osg/Version>

#include <osgViewer/Viewer>
#include <osgUtil/Optimizer>

#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include <osg/Material>
#include <osg/StateSet>
#include <osgDB/ReadFile>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Node>
#include <osg/PositionAttitudeTransform>
#include <osg/Light>
#include <osg/LightSource>
#include <osg/AnimationPath>
#include <osg/MatrixTransform>
#include <osg/LOD>
#include <osgUtil/Simplifier>


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

    const float dimX = 200;
    const float dimY = 200;

    // define ground
    osg::ref_ptr<osg::HeightField> field = new osg::HeightField();

    //allocate
    field->allocate(dimX, dimY);
    field->setXInterval(1.0f);
    field->setYInterval(1.0f);
    field->setOrigin( osg::Vec3( -(dimX / 2), -(dimY / 2) , 0.0f) );

    //set the Height at each point
    for(int r = 0; r < field->getNumRows(); r++) {
        for (int c = 0; c < field->getNumColumns(); c++) {
            field->setHeight(r, c, cos(r/2) + sin(c/2));
        }
    }


    // add the ground to the scene
    osg::ref_ptr<osg::Geode> groundGeode = new osg::Geode();

    groundGeode->addDrawable(new osg::ShapeDrawable(field));

    root->addChild(groundGeode);


    //define model
    osg::ref_ptr<osg::Node> gliderNode = osgDB::readNodeFile("/Users/VikH/Documents/Skola/TNM086/TNM086/lab1/glider.osg");

    osg::ref_ptr<osg::PositionAttitudeTransform> gliderTransform = new osg::PositionAttitudeTransform();
    gliderTransform->addChild(gliderNode);
    gliderTransform->setPosition( osg::Vec3( (dimX/5),(dimY/5), 25) );
    gliderTransform->setScale( osg::Vec3(10,10,10) );
    root->addChild(gliderTransform);






    //Optimizes the scene-graph
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
