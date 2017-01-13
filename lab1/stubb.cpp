#include <osg/Version>
#include <osg/Node>
#include <osgDB/ReadFile>
#include <osg/PositionAttitudeTransform>
#include <osg/AnimationPath>
#include <osg/MatrixTransform>
#include <osgViewer/Viewer>
#include <osgUtil/Simplifier>
#include <osgUtil/Optimizer>
#include <osg/ShapeDrawable>
#include <osg/CopyOp>
#include <osgUtil/IntersectVisitor>

osg::AnimationPath::ControlPoint createPoint(osg::Vec3 position, osg::Vec3 scale);
osg::ref_ptr<osg::Geode> createGround( int dimX, int dimY, float intervalX, float intervalY );
osg::ref_ptr<osg::HeightField> createHeightField( int dimX, int dimY, float intervalX, float intervalY  );
void setHeights ( osg::ref_ptr<osg::HeightField> field, osg::ref_ptr<osg::Image> heightMap );
osg::ref_ptr<osg::Texture2D> addTexture();
void addPathTo( osg::ref_ptr<osg::PositionAttitudeTransform> nodeTransform);
void addPoints( osg::ref_ptr<osg::AnimationPath> path );
void addLight(osg::ref_ptr<osg::LightSource> lightSource, int lightNum, osg::Vec4 position, osg::Vec4 diffuse, osg::Vec4 ambient, osg::StateSet *r_state);

osg::ref_ptr<osg::LightSource> lightSource3 = new osg::LightSource();/*

class IntersectRef : public osg::Referenced {

public:
    IntersectRef( osgUtil::IntersectionVisitor iv, osg::ref_ptr<osg::Light> light ){

        //iv.clone(this);
        this->iv = iv;

        this->light = light;
    }


    osgUtil::IntersectionVisitor getVisitor() {
        return this->iv;
    }

    osg::ref_ptr<osg::Light> getLight() {
        return this->light;
    }

protected:
    osgUtil::IntersectionVisitor iv;
    osg::ref_ptr<osg::Light> light;
    osg::ref_ptr<osg::Group> root;
};*/

class IntersectCallback : public osg::NodeCallback
{
public:
    virtual void operator() ( osg::Node* node, osg::NodeVisitor* nodeVisit )
    {
        osg::Vec3 lineOne (-200, 0, 50);
        osg::Vec3 lineTwo (200, 0, 50);

        osg::ref_ptr<osgUtil::LineSegmentIntersector> lineIntersector =
                new osgUtil::LineSegmentIntersector(lineOne, lineTwo);
        osgUtil::IntersectionVisitor visitor( lineIntersector.get() );
        node->accept( visitor );

        if(lineIntersector->containsIntersections()){
            lightSource3->getLight()->setDiffuse( osg::Vec4(1.0f, 0.2f, 0.2f,1.0f) );
            lightSource3->getLight()->setAmbient( osg::Vec4( 0.3f, 0.05f, 0.05f, 1.0f));
        }
        else{
            lightSource3->getLight()->setDiffuse( osg::Vec4 (0,0,0,1.0) );
            lightSource3->getLight()->setAmbient( osg::Vec4 (0.0,0,0,1.0));
        }

        traverse(node, nodeVisit);
        /*
        osgUtil::IntersectionVisitor visit = intersectRef->getVisitor();
        node->accept(visit);
        osg::ref_ptr<osgUtil::Intersector> lineIntersector = visit.getIntersector();

        if(lineIntersector->containsIntersections()){
            intersectRef->getLight()->setDiffuse( osg::Vec4(1,1,1,1) );
        }
        else{
            intersectRef->getLight()->setDiffuse( osg::Vec4 (1,0 ,0,1) );
        }
        lineIntersector->reset();
        traverse(node, nodeVisit); */
    }
};


