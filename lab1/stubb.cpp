#include <osg/Version>
#include <iostream>
#include <osg/Node>
#include <iostream>
#include <osgDB/ReadFile>
#include <osg/PositionAttitudeTransform>
#include <osg/MatrixTransform>
#include <osgViewer/Viewer>
#include <osgUtil/Simplifier>
#include <osgUtil/Optimizer>
#include <osg/ShapeDrawable>
#include <osg/CopyOp>
#include <osgUtil/IntersectVisitor>
#include <osg/AnimationPath>


int main(int argc, char *argv[]) {

    osg::ref_ptr<osg::Group> root = new osg::Group;

#if 1
    /// Line ---

    osg::Vec3 line_p0(-1, 0, 0);
    osg::Vec3 line_p1(1, 0, 0);

    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array();
    vertices->push_back(line_p0);
    vertices->push_back(line_p1);

    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
    colors->push_back(osg::Vec4(0.9f, 0.2f, 0.3f, 1.0f));

    osg::ref_ptr<osg::Geometry> linesGeom = new osg::Geometry();
    linesGeom->setVertexArray(vertices);
    linesGeom->setColorArray(colors, osg::Array::BIND_OVERALL);

    linesGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, 2));

    osg::ref_ptr<osg::Geode> lineGeode = new osg::Geode();
    lineGeode->addDrawable(linesGeom);
    lineGeode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    root->addChild(lineGeode);

    /// ---
#endif

    //set dim of ground plane
    const int dimX = 256;
    const int dimY = 256;

    //set ground texture
    osg::ref_ptr<osg::Texture2D> groundTexture = new osg::Texture2D(osgDB::readImageFile("heightmap_256sqr3.jpg"));

    //read height map file
    osg::ref_ptr<osg::Image> heightMap = osgDB::readImageFile("heightmap_256sqr3.jpg");

    //wrapping of texture
    groundTexture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
    groundTexture->setWrap(osg::Texture::WRAP_R, osg::Texture::REPEAT);

    //define ground
    osg::ref_ptr<osg::HeightField> field = new osg::HeightField();

    //allocate
    field->allocate(dimX, dimY);
    field->setXInterval(1.0f);
    field->setYInterval(1.0f);
    field->setOrigin(osg::Vec3(-(dimX / 2), -(dimY / 2), 0.0f));

    //set the Height at each point
    for (unsigned int r = 0; r < field->getNumRows(); r++) {
        for (unsigned int c = 0; c < field->getNumColumns(); c++) {

            //std::cout << "Column: " << c << " Row: " << r << " Value: " << ( (float)*heightMap-> data( c, r ) / 255 ) <<std:: endl;
            //field->setHeight(r, c, cos(r/8) + sin(c/8));
            float heightMapVal = ((float) *heightMap->data(c, r) / 12);
            field->setHeight(c, r, heightMapVal);
        }
    }

    //add ground with texture to scene
    osg::ref_ptr<osg::Geode> groundGeode = new osg::Geode();
    groundGeode->addDrawable(new osg::ShapeDrawable(field));
    groundGeode->getOrCreateStateSet()->setTextureAttributeAndModes(0, groundTexture);

    //add to root
    root->addChild(groundGeode);

    //define model
    osg::ref_ptr<osg::Node> gliderNode = osgDB::readNodeFile("cessna.osg");
    osg::ref_ptr<osg::PositionAttitudeTransform> gliderNodeTransform =
            new osg::PositionAttitudeTransform();
    gliderNodeTransform->addChild(gliderNode);
    gliderNodeTransform->setScale(osg::Vec3(1, 1, 1));

    //set animation path
    osg::ref_ptr<osg::AnimationPath> gliderPath = new osg::AnimationPath();
    osg::AnimationPath::ControlPoint p1(
            osg::Vec3(50, 50, 50));
    p1.setScale(osg::Vec3(1, 1, 1));
    osg::AnimationPath::ControlPoint p2(
            osg::Vec3(50, 500, 50));
    p2.setScale(osg::Vec3(1, 1, 1));
    osg::AnimationPath::ControlPoint p3(
            osg::Vec3(50, -700, 50));
    p3.setScale(osg::Vec3(1, 1, 1));

    gliderPath->insert(0.0f, p1);
    gliderPath->insert(2.0f, p2);
    gliderPath->insert(4.0f, p3);

    gliderPath->setLoopMode(osg::AnimationPath::SWING);
    osg::ref_ptr<osg::AnimationPathCallback> glidercb =
            new osg::AnimationPathCallback(gliderPath);
    gliderNodeTransform->setUpdateCallback(glidercb);

    //add to root
    root->addChild(gliderNodeTransform);

    //Add light to scene
    osg::ref_ptr<osg::LightSource> lightSource = new osg::LightSource();
    osg::ref_ptr<osg::Light> light = new osg::Light();
    light->setLightNum(0);
    light->setPosition(osg::Vec4(45, 45, 45, 1.0));
    light->setDiffuse(osg::Vec4(1.0, 0, 0, 1.0));
    light->setAmbient(osg::Vec4(0.03f, 0.03f, 0.03f, 1.0));
    lightSource->setLight(light);

    //add to root
    root->addChild(lightSource);

    //Add light 2 to scene
    osg::ref_ptr<osg::LightSource> lightSource2 = new osg::LightSource();
    osg::ref_ptr<osg::Light> light2 = new osg::Light();
    light2->setLightNum(0);
    light2->setPosition(osg::Vec4(200, 200, 200, 1.0));
    light2->setDiffuse(osg::Vec4(1.0, 0.9f, 0.9f, 1.0));
    //light2->setAmbient(osg::Vec4(0.3, 0.3f, 0.3f, 1.0));
    lightSource2->setLight(light2);

    //add to root
    root->addChild(lightSource2);

    //Optimizes the scene-graph
    //osgUtil::Optimizer optimizer;
    //optimizer.optimize(root);

    // Set up the viewer and add the scene-graph root
    osgViewer::Viewer viewer;
    viewer.setSceneData(root);

    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setProjectionMatrixAsPerspective(60.0, 1.0, 0.1, 100.0);
    camera->setViewMatrixAsLookAt(osg::Vec3d(0.0, 0.0, 2.0),
                                  osg::Vec3d(0.0, 0.0, 0.0),
                                  osg::Vec3d(0.0, 1.0, 0.0));
    camera->getOrCreateStateSet()->setGlobalDefaults();
    viewer.setCamera(camera);

    return viewer.run();
}