int main(int argc, char *argv[]) {

    osg::ref_ptr<osg::Group> root = new osg::Group;

#if 1
    /// Line ---

    osg::Vec3 line_p0(-200, 0, 50);
    osg::Vec3 line_p1(200, 0, 50);

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

    //root->addChild(lineGeode);

    root->setUpdateCallback(new IntersectCallback);

    /// ---
#endif
    /***********************************************************************************************************
     *                                     CODE
     **********************************************************************************************************/

    //set dim of ground plane
    const unsigned int DIMX = 256;
    const unsigned int DIMY = 256;
    //set intervals of ground plane
    const float INTX = 1.0f;
    const float INTY = 1.0f;

    //create ground plane
    osg::ref_ptr<osg::Geode> groundGeode = createGround( DIMX, DIMY, INTX, INTY ); //create the ground
    root->addChild(groundGeode); //add ground to root

    //define model
    osg::ref_ptr<osg::Node> gliderNode = osgDB::readNodeFile("cessna.osg");
    osg::ref_ptr<osg::PositionAttitudeTransform> gliderNodeTransform =
            new osg::PositionAttitudeTransform();
    gliderNodeTransform->addChild(gliderNode);
    gliderNodeTransform->setScale(osg::Vec3(5, 5, 5));

    addPathTo(gliderNodeTransform);
    //add to root
    root->addChild(gliderNodeTransform);

    //create dupTruck with LOD
    osg::ref_ptr<osg::Node> dumpTruck = osgDB::readNodeFile("dumptruck.osg");

    //use LODs
    osgUtil::Simplifier simple(0.53);
    simple.setMaximumLength(2.0f);

    osg::ref_ptr<osg::Node> dumpTruckLower =
            dynamic_cast<osg::Node*>(dumpTruck->clone(osg::CopyOp::DEEP_COPY_ALL));
    dumpTruckLower->accept(simple);

    osg::ref_ptr<osg::Node> dumpTruckLowest =
            dynamic_cast<osg::Node*>(dumpTruck->clone(osg::CopyOp::DEEP_COPY_ALL));
    simple.setSampleRatio(0.1f);
    dumpTruckLowest->accept(simple);

    osg::ref_ptr<osg::LOD> dumpTruckLOD = new osg::LOD();
    dumpTruckLOD->setRangeMode( osg::LOD::DISTANCE_FROM_EYE_POINT );
    dumpTruckLOD->addChild(dumpTruck, 0,200);
    dumpTruckLOD->addChild(dumpTruckLower, 201,500);
    dumpTruckLOD->addChild(dumpTruckLowest,501,10000);

    osg::ref_ptr<osg::PositionAttitudeTransform> dumpTruckTransform =
            new osg::PositionAttitudeTransform();

    dumpTruckTransform->addChild(dumpTruckLOD);
    dumpTruckTransform->setPosition(osg::Vec3(-50,5,20));
    dumpTruckTransform->setScale(osg::Vec3(1.5,1.5,1.5));
    //add to root
    root->addChild(dumpTruckTransform);


    osg::StateSet *root_state = root->getOrCreateStateSet();

    //Add light to scene
    osg::ref_ptr<osg::LightSource> lightSource = new osg::LightSource();
    osg::ref_ptr<osg::LightSource> lightSource2 = new osg::LightSource();
    //addLight(lightSource, 0 , osg::Vec4(45, 45, 45, 1.0) , osg::Vec4(0, 0, 0, 0), osg::Vec4(0.0f, 0.9f, 0.0f, 1.0), root_state );
    //root->addChild(lightSource); //add to root

    //spotlight
    osg::ref_ptr<osg::Light> light = new osg::Light();
    light->setLightNum(0);
    light->setPosition( osg::Vec4(-128,-128,100,1) );
    light->setAmbient( osg::Vec4(0.0f, 0.1f, 0.2f, 1.0) );
    light->setDiffuse( osg::Vec4(0.0f, 0.4f ,0.9, 1.0) );
    light->setSpotCutoff(28.0);
    light->setSpotExponent(5.0);
    light->setDirection( osg::Vec3(1.0f, 1.0f, -1.0f) );

    lightSource->setLight(light);
    lightSource->setLocalStateSetModes(osg::StateAttribute::ON);
    lightSource->setStateSetModes(*root_state, osg::StateAttribute::ON);

    root->addChild(lightSource);

    //ambient light
    osg::ref_ptr<osg::Light> light2 = new osg::Light();
    light2->setLightNum(1);
    light2->setPosition( osg::Vec4(0.0f,0.0f,200.0f,1.0f) );
    light2->setAmbient( osg::Vec4( 0.0f, 0.25f, 0.0f, 1.0f) );
    light2->setDiffuse( osg::Vec4(0.0f, 0.5f, 0.4f, 1.0f) );
    light2->setConstantAttenuation(1.0f);
    light2->setLinearAttenuation(1.0f/128.0f);
    //light2->setQuadraticAttenuation( 2.0f/osg::square(128.0f) );

    lightSource2->setLight(light2);
    lightSource2->setLocalStateSetModes(osg::StateAttribute::ON);
    lightSource2->setStateSetModes(*root_state, osg::StateAttribute::ON);

    root->addChild(lightSource2);

    //bomb light
    osg::ref_ptr<osg::Light> light3 = new osg::Light();
    light3->setLightNum(2);
    light3->setPosition( osg::Vec4(-50, 5, 25,1) );
    light3->setAmbient( osg::Vec4(0, 0, 0, 0) );
    light3->setDiffuse( osg::Vec4(0,0,0,0) );

    lightSource3->setLight(light3);
    lightSource3->setLocalStateSetModes( osg::StateAttribute::ON );
    lightSource3->setStateSetModes( *root_state, osg::StateAttribute::ON );

    root->addChild(lightSource3);

    //Add light 2 to scene

    //addLight(lightSource2, 1 , osg::Vec4(200, 200, 200, 1.0) , osg::Vec4(0.0, 0, 0.2f, 1.0), osg::Vec4(0, 0, 0, 0), root_state );
    //root->addChild(lightSource2); //add too root

    //addLight(lightSource3, 0, osg::Vec4(0, 0, 0, 0), osg::Vec4(0.1, 0.1, 0.1, 1), osg::Vec4(0.5, 0.4, 0.4, 1.0) );
    //lightSource->getLight()->setLightNum(2);
    //root->addChild(lightSource3);

    //Optimizes the scene-graph
    osgUtil::Optimizer optimizer;
    optimizer.optimize(root);

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

/***********************************************************************************************************
*                                     SUPPORT FUNCTIONS
**********************************************************************************************************/


osg::ref_ptr<osg::Geode> createGround(int dimX, int dimY, float intervalX, float intervalY) {
    //create field
    osg::ref_ptr<osg::HeightField> field = createHeightField( dimX, dimY, intervalX, intervalY );

    //add texture to field
    osg::ref_ptr<osg::Texture2D> groundTexture = addTexture();

    //add ground with texture to scene
    osg::ref_ptr<osg::Geode> groundGeode = new osg::Geode();
    groundGeode->addDrawable(new osg::ShapeDrawable(field));
    groundGeode->getOrCreateStateSet()->setTextureAttributeAndModes(0, groundTexture);

    return groundGeode;

}

osg::ref_ptr<osg::HeightField> createHeightField( int dimX, int dimY, float intervalX, float intervalY ) {
    //read height map file
    osg::ref_ptr<osg::Image> heightMap = osgDB::readImageFile("heightmap_256sqr3.jpg");

    //define ground
    osg::ref_ptr<osg::HeightField> field = new osg::HeightField();

    //allocate
    field->allocate( dimX, dimY );
    field->setXInterval( intervalX );
    field->setYInterval( intervalY );
    field->setOrigin(osg::Vec3(-(dimX / 2), -(dimY / 2), 0.0f));

    setHeights(field, heightMap);

    return field;
}


osg::ref_ptr<osg::Texture2D> addTexture(){

    //set ground texture
    osg::ref_ptr<osg::Texture2D> groundTexture  = new osg::Texture2D(osgDB::readImageFile("ground.png"));

//wrapping of texture
    groundTexture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
    groundTexture->setWrap(osg::Texture::WRAP_R, osg::Texture::REPEAT);

    return groundTexture;

}


void setHeights ( osg::ref_ptr<osg::HeightField> field, osg::ref_ptr<osg::Image> heightMap ){
    //set the Height at each point
    for (unsigned int r = 0; r < field->getNumRows(); r++) {
        for (unsigned int c = 0; c < field->getNumColumns(); c++) {
            float heightMapVal = ((float) *heightMap->data(c, r) / 12);
            field->setHeight(c, r, heightMapVal);
        }
    }
}

void addPathTo( osg::ref_ptr<osg::PositionAttitudeTransform> nodeTransform) {

    //set animation path
    osg::ref_ptr<osg::AnimationPath> gliderPath = new osg::AnimationPath();

    //add some points
    addPoints(gliderPath);

    //set loop-mode
    gliderPath->setLoopMode(osg::AnimationPath::LOOP);

    osg::ref_ptr<osg::AnimationPathCallback> glidercb =
            new osg::AnimationPathCallback(gliderPath);

    nodeTransform->setUpdateCallback(glidercb);
}

void addPoints( osg::ref_ptr<osg::AnimationPath> path ){

    //create four points for the path
    osg::AnimationPath::ControlPoint p1( osg::Vec3(-50, 250, 150) );
    p1.setScale(osg::Vec3(0.3, 0.3, 0.3));
    path->insert(0.0f, p1);

    osg::AnimationPath::ControlPoint p2( osg::Vec3(-50, 50, 50) );
    p2.setScale(osg::Vec3(1, 1, 1));
    path->insert(3.0f, p2);

    osg::AnimationPath::ControlPoint p3( osg::Vec3(-50, -50, 50) );
    p3.setScale(osg::Vec3(1, 1, 1));
    path->insert(4.5f, p3);

    osg::AnimationPath::ControlPoint p4( osg::Vec3(-50, -250, 150) );
    p4.setScale(osg::Vec3(0.3, 0.3, 0.3));
    path->insert(7.5f, p4);
}

void addLight(osg::ref_ptr<osg::LightSource> lightSource, int lightNum, osg::Vec4 position, osg::Vec4 diffuse, osg::Vec4 ambient, osg::StateSet *r_state) {

/*    osg::ref_ptr<osg::Light> light = new osg::Light();

    light->setLightNum( lightNum );
    light->setPosition( position );
    light->setDiffuse( diffuse );
    light->setAmbient( ambient );

    lightSource->setLight( light );
    lightSource->setLocalStateSetModes( osg::StateAttribute::ON );
    lightSource->setStateSetModes( *r_state, osg::StateAttribute::ON );
    */
}
